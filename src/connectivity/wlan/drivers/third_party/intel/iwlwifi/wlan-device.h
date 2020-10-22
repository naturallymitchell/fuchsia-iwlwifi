// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The place holder for the code to interact with the MLME.

#ifndef SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_WLAN_DEVICE_H_
#define SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_WLAN_DEVICE_H_

#include <ddk/device.h>
#include <ddk/protocol/wlanphyimpl.h>

#include "garnet/lib/wlan/protocol/include/wlan/protocol/mac.h"
#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/iwl-eeprom-parse.h"

extern wlanmac_protocol_ops_t wlanmac_ops;
extern zx_protocol_device_t device_mac_ops;  // for testing only
extern wlanphy_impl_protocol_ops_t wlanphy_ops;

// for testing
size_t compose_band_list(const struct iwl_nvm_data* nvm_data,
                         wlan_info_band_t bands[WLAN_INFO_BAND_COUNT]);
void fill_band_infos(const struct iwl_nvm_data* nvm_data, const wlan_info_band_t* bands,
                     size_t bands_count, wlan_info_band_info_t* band_infos);

zx_status_t phy_query(void* ctx, wlanphy_impl_info_t* info);
zx_status_t phy_create_iface(void* ctx, const wlanphy_impl_create_iface_req_t* req,
                             uint16_t* out_iface_id);
zx_status_t phy_destroy_iface(void* ctx, uint16_t id);
zx_status_t phy_set_country(void* ctx, const wlanphy_country_t* country);
zx_status_t phy_get_country(void* ctx, wlanphy_country_t* out_country);

#endif  // SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_WLAN_DEVICE_H_
