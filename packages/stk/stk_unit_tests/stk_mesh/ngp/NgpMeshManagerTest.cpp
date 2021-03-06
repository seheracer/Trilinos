#include <gtest/gtest.h>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/HostMeshManager.hpp>
#include <stk_mesh/base/DeviceMeshManager.hpp>
#include <stk_mesh/base/NgpMesh.hpp>
#include <stk_io/FillMesh.hpp>
#include <type_traits>

namespace  {

TEST(NgpMeshManager, HostMeshManager)
{
  stk::mesh::MetaData meta(3);
  stk::mesh::BulkData bulk(meta, MPI_COMM_WORLD);

  stk::mesh::NgpMeshManager * ngpMeshManager = new stk::mesh::HostMeshManager(bulk);
  delete ngpMeshManager;
}

TEST(NgpMeshManager, DeviceMeshManager)
{
  stk::mesh::MetaData meta(3);
  stk::mesh::BulkData bulk(meta, MPI_COMM_WORLD);

  stk::mesh::NgpMeshManager * ngpMeshManager = new stk::mesh::DeviceMeshManager(bulk);
  delete ngpMeshManager;
}

TEST(NgpMeshManager, NgpMesh_FromBulkData)
{
  stk::mesh::MetaData meta(3);
  stk::mesh::BulkData bulk(meta, MPI_COMM_WORLD);

  stk::mesh::NgpMesh & ngpMesh = bulk.get_updated_ngp_mesh();

#ifdef STK_USE_DEVICE_MESH
  EXPECT_TRUE((std::is_same<decltype(ngpMesh), stk::mesh::DeviceMesh&>::value));
#else
  EXPECT_TRUE((std::is_same<decltype(ngpMesh), stk::mesh::HostMesh&>::value));
#endif
}

TEST(NgpMeshManager, NgpMesh_Update)
{
  stk::mesh::MetaData meta(3);
  stk::mesh::BulkData bulk(meta, MPI_COMM_WORLD);
  stk::io::fill_mesh("generated:1x1x4", bulk);

#ifdef STK_USE_DEVICE_MESH
  stk::mesh::NgpMeshManager * ngpMeshManager = new stk::mesh::DeviceMeshManager(bulk);
#else
  stk::mesh::NgpMeshManager * ngpMeshManager = new stk::mesh::HostMeshManager(bulk);
#endif

  bulk.modification_begin();
  bulk.modification_end();

  stk::mesh::NgpMesh& ngpMesh = ngpMeshManager->get_mesh();

#ifdef STK_USE_DEVICE_MESH
  EXPECT_FALSE(ngpMesh.is_up_to_date());
#else
  EXPECT_TRUE(ngpMesh.is_up_to_date());
#endif

  ngpMeshManager->update_mesh();

  EXPECT_TRUE(ngpMesh.is_up_to_date());

  delete ngpMeshManager;
}

}
