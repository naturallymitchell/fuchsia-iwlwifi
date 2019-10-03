// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// To test PHY and MAC device callback functions.

extern "C" {
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/wlan-device.h"
}

#include <lib/fake_ddk/fake_ddk.h>
#include <lib/mock-function/mock-function.h>
#include <stdio.h>
#include <zircon/syscalls.h>

#include <zxtest/zxtest.h>

extern "C" {
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/mvm/mvm.h"
}

#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/test/single-ap-test.h"

namespace wlan::testing {
namespace {

class WlanDeviceTest : public SingleApTest {
 public:
  WlanDeviceTest()
      : mvmvif_sta_{
            .mac_role = WLAN_INFO_MAC_ROLE_CLIENT,
        } {}
  ~WlanDeviceTest() {}

 protected:
  static constexpr zx_handle_t sme_channel_ = 73939133;  // An arbitrary value not ZX_HANDLE_INVALID
  struct iwl_mvm_vif mvmvif_sta_;                        // The mvm_vif settings for station role.
};

/////////////////////////////////////       MAC       //////////////////////////////////////////////

TEST_F(WlanDeviceTest, MacQuery) {
  // Test input null pointers
  uint32_t options = 0;
  void* whatever = &options;
  ASSERT_EQ(wlanmac_ops.query(nullptr, options, nullptr), ZX_ERR_INVALID_ARGS);
  ASSERT_EQ(wlanmac_ops.query(whatever, options, nullptr), ZX_ERR_INVALID_ARGS);
  ASSERT_EQ(wlanmac_ops.query(nullptr, options, reinterpret_cast<wlanmac_info*>(whatever)),
            ZX_ERR_INVALID_ARGS);

  wlanmac_info_t info;
  ASSERT_EQ(wlanmac_ops.query(&mvmvif_sta_, options, &info), ZX_OK);
  ASSERT_EQ(info.ifc_info.mac_role, WLAN_INFO_MAC_ROLE_CLIENT);
}

TEST_F(WlanDeviceTest, MacStart) {
  wlanmac_ifc_t ifc;
  zx_handle_t sme_channel;

  // Test input null pointers
  ASSERT_EQ(wlanmac_ops.start(nullptr, &ifc, &sme_channel, nullptr), ZX_ERR_INVALID_ARGS);
  ASSERT_EQ(wlanmac_ops.start(&mvmvif_sta_, nullptr, &sme_channel, nullptr), ZX_ERR_INVALID_ARGS);
  ASSERT_EQ(wlanmac_ops.start(&mvmvif_sta_, &ifc, nullptr, nullptr), ZX_ERR_INVALID_ARGS);
}

TEST_F(WlanDeviceTest, MacStartSmeChannel) {
  wlanmac_ifc_t ifc;
  zx_handle_t sme_channel;

  // The normal case. A channel will be transferred to MLME.
  constexpr zx_handle_t from_devmgr = sme_channel_;
  mvmvif_sta_.sme_channel = from_devmgr;
  ASSERT_EQ(wlanmac_ops.start(&mvmvif_sta_, &ifc, &sme_channel, nullptr), ZX_OK);
  ASSERT_EQ(sme_channel, from_devmgr);                    // The channel handle is returned.
  ASSERT_EQ(mvmvif_sta_.sme_channel, ZX_HANDLE_INVALID);  // Driver no longer holds the ownership.

  // Since the driver no longer owns the handle, the start should fail.
  ASSERT_EQ(wlanmac_ops.start(&mvmvif_sta_, &ifc, &sme_channel, nullptr), ZX_ERR_ALREADY_BOUND);
}

TEST_F(WlanDeviceTest, MacUnbind) {
  wlanphy_impl_create_iface_req_t req = {
      .role = WLAN_INFO_MAC_ROLE_CLIENT,
      .sme_channel = sme_channel_,
  };
  uint16_t iface_id;
  struct iwl_trans* iwl_trans = sim_trans_.iwl_trans();

  // Create an interface
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_OK);

  // To verify the internal state of MVM driver.
  struct iwl_mvm* mvm = iwl_trans_get_mvm(iwl_trans);
  struct iwl_mvm_vif* mvmvif = mvm->mvmvif[iface_id];

  // Then unbind it
  device_mac_ops.unbind(mvmvif);
  // unbind() doesn't have return value. Expect it is not crashed.

  // Do again and expect not crashed
  device_mac_ops.unbind(mvmvif);
}

TEST_F(WlanDeviceTest, MacUnbindInvalidZxdev) {
  wlanphy_impl_create_iface_req_t req = {
      .role = WLAN_INFO_MAC_ROLE_CLIENT,
      .sme_channel = sme_channel_,
  };
  uint16_t iface_id;
  struct iwl_trans* iwl_trans = sim_trans_.iwl_trans();

  // Create an interface
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_OK);

  // To verify the internal state of MVM driver.
  struct iwl_mvm* mvm = iwl_trans_get_mvm(iwl_trans);
  struct iwl_mvm_vif* mvmvif = mvm->mvmvif[iface_id];

  // Invalidate the zxdev with whatever value
  mvmvif->zxdev = fake_ddk::kFakeParent;

  // Expect the unbind still cleans up the internal state.
  device_mac_ops.unbind(mvmvif);
  ASSERT_EQ(mvmvif->zxdev, nullptr);
}

TEST_F(WlanDeviceTest, MacRelease) {
  // Allocate an instance so that we can free that in mac_release().
  struct iwl_mvm_vif* mvmvif =
      reinterpret_cast<struct iwl_mvm_vif*>(calloc(1, sizeof(struct iwl_mvm_vif)));

  // Create a channel. Let this test case holds one end while driver holds the other end.
  char dummy[1];
  zx_handle_t case_end;
  ASSERT_EQ(zx_channel_create(0 /* option */, &case_end, &mvmvif->sme_channel), ZX_OK);
  ASSERT_EQ(zx_channel_write(case_end, 0 /* option */, dummy, sizeof(dummy), nullptr, 0), ZX_OK);

  // Call release and the sme channel should be closed so that we will get a peer-close error while
  // trying to write any data to it.
  device_mac_ops.release(mvmvif);
  ASSERT_EQ(zx_channel_write(case_end, 0 /* option */, dummy, sizeof(dummy), nullptr, 0),
            ZX_ERR_PEER_CLOSED);
}

/////////////////////////////////////       PHY       //////////////////////////////////////////////

TEST_F(WlanDeviceTest, PhyQuery) {
  // Test input null pointers
  ASSERT_EQ(wlanphy_ops.query(nullptr, nullptr), ZX_ERR_INVALID_ARGS);

  // 'ctx' null is okay for now because the code under test is still not using that.
  wlanphy_impl_info_t info;
  ASSERT_EQ(wlanphy_ops.query(nullptr, &info), ZX_OK);
  ASSERT_EQ(info.wlan_info.mac_role, WLAN_INFO_MAC_ROLE_CLIENT);
}

TEST_F(WlanDeviceTest, PhyCreateDestroySingleInterface) {
  wlanphy_impl_create_iface_req_t req = {
      .role = WLAN_INFO_MAC_ROLE_CLIENT,
      .sme_channel = sme_channel_,
  };
  uint16_t iface_id;
  struct iwl_trans* iwl_trans = sim_trans_.iwl_trans();

  // Test input null pointers
  ASSERT_EQ(wlanphy_ops.create_iface(nullptr, &req, &iface_id), ZX_ERR_INVALID_ARGS);
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, nullptr, &iface_id), ZX_ERR_INVALID_ARGS);
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, nullptr), ZX_ERR_INVALID_ARGS);
  ASSERT_EQ(wlanphy_ops.destroy_iface(nullptr, 0), ZX_ERR_INVALID_ARGS);
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, MAX_NUM_MVMVIF), ZX_ERR_INVALID_ARGS);
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, 0), ZX_ERR_NOT_FOUND);  // hasn't been added yet.

  // To verify the internal state of MVM driver.
  struct iwl_mvm* mvm = iwl_trans_get_mvm(iwl_trans);

  // Add interface
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_OK);
  ASSERT_EQ(iface_id, 0);  // the first interface should have id 0.
  ASSERT_NE(mvm->mvmvif[iface_id], nullptr);
  ASSERT_EQ(mvm->mvmvif[iface_id]->mac_role, WLAN_INFO_MAC_ROLE_CLIENT);
  ASSERT_EQ(mvm->vif_count, 1);

  // Remove interface
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, 0), ZX_OK);
  ASSERT_EQ(mvm->mvmvif[iface_id], nullptr);
  ASSERT_EQ(mvm->vif_count, 0);
}

TEST_F(WlanDeviceTest, PhyCreateDestroyMultipleInterfaces) {
  wlanphy_impl_create_iface_req_t req = {
      .role = WLAN_INFO_MAC_ROLE_CLIENT,
      .sme_channel = sme_channel_,
  };
  uint16_t iface_id;
  struct iwl_trans* iwl_trans = sim_trans_.iwl_trans();
  struct iwl_mvm* mvm = iwl_trans_get_mvm(iwl_trans);  // To verify the internal state of MVM driver

  // Add 1st interface
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_OK);
  ASSERT_EQ(iface_id, 0);  // the first interface should have id 0.
  ASSERT_NE(mvm->mvmvif[iface_id], nullptr);
  ASSERT_EQ(mvm->mvmvif[iface_id]->mac_role, WLAN_INFO_MAC_ROLE_CLIENT);
  ASSERT_EQ(mvm->vif_count, 1);

  // Add 2nd interface
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_OK);
  ASSERT_EQ(iface_id, 1);  // the first interface should have id 0.
  ASSERT_NE(mvm->mvmvif[iface_id], nullptr);
  ASSERT_EQ(mvm->mvmvif[iface_id]->mac_role, WLAN_INFO_MAC_ROLE_CLIENT);
  ASSERT_EQ(mvm->vif_count, 2);

  // Add 3rd interface
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_OK);
  ASSERT_EQ(iface_id, 2);  // the first interface should have id 0.
  ASSERT_NE(mvm->mvmvif[iface_id], nullptr);
  ASSERT_EQ(mvm->mvmvif[iface_id]->mac_role, WLAN_INFO_MAC_ROLE_CLIENT);
  ASSERT_EQ(mvm->vif_count, 3);

  // Remove the 2nd interface
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, 1), ZX_OK);
  ASSERT_EQ(mvm->mvmvif[1], nullptr);
  ASSERT_EQ(mvm->vif_count, 2);

  // Add a new interface and it should be the 2nd one.
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_OK);
  ASSERT_EQ(iface_id, 1);  // the first interface should have id 0.
  ASSERT_NE(mvm->mvmvif[iface_id], nullptr);
  ASSERT_EQ(mvm->mvmvif[iface_id]->mac_role, WLAN_INFO_MAC_ROLE_CLIENT);
  ASSERT_EQ(mvm->vif_count, 3);

  // Add 4th interface
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_OK);
  ASSERT_EQ(iface_id, 3);  // the first interface should have id 0.
  ASSERT_NE(mvm->mvmvif[iface_id], nullptr);
  ASSERT_EQ(mvm->mvmvif[iface_id]->mac_role, WLAN_INFO_MAC_ROLE_CLIENT);
  ASSERT_EQ(mvm->vif_count, 4);

  // Add 5th interface and it should fail
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_ERR_NO_RESOURCES);
  ASSERT_EQ(mvm->vif_count, 4);

  // Remove the 2nd interface
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, 1), ZX_OK);
  ASSERT_EQ(mvm->mvmvif[1], nullptr);
  ASSERT_EQ(mvm->vif_count, 3);

  // Remove the 3rd interface
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, 2), ZX_OK);
  ASSERT_EQ(mvm->mvmvif[2], nullptr);
  ASSERT_EQ(mvm->vif_count, 2);

  // Remove the 4th interface
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, 3), ZX_OK);
  ASSERT_EQ(mvm->mvmvif[3], nullptr);
  ASSERT_EQ(mvm->vif_count, 1);

  // Remove the 1st interface
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, 0), ZX_OK);
  ASSERT_EQ(mvm->mvmvif[0], nullptr);
  ASSERT_EQ(mvm->vif_count, 0);

  // Remove the 1st interface again and it should fail.
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, 0), ZX_ERR_NOT_FOUND);
  ASSERT_EQ(mvm->vif_count, 0);
}

TEST_F(WlanDeviceTest, PhyDestroyInvalidZxdev) {
  wlanphy_impl_create_iface_req_t req = {
      .role = WLAN_INFO_MAC_ROLE_CLIENT,
      .sme_channel = sme_channel_,
  };
  uint16_t iface_id;
  struct iwl_trans* iwl_trans = sim_trans_.iwl_trans();

  // To verify the internal state of MVM driver.
  struct iwl_mvm* mvm = iwl_trans_get_mvm(iwl_trans);

  // Add interface
  ASSERT_EQ(wlanphy_ops.create_iface(iwl_trans, &req, &iface_id), ZX_OK);
  ASSERT_NE(mvm->mvmvif[iface_id], nullptr);
  ASSERT_EQ(mvm->vif_count, 1);

  // Replace the zxdev with invalid value
  mvm->mvmvif[iface_id]->zxdev = fake_ddk::kFakeParent;

  // Remove interface
  ASSERT_EQ(wlanphy_ops.destroy_iface(iwl_trans, 0), ZX_OK);
  ASSERT_EQ(mvm->mvmvif[iface_id], nullptr);
  ASSERT_EQ(mvm->vif_count, 0);
}

}  // namespace
}  // namespace wlan::testing
