// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/platform/wlan-softmac-device.h"

#include <zircon/assert.h>
#include <zircon/status.h>

#include <memory>

extern "C" {
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/mvm/mvm.h"
}  // extern "C"

#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/platform/ieee80211.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/platform/mvm-mlme.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/platform/mvm-sta.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/platform/scoped_utils.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/platform/stats.h"

#define CHECK_DELETE_IN_PROGRESS_WITHOUT_ERRCODE(mvmvif)                \
  do {                                                                  \
    if (mvmvif->delete_in_progress) {                                   \
      IWL_WARN(mvmvif, "Interface is in the process of being deleted"); \
      return;                                                           \
    }                                                                   \
  } while (0)

#define CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif)                   \
  do {                                                                  \
    if (mvmvif->delete_in_progress) {                                   \
      IWL_WARN(mvmvif, "Interface is in the process of being deleted"); \
      return ZX_ERR_BAD_STATE;                                          \
    }                                                                   \
  } while (0)

namespace wlan::iwlwifi {

WlanSoftmacDevice::WlanSoftmacDevice(zx_device* parent, iwl_trans* drvdata, uint16_t iface_id,
                                     struct iwl_mvm_vif* mvmvif)
    : ddk::Device<WlanSoftmacDevice, ddk::Initializable, ddk::Unbindable>(parent),
      mvmvif_(mvmvif),
      drvdata_(drvdata),
      iface_id_(iface_id) {}

WlanSoftmacDevice::~WlanSoftmacDevice() = default;

zx_status_t WlanSoftmacDevice::WlanSoftmacQuery(wlan_softmac_info_t* out_info) {
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  return mac_query(mvmvif_, out_info);
}

void WlanSoftmacDevice::WlanSoftmacQueryDiscoverySupport(discovery_support_t* out_resp) {
  CHECK_DELETE_IN_PROGRESS_WITHOUT_ERRCODE(mvmvif_);
  mac_query_discovery_support(out_resp);
}

void WlanSoftmacDevice::WlanSoftmacQueryMacSublayerSupport(mac_sublayer_support_t* out_resp) {
  CHECK_DELETE_IN_PROGRESS_WITHOUT_ERRCODE(mvmvif_);
  mac_query_mac_sublayer_support(out_resp);
}

void WlanSoftmacDevice::WlanSoftmacQuerySecuritySupport(security_support_t* out_resp) {
  CHECK_DELETE_IN_PROGRESS_WITHOUT_ERRCODE(mvmvif_);
  mac_query_security_support(out_resp);
}

void WlanSoftmacDevice::WlanSoftmacQuerySpectrumManagementSupport(
    spectrum_management_support_t* out_resp) {
  CHECK_DELETE_IN_PROGRESS_WITHOUT_ERRCODE(mvmvif_);
  mac_query_spectrum_management_support(out_resp);
}

zx_status_t WlanSoftmacDevice::WlanSoftmacStart(const wlan_softmac_ifc_protocol_t* ifc,
                                                zx::channel* out_mlme_channel) {
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  return mac_start(mvmvif_, ifc, (zx_handle_t*)out_mlme_channel);
}

void WlanSoftmacDevice::WlanSoftmacStop() {
  ap_mvm_sta_.reset();
  mac_stop(mvmvif_);
}

zx_status_t WlanSoftmacDevice::WlanSoftmacQueueTx(const wlan_tx_packet_t* packet,
                                                  bool* out_enqueue_pending) {
  iwl_stats_inc(IWL_STATS_CNT_DATA_FROM_MLME);

  // Delayed transmission is never used right now.
  *out_enqueue_pending = false;

  if (ap_mvm_sta_ == nullptr) {
    return ZX_ERR_BAD_STATE;
  }
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);

  if (packet->mac_frame_size > WLAN_MSDU_MAX_LEN) {
    IWL_ERR(mvmvif_, "Frame size is to large (%lu). expect less than %lu.\n",
            packet->mac_frame_size, WLAN_MSDU_MAX_LEN);
    return ZX_ERR_INVALID_ARGS;
  }

  ieee80211_mac_packet mac_packet = {};
  mac_packet.common_header =
      reinterpret_cast<const ieee80211_frame_header*>(packet->mac_frame_buffer);
  mac_packet.header_size = ieee80211_get_header_len(mac_packet.common_header);
  if (mac_packet.header_size > packet->mac_frame_size) {
    IWL_ERR(mvmvif_, "TX packet header size %zu too large for data size %zu\n",
            mac_packet.header_size, packet->mac_frame_size);
    return ZX_ERR_INVALID_ARGS;
  }

  mac_packet.body = packet->mac_frame_buffer + mac_packet.header_size;
  mac_packet.body_size = packet->mac_frame_size - mac_packet.header_size;
  if (ieee80211_pkt_is_protected(mac_packet.common_header)) {
    switch (ieee80211_get_frame_type(mac_packet.common_header)) {
      case ieee80211_frame_type::IEEE80211_FRAME_TYPE_MGMT:
        mac_packet.info.control.hw_key = ap_mvm_sta_->GetKey(WLAN_KEY_TYPE_IGTK);
        break;
      case ieee80211_frame_type::IEEE80211_FRAME_TYPE_DATA:
        mac_packet.info.control.hw_key = ap_mvm_sta_->GetKey(WLAN_KEY_TYPE_PAIRWISE);
        break;
      default:
        break;
    }
  }

  auto lock = std::lock_guard(mvmvif_->mvm->mutex);
  return iwl_mvm_mac_tx(mvmvif_, ap_mvm_sta_->iwl_mvm_sta(), &mac_packet);
}

// Reject the request that firmware doesn't allow. See fxb/89911 for more context.
bool WlanSoftmacDevice::IsValidChannel(const wlan_channel_t* channel) {
  if (channel->cbw == CHANNEL_BANDWIDTH_CBW40 || channel->cbw == CHANNEL_BANDWIDTH_CBW40BELOW) {
    if (channel->primary >= 10 && channel->primary <= 14) {
      IWL_WARN(mvmvif_, "The 40%sMHz bandwidth is not supported on the channel %d.\n",
               channel->cbw == CHANNEL_BANDWIDTH_CBW40BELOW ? "-" : "", channel->primary);
      return false;
    }
  }

  if (channel->primary <= 14 && channel->cbw >= CHANNEL_BANDWIDTH_CBW80) {
    IWL_WARN(mvmvif_, "The 80+MHz bandwidth is not supported on the 2.4GHz band (channel %d).\n",
             channel->primary);
    return false;
  }

  return true;
}

zx_status_t WlanSoftmacDevice::WlanSoftmacSetChannel(const wlan_channel_t* channel) {
  zx_status_t status = ZX_OK;

  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);

  if (!IsValidChannel(channel)) {
    return ZX_ERR_NOT_SUPPORTED;
  }

  // If the AP sta already exists, it probably was left from the previous association attempt.
  // Remove it first.
  if (ap_mvm_sta_ != nullptr) {
    if ((status = mac_unconfigure_bss(mvmvif_)) != ZX_OK) {
      return status;
    }
    ap_mvm_sta_.reset();
  }
  return mac_set_channel(mvmvif_, channel);
}

zx_status_t WlanSoftmacDevice::WlanSoftmacConfigureBss(const bss_config_t* config) {
  zx_status_t status = ZX_OK;
  if (ap_mvm_sta_ != nullptr) {
    return ZX_ERR_ALREADY_BOUND;
  }
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  if ((status = mac_configure_bss(mvmvif_, config)) != ZX_OK) {
    return status;
  }

  ZX_DEBUG_ASSERT(mvmvif_->mac_role == WLAN_MAC_ROLE_CLIENT);
  std::unique_ptr<MvmSta> ap_mvm_sta;
  if ((status = MvmSta::Create(mvmvif_, config->bssid, &ap_mvm_sta)) != ZX_OK) {
    return status;
  }

  ap_mvm_sta_ = std::move(ap_mvm_sta);
  return ZX_OK;
}

zx_status_t WlanSoftmacDevice::WlanSoftmacEnableBeaconing(const wlan_bcn_config_t* bcn_cfg) {
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  return mac_enable_beaconing(mvmvif_, bcn_cfg);
}

zx_status_t WlanSoftmacDevice::WlanSoftmacConfigureBeacon(const wlan_tx_packet_t* pkt) {
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  return mac_configure_beacon(mvmvif_, pkt);
}

zx_status_t WlanSoftmacDevice::WlanSoftmacSetKey(const wlan_key_config_t* key_config) {
  if (ap_mvm_sta_ == nullptr) {
    return ZX_ERR_BAD_STATE;
  }
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  return ap_mvm_sta_->SetKey(key_config);
}

zx_status_t WlanSoftmacDevice::WlanSoftmacConfigureAssoc(const wlan_assoc_ctx_t* assoc_ctx) {
  if (ap_mvm_sta_ == nullptr) {
    return ZX_ERR_BAD_STATE;
  }
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  return mac_configure_assoc(mvmvif_, assoc_ctx);
}

zx_status_t WlanSoftmacDevice::WlanSoftmacClearAssoc(
    const uint8_t peer_addr_list[fuchsia_wlan_ieee80211_MAC_ADDR_LEN]) {
  zx_status_t status = ZX_OK;

  if (ap_mvm_sta_ == nullptr) {
    return ZX_ERR_BAD_STATE;
  }
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);

  // Mark the station is no longer associated. This must be set before we start operating on the STA
  // instance.
  mvmvif_->bss_conf.assoc = false;
  ap_mvm_sta_.reset();

  if ((status = mac_clear_assoc(mvmvif_, peer_addr_list)) != ZX_OK) {
    return status;
  }

  return ZX_OK;
}

zx_status_t WlanSoftmacDevice::WlanSoftmacStartPassiveScan(
    const wlan_softmac_passive_scan_args_t* passive_scan_args, uint64_t* out_scan_id) {
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  return mac_start_passive_scan(mvmvif_, passive_scan_args, out_scan_id);
}

zx_status_t WlanSoftmacDevice::WlanSoftmacStartActiveScan(
    const wlan_softmac_active_scan_args_t* active_scan_args, uint64_t* out_scan_id) {
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  return mac_start_active_scan(mvmvif_, active_scan_args, out_scan_id);
}

zx_status_t WlanSoftmacDevice::WlanSoftmacUpdateWmmParams(wlan_ac_t ac,
                                                          const wlan_wmm_params_t* params) {
  IWL_ERR(this, "%s() needs porting\n", __func__);
  CHECK_DELETE_IN_PROGRESS_WITH_ERRCODE(mvmvif_);
  return ZX_ERR_NOT_SUPPORTED;
}

void WlanSoftmacDevice::DdkInit(ddk::InitTxn txn) {
  txn.Reply(mac_init(mvmvif_, drvdata_, zxdev(), iface_id_));
}

void WlanSoftmacDevice::DdkRelease() {
  IWL_DEBUG_INFO(this, "Releasing iwlwifi mac-device\n");
  mac_release(mvmvif_);
  delete this;
}

void WlanSoftmacDevice::DdkUnbind(ddk::UnbindTxn txn) {
  IWL_DEBUG_INFO(this, "Unbinding iwlwifi mac-device\n");
  mac_unbind(mvmvif_);
  txn.Reply();
}

}  // namespace wlan::iwlwifi
