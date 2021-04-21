// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_PCIE_PCIE_DEVICE_H_
#define SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_PCIE_PCIE_DEVICE_H_

#include <lib/ddk/device.h>

#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/device.h"

struct iwl_trans;

namespace wlan::iwlwifi {

// This class uses the DDKTL classes to manage the lifetime of a iwlwifi driver instance.
class PcieDevice : public Device {
 public:
  PcieDevice(const PcieDevice& device) = delete;
  PcieDevice& operator=(const PcieDevice& other) = delete;
  virtual ~PcieDevice() override;

  // Creates and binds PcieDevice instance. On success hands device off to device lifecycle
  // management.
  static zx_status_t Create(zx_device_t* parent_device, bool load_firmware);

  // Device implementation.
  iwl_trans* drvdata() override;
  const iwl_trans* drvdata() const override;
  void DdkInit(::ddk::InitTxn txn) override;
  void DdkUnbind(::ddk::UnbindTxn txn) override;

 protected:
  explicit PcieDevice(zx_device_t* parent, iwl_trans* iwl_trans);
  iwl_trans* drvdata_ = nullptr;
};

}  // namespace wlan::iwlwifi

#endif  // SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_PCIE_PCIE_DEVICE_H_
