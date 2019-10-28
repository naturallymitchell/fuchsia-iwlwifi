// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The place holder for the code to interact with the MLME.
//
//          MLME
//            |
//            v
//  +-------------------+
//  |   wlan-device.c   |
//  +-------------------+
//  | PHY ops | MAC ops |
//  +-------------------+
//       |         |
//       v         v
//     mvm/mac80211.c
//
// Note that the |*ctx| in this file is actually |*iwl_trans| passed when device_add() is called.
//
// - sme_channel
//
// The below steps briefly describe how the 'sme_channel' is used and transferred. In short,
// the goal is going to let SME and MLME to have a channel to communicate.
//
// + After the devmgr (the device manager in wlanstack) detects a PHY device, the devmgr first
//   creates a SME instance in order to handle the MAC operation later. Then the devmgr establishes
//   a channel and passes one end of the channel to the SME instance.
//
// + The devmgr requests the PHY device to create a MAC interface. In the request, the other end
//   of channel is passed.
//
// + The driver's phy_create_iface() gets called, and saves the 'sme_channel' handle in the new
//   created MAC context.
//
// + Once the MAC device is added, its mac_start() will be called. Then it will transfer the
//   'sme_channel' handle back to the MLME.
//
// + Now, both sides of channel (SME and MLME) can talk now.
//

#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/wlan-device.h"

#include <stdio.h>
#include <string.h>
#include <zircon/status.h>

#include <ddk/device.h>
#include <ddk/driver.h>

#include "garnet/lib/wlan/protocol/include/wlan/protocol/mac.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/iwl-debug.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/mvm/mvm.h"

/////////////////////////////////////       MAC       //////////////////////////////////////////////

static zx_status_t mac_query(void* ctx, uint32_t options, wlanmac_info_t* info) {
  struct iwl_mvm_vif* mvmvif = ctx;

  if (!ctx || !info) {
    return ZX_ERR_INVALID_ARGS;
  }
  memset(info, 0, sizeof(*info));

  info->ifc_info.mac_role = mvmvif->mac_role;
  info->ifc_info.driver_features = WLAN_INFO_DRIVER_FEATURE_TEMP_DIRECT_SME_CHANNEL;

  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_OK;  // Temporarily returns OK to make the interface list-able.
}

static zx_status_t mac_start(void* ctx, wlanmac_ifc_t* ifc, zx_handle_t* out_sme_channel,
                             void* cookie) {
  struct iwl_mvm_vif* mvmvif = ctx;

  if (!ctx || !ifc || !out_sme_channel) {
    return ZX_ERR_INVALID_ARGS;
  }

  // Clear the output result first.
  *out_sme_channel = ZX_HANDLE_INVALID;

  // The SME channel assigned in phy_create_iface() is gone.
  if (mvmvif->sme_channel == ZX_HANDLE_INVALID) {
    IWL_ERR(mvmvif, "Invalid SME channel. The interface might have been bound already.\n");
    return ZX_ERR_ALREADY_BOUND;
  }

  // Transfer the handle to MLME. Also invalid the copy we hold to indicate that this interface has
  // been bound.
  *out_sme_channel = mvmvif->sme_channel;
  mvmvif->sme_channel = ZX_HANDLE_INVALID;

  mvmvif->ifc = *ifc;

  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_OK;  // Temporarily returns OK to make the interface list-able.
}

static void mac_stop(void* ctx) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
}

static zx_status_t mac_queue_tx(void* ctx, uint32_t options, wlan_tx_packet_t* pkt) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_set_channel(void* ctx, uint32_t options, wlan_channel_t* chan) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_configure_bss(void* ctx, uint32_t options, wlan_bss_config_t* config) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_enable_beaconing(void* ctx, uint32_t options, wlan_bcn_config_t* bcn_cfg) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_configure_beacon(void* ctx, uint32_t options, wlan_tx_packet_t* pkt) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_set_key(void* ctx, uint32_t options, wlan_key_config_t* key_config) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_configure_assoc(void* ctx, uint32_t options, wlan_assoc_ctx_t* assoc_ctx) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_clear_assoc(void* ctx, uint32_t options, const uint8_t* peer_addr) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_start_hw_scan(void* ctx, const wlan_hw_scan_config_t* scan_config) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

wlanmac_protocol_ops_t wlanmac_ops = {
    .query = mac_query,
    .start = mac_start,
    .stop = mac_stop,
    .queue_tx = mac_queue_tx,
    .set_channel = mac_set_channel,
    .configure_bss = mac_configure_bss,
    .enable_beaconing = mac_enable_beaconing,
    .configure_beacon = mac_configure_beacon,
    .set_key = mac_set_key,
    .configure_assoc = mac_configure_assoc,
    .clear_assoc = mac_clear_assoc,
    .start_hw_scan = mac_start_hw_scan,
};

static void mac_unbind(void* ctx) {
  struct iwl_mvm_vif* mvmvif = ctx;

  if (!mvmvif->zxdev) {
    return;
  }

  device_unbind_reply(mvmvif->zxdev);
  mvmvif->zxdev = NULL;
}

static void mac_release(void* ctx) {
  struct iwl_mvm_vif* mvmvif = ctx;

  // Close the SME channel if it is NOT transferred to MLME yet.
  if (mvmvif->sme_channel != ZX_HANDLE_INVALID) {
    zx_handle_close(mvmvif->sme_channel);
    mvmvif->sme_channel = ZX_HANDLE_INVALID;
  }

  free(mvmvif);
}

zx_protocol_device_t device_mac_ops = {
    .version = DEVICE_OPS_VERSION,
    .unbind = mac_unbind,
    .release = mac_release,
};

/////////////////////////////////////       PHY       //////////////////////////////////////////////

static zx_status_t phy_query(void* ctx, wlanphy_impl_info_t* info) {
  if (!info) {
    return ZX_ERR_INVALID_ARGS;
  }

  // Returns dummy info for now.
  memset(info, 0, sizeof(*info));

  // TODO(fxb/36682): reads real MAC address from hardware.
  uint8_t fake_mac[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
  memcpy(info->wlan_info.mac_addr, fake_mac, sizeof(info->wlan_info.mac_addr));

  // TODO(fxb/36677): supports AP role
  info->wlan_info.mac_role = WLAN_INFO_MAC_ROLE_CLIENT;

  info->wlan_info.supported_phys =
      WLAN_INFO_PHY_TYPE_DSSS | WLAN_INFO_PHY_TYPE_CCK | WLAN_INFO_PHY_TYPE_OFDM;
  // TODO(fxb/36683): supports HT (802.11n): WLAN_INFO_PHY_TYPE_HT
  // TODO(fxb/36684): suuports VHT (802.11ac): WLAN_INFO_PHY_TYPE_VHT

  info->wlan_info.driver_features = WLAN_INFO_DRIVER_FEATURE_TEMP_DIRECT_SME_CHANNEL;

  // The current band/channel setting is for channel 11 only (in 2.4GHz).
  // TODO(fxb/36689): lists all bands and their channels.
  wlan_info_band_info_t* wlan_band = &info->wlan_info.bands[info->wlan_info.bands_count++];
  wlan_band->band = WLAN_INFO_BAND_2GHZ;
  // See IEEE Std 802.11-2016, 9.4.2.3 for encoding. Those values are:
  //   [1Mbps, 2Mbps, 5.5Mbps, 11Mbps, 6Mbps, 9Mbps, 12Mbps, 18Mbps, 24Mbps, 36Mbps, 48Mbps, 54Mbps]
  uint8_t rates[] = {0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c};
  static_assert(sizeof(rates) <= sizeof(wlan_band->rates), "Too many basic_rates to copy");
  memcpy(wlan_band->rates, rates, sizeof(rates));
  wlan_band->supported_channels.base_freq = 2407;
  wlan_band->supported_channels.channels[0] = 11;

  return ZX_OK;
}

// This function is working with a PHY context ('ctx') to create a MAC interface.
static zx_status_t phy_create_iface(void* ctx, const wlanphy_impl_create_iface_req_t* req,
                                    uint16_t* out_iface_id) {
  struct iwl_trans* iwl_trans = ctx;
  struct iwl_mvm* mvm = iwl_trans_get_mvm(iwl_trans);
  zx_status_t ret = ZX_OK;

  if (!req) {
    IWL_ERR(mvm, "req is not given\n");
    return ZX_ERR_INVALID_ARGS;
  }

  if (req->sme_channel == ZX_HANDLE_INVALID) {
    IWL_ERR(mvm, "the given sme channel is invalid\n");
    return ZX_ERR_INVALID_ARGS;
  }

  if (!mvm) {
    IWL_ERR(mvm, "cannot obtain MVM from ctx=%p while creating interface\n", ctx);
    return ZX_ERR_INVALID_ARGS;
  }

  if (!out_iface_id) {
    IWL_ERR(mvm, "out_iface_id pointer is not given\n");
    return ZX_ERR_INVALID_ARGS;
  }

  mtx_lock(&mvm->mutex);

  // Find the first empty mvmvif slot.
  uint16_t id;
  for (id = 0; id < MAX_NUM_MVMVIF; id++) {
    if (!mvm->mvmvif[id]) {
      break;
    }
  }
  if (id >= MAX_NUM_MVMVIF) {
    IWL_ERR(mvm, "cannot find an empty slot for new MAC interface\n");
    ret = ZX_ERR_NO_RESOURCES;
    goto unlock;
  }

  // Allocate a MAC context. This will be initialized once iwl_mvm_mac_add_interface() is called.
  struct iwl_mvm_vif* mvmvif = calloc(1, sizeof(struct iwl_mvm_vif));
  if (!mvmvif) {
    ret = ZX_ERR_NO_MEMORY;
    goto unlock;
  }

  // Add MAC interface
  device_add_args_t mac_args = {
      .version = DEVICE_ADD_ARGS_VERSION,
      .name = "iwlwifi-wlanmac",
      .ctx = mvmvif,
      .ops = &device_mac_ops,
      .proto_id = ZX_PROTOCOL_WLANMAC,
      .proto_ops = &wlanmac_ops,
  };

  // Add this MAC device into the tree. The parent device is the PHY device.
  ret = device_add(iwl_trans->zxdev, &mac_args, &mvmvif->zxdev);
  if (ret == ZX_OK) {
    mvmvif->mac_role = req->role;
    mvmvif->sme_channel = req->sme_channel;
    mvm->mvmvif[id] = mvmvif;
    mvm->vif_count++;
    *out_iface_id = id;
  }

unlock:
  mtx_unlock(&mvm->mutex);

  return ret;
}

// This function is working with a PHY context ('ctx') to delete a MAC interface.
// The 'id' is the value assigned by phy_create_iface().
static zx_status_t phy_destroy_iface(void* ctx, uint16_t id) {
  struct iwl_trans* iwl_trans = ctx;
  struct iwl_mvm* mvm = iwl_trans_get_mvm(iwl_trans);
  zx_status_t ret = ZX_OK;

  if (!mvm) {
    IWL_ERR(mvm, "cannot obtain MVM from ctx=%p while destroying interface (%d)\n", ctx, id);
    return ZX_ERR_INVALID_ARGS;
  }

  mtx_lock(&mvm->mutex);

  if (id >= MAX_NUM_MVMVIF) {
    IWL_ERR(mvm, "the interface id (%d) is invalid\n", id);
    ret = ZX_ERR_INVALID_ARGS;
    goto unlock;
  }

  struct iwl_mvm_vif* mvmvif = mvm->mvmvif[id];
  if (!mvmvif) {
    IWL_ERR(mvm, "the interface id (%d) has no MAC context\n", id);
    ret = ZX_ERR_NOT_FOUND;
    goto unlock;
  }

  // Only remove the device if it has been added and not removed yet.
  if (mvmvif->zxdev) {
    ret = device_remove_deprecated(mvmvif->zxdev);
    if (ret != ZX_OK) {
      IWL_WARN(mvmvif, "cannot remove the zxdev of interface (%d): %s\n", id,
               zx_status_get_string(ret));
    }
  }

  // Unlink the 'mvmvif' from the 'mvm' and remove the zxdev. The memory of 'mvmvif' will be freed
  // in mac_release().
  mvmvif->zxdev = NULL;
  mvm->vif_count--;
  mvm->mvmvif[id] = NULL;

unlock:
  mtx_unlock(&mvm->mutex);

  return ret;
}

static zx_status_t phy_set_country(void* ctx, const wlanphy_country_t* country) {
  IWL_ERR(ctx, "%s() needs porting ...\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

// PHY interface
wlanphy_impl_protocol_ops_t wlanphy_ops = {
    .query = phy_query,
    .create_iface = phy_create_iface,
    .destroy_iface = phy_destroy_iface,
    .set_country = phy_set_country,
};
