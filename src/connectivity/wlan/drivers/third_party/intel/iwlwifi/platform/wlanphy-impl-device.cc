// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/platform/wlanphy-impl-device.h"

#include <zircon/status.h>

#include <memory>

extern "C" {
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/mvm/mvm.h"
}  // extern "C"

#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/platform/mvm-mlme.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/platform/wlan-softmac-device.h"

namespace wlan::iwlwifi {

WlanphyImplDevice::WlanphyImplDevice(zx_device_t* parent)
    : ::ddk::Device<WlanphyImplDevice, ::ddk::Initializable, ::ddk::Unbindable>(parent) {}

WlanphyImplDevice::~WlanphyImplDevice() = default;

void WlanphyImplDevice::DdkRelease() { delete this; }

zx_status_t WlanphyImplDevice::WlanphyImplGetSupportedMacRoles(
    wlan_mac_role_t out_supported_mac_roles_list[fuchsia_wlan_common_MAX_SUPPORTED_MAC_ROLES],
    uint8_t* out_supported_mac_roles_count) {
  return phy_get_supported_mac_roles(drvdata(), out_supported_mac_roles_list,
                                     out_supported_mac_roles_count);
}

zx_status_t WlanphyImplDevice::WlanphyImplCreateIface(const wlanphy_impl_create_iface_req_t* req,
                                                      uint16_t* out_iface_id) {
  zx_status_t status = ZX_OK;

  if (req == nullptr || out_iface_id == nullptr) {
    IWL_ERR(this, "%s() invalid input args req:%p out_iface_id:%p\n", __func__, req, out_iface_id);
    return ZX_ERR_INVALID_ARGS;
  }

  if ((status = phy_create_iface(drvdata(), req, out_iface_id)) != ZX_OK) {
    IWL_ERR(this, "%s() failed phy create: %s\n", __func__, zx_status_get_string(status));
    return status;
  }

  struct iwl_mvm* mvm = iwl_trans_get_mvm(drvdata());
  struct iwl_mvm_vif* mvmvif = mvm->mvmvif[*out_iface_id];

  auto wlan_softmac_device =
      std::make_unique<WlanSoftmacDevice>(parent(), drvdata(), *out_iface_id, mvmvif);

  if ((status = wlan_softmac_device->DdkAdd("iwlwifi-wlan-softmac")) != ZX_OK) {
    IWL_ERR(this, "%s() failed mac device add: %s\n", __func__, zx_status_get_string(status));
    phy_create_iface_undo(drvdata(), *out_iface_id);
    return status;
  }
  wlan_softmac_device.release();
  return ZX_OK;
}

zx_status_t WlanphyImplDevice::WlanphyImplDestroyIface(uint16_t iface_id) {
  return phy_destroy_iface(drvdata(), iface_id);
}

zx_status_t WlanphyImplDevice::WlanphyImplSetCountry(const wlanphy_country_t* country) {
  return phy_set_country(drvdata(), country);
}

zx_status_t WlanphyImplDevice::WlanphyImplClearCountry() {
  IWL_ERR(this, "%s() not implemented ...\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

zx_status_t WlanphyImplDevice::WlanphyImplGetCountry(wlanphy_country_t* out_country) {
  return phy_get_country(drvdata(), out_country);
}

zx_status_t WlanphyImplDevice::WlanphyImplSetPsMode(const wlanphy_ps_mode_t* pm_mode) {
  return ZX_ERR_NOT_SUPPORTED;
}

zx_status_t WlanphyImplDevice::WlanphyImplGetPsMode(wlanphy_ps_mode_t* out_pm_mode) {
  return ZX_ERR_NOT_SUPPORTED;
}

}  // namespace wlan::iwlwifi
