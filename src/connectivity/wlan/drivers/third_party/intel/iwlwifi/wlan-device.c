// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The place holder for the driver code to interact with the MLME.
//
//                                 devmgr
//                                   |
//                                   v
//          MLME  === channel ===   SME
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
// Note that the '*ctx' in this file may refer to:
//
//   - 'struct iwl_trans*' for PHY ops.
//   - 'struct iwl_mvm_vif*' for MAC ops.
//
//
// Sme_channel
//
//   The steps below briefly describe how the 'sme_channel' is used and transferred. In short,
//   the goal is to let SME and MLME have a channel to communicate with each other.
//
//   + After the devmgr (the device manager in wlanstack) detects a PHY device, the devmgr first
//     creates an SME instance in order to handle the MAC operation later. Then the devmgr
//     establishes a channel and passes one end to the SME instance.
//
//   + The devmgr requests the PHY device to create a MAC interface. In the request, the other end
//     of channel is passed to the driver.
//
//   + The driver's phy_create_iface() gets called, and saves the 'sme_channel' handle in the newly
//     created MAC context.
//
//   + Once the MAC device is added, its mac_start() will be called. Then it will transfer the
//     'sme_channel' handle back to the MLME.
//
//   + Now, both sides of channel (SME and MLME) can talk now.
//

#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/wlan-device.h"

#include <stdio.h>
#include <string.h>
#include <zircon/status.h>

#include <ddk/device.h>
#include <ddk/driver.h>

#include "garnet/lib/wlan/protocol/include/wlan/protocol/ieee80211.h"
#include "garnet/lib/wlan/protocol/include/wlan/protocol/mac.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/iwl-debug.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/mvm/mvm.h"

////////////////////////////////////  Helper Functions  ////////////////////////////////////////////

//
// Given a NVM data structure, and return the list of bands.
//
// Returns:
//   size_t: # of bands enabled in the NVM data.
//   bands[]: contains the list of enabled bands.
//
size_t compose_band_list(const struct iwl_nvm_data* nvm_data,
                         wlan_info_band_t bands[WLAN_INFO_BAND_COUNT]) {
  size_t bands_count = 0;

  if (nvm_data->sku_cap_band_24ghz_enable) {
    bands[bands_count++] = WLAN_INFO_BAND_2GHZ;
  }
  if (nvm_data->sku_cap_band_52ghz_enable) {
    bands[bands_count++] = WLAN_INFO_BAND_5GHZ;
  }
  ZX_ASSERT(bands_count <= WLAN_INFO_BAND_COUNT);

  return bands_count;
}

//
// Given a NVM data, copy the band and channel info into the 'wlan_info_band_info_t' structure.
//
// - 'bands_count' is the number of bands in 'bands[]'.
// - 'band_infos[]' must have at least bands_count for this function to write.
//
void fill_band_infos(const struct iwl_nvm_data* nvm_data, const wlan_info_band_t* bands,
                     size_t bands_count, wlan_info_band_info_t* band_infos) {
  ZX_ASSERT(bands_count <= ARRAY_SIZE(nvm_data->bands));

  for (size_t band_idx = 0; band_idx < bands_count; ++band_idx) {
    wlan_info_band_t band_id = bands[band_idx];
    const struct ieee80211_supported_band* sband = &nvm_data->bands[band_id];  // source
    wlan_info_band_info_t* band_info = &band_infos[band_idx];                  // destination

    band_info->band = band_id;
    band_info->ht_supported = nvm_data->sku_cap_11n_enable;
    // TODO(43517): Better handling of driver features bits/flags
    band_info->ht_caps.ht_capability_info =
        IEEE80211_HT_CAPS_CHAN_WIDTH | IEEE80211_HT_CAPS_SMPS_DYNAMIC;
    band_info->ht_caps.ampdu_params = (3 << IEEE80211_AMPDU_RX_LEN_SHIFT) |  // (64K - 1) bytes
                                      (6 << IEEE80211_AMPDU_DENSITY_SHIFT);  // 8 us
    // TODO(36683): band_info->ht_caps->supported_mcs_set =
    // TODO(36684): band_info->vht_caps =

    ZX_ASSERT(sband->n_bitrates <= (int)ARRAY_SIZE(band_info->rates));
    for (int rate_idx = 0; rate_idx < sband->n_bitrates; ++rate_idx) {
      band_info->rates[rate_idx] = cfg_rates_to_80211(sband->bitrates[rate_idx]);
    }

    // Fill the channel list of this band.
    wlan_info_channel_list_t* ch_list = &band_info->supported_channels;
    switch (band_info->band) {
      case WLAN_INFO_BAND_2GHZ:
        ch_list->base_freq = 2407;
        break;
      case WLAN_INFO_BAND_5GHZ:
        ch_list->base_freq = 5000;
        break;
      default:
        ZX_ASSERT(0);  // Unknown band ID.
        break;
    }
    ZX_ASSERT(sband->n_channels <= (int)ARRAY_SIZE(ch_list->channels));
    for (int ch_idx = 0; ch_idx < sband->n_channels; ++ch_idx) {
      ch_list->channels[ch_idx] = sband->channels[ch_idx].ch_num;
    }
  }
}

/////////////////////////////////////       MAC       //////////////////////////////////////////////

static zx_status_t mac_query(void* ctx, uint32_t options, wlanmac_info_t* info) {
  struct iwl_mvm_vif* mvmvif = ctx;

  if (!ctx || !info) {
    return ZX_ERR_INVALID_ARGS;
  }
  memset(info, 0, sizeof(*info));

  ZX_ASSERT(mvmvif->mvm);
  ZX_ASSERT(mvmvif->mvm->nvm_data);
  struct iwl_nvm_data* nvm_data = mvmvif->mvm->nvm_data;

  memcpy(info->ifc_info.mac_addr, nvm_data->hw_addr, sizeof(info->ifc_info.mac_addr));
  info->ifc_info.mac_role = mvmvif->mac_role;
  // TODO(43517): Better handling of driver features bits/flags
  info->ifc_info.driver_features = WLAN_INFO_DRIVER_FEATURE_TEMP_DIRECT_SME_CHANNEL;
  info->ifc_info.supported_phys = WLAN_INFO_PHY_TYPE_DSSS | WLAN_INFO_PHY_TYPE_CCK |
                                  WLAN_INFO_PHY_TYPE_OFDM | WLAN_INFO_PHY_TYPE_HT;
  info->ifc_info.caps = WLAN_INFO_HARDWARE_CAPABILITY_SHORT_PREAMBLE |
                        WLAN_INFO_HARDWARE_CAPABILITY_SPECTRUM_MGMT |
                        WLAN_INFO_HARDWARE_CAPABILITY_SHORT_SLOT_TIME;

  // Determine how many bands this adapter supports.
  wlan_info_band_t bands[WLAN_INFO_BAND_COUNT];
  info->ifc_info.bands_count = compose_band_list(nvm_data, bands);

  fill_band_infos(nvm_data, bands, info->ifc_info.bands_count, info->ifc_info.bands);

  return ZX_OK;
}

static zx_status_t mac_start(void* ctx, const wlanmac_ifc_protocol_t* ifc,
                             zx_handle_t* out_sme_channel) {
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
  struct iwl_mvm_vif* mvmvif = ctx;

  zx_status_t ret = iwl_mvm_mac_remove_interface(mvmvif);
  if (ret != ZX_OK) {
    IWL_ERR(mvmvif, "Cannot remove MAC interface: %s\n", zx_status_get_string(ret));
  }
}

static zx_status_t mac_queue_tx(void* ctx, uint32_t options, wlan_tx_packet_t* pkt) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_set_channel(void* ctx, uint32_t options, const wlan_channel_t* chan) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_configure_bss(void* ctx, uint32_t options, const wlan_bss_config_t* config) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_enable_beaconing(void* ctx, uint32_t options,
                                        const wlan_bcn_config_t* bcn_cfg) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_configure_beacon(void* ctx, uint32_t options, const wlan_tx_packet_t* pkt) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_set_key(void* ctx, uint32_t options, const wlan_key_config_t* key_config) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_configure_assoc(void* ctx, uint32_t options,
                                       const wlan_assoc_ctx_t* assoc_ctx) {
  IWL_ERR(ctx, "%s() needs porting ... see fxb/36742\n", __func__);
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_status_t mac_clear_assoc(void* ctx, uint32_t options, const uint8_t* peer_addr,
                                   size_t peer_addr_size) {
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
  struct iwl_trans* iwl_trans = ctx;
  struct iwl_mvm* mvm = iwl_trans_get_mvm(iwl_trans);
  if (!mvm || !info) {
    return ZX_ERR_INVALID_ARGS;
  }

  struct iwl_nvm_data* nvm_data = mvm->nvm_data;
  ZX_ASSERT(nvm_data);

  memset(info, 0, sizeof(*info));

  memcpy(info->wlan_info.mac_addr, nvm_data->hw_addr, sizeof(info->wlan_info.mac_addr));

  // TODO(fxb/36677): supports AP role
  info->wlan_info.mac_role = WLAN_INFO_MAC_ROLE_CLIENT;

  // TODO(43517): Better handling of driver features bits/flags
  info->wlan_info.supported_phys =
      WLAN_INFO_PHY_TYPE_DSSS | WLAN_INFO_PHY_TYPE_CCK | WLAN_INFO_PHY_TYPE_OFDM;
  // TODO(fxb/36683): supports HT (802.11n): WLAN_INFO_PHY_TYPE_HT
  // TODO(fxb/36684): suuports VHT (802.11ac): WLAN_INFO_PHY_TYPE_VHT

  info->wlan_info.driver_features = WLAN_INFO_DRIVER_FEATURE_TEMP_DIRECT_SME_CHANNEL;

  // TODO(43517): Better handling of driver features bits/flags
  info->wlan_info.caps = WLAN_INFO_HARDWARE_CAPABILITY_SHORT_PREAMBLE |
                         WLAN_INFO_HARDWARE_CAPABILITY_SPECTRUM_MGMT |
                         WLAN_INFO_HARDWARE_CAPABILITY_SHORT_SLOT_TIME;

  // Determine how many bands this adapter supports.
  wlan_info_band_t bands[WLAN_INFO_BAND_COUNT];
  info->wlan_info.bands_count = compose_band_list(nvm_data, bands);

  fill_band_infos(nvm_data, bands, info->wlan_info.bands_count, info->wlan_info.bands);

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
    mvmvif->mvm = mvm;
    mvmvif->mac_role = req->role;
    mvmvif->sme_channel = req->sme_channel;
    mvm->mvmvif[id] = mvmvif;
    *out_iface_id = id;
  }

unlock:
  mtx_unlock(&mvm->mutex);

  return ret;
}

// This function is working with a PHY context ('ctx') to delete a MAC interface ('id').
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
  mvm->mvmvif[id] = NULL;

  // the last MAC interface. stop the MVM to save power. 'vif_count' had been decreased in
  // iwl_mvm_mac_remove_interface().
  if (mvm->vif_count == 0) {
    __iwl_mvm_mac_stop(mvm);
  }

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
