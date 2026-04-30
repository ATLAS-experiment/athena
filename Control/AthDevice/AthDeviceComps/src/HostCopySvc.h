//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_HOSTCOPYSVC_H
#define ATHDEVICECOMPS_HOSTCOPYSVC_H

// Framework include(s).
#include "AthenaBaseComps/AthService.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/ICopySvc.h"

// System include(s).
#include <memory>

namespace AthDevice {

/// Service providing a copy object for host->host copies
class HostCopySvc : public extends<AthService, ICopySvc> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthService
  /// @{

  /// Initialize the service
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c ICopySvc
  /// @{

  virtual vecmem::copy& copy() const override;

  /// @}

 private:
  /// The copy object that this service provides
  std::unique_ptr<vecmem::copy> m_copy;

};  // class HostCopySvc

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_HOSTCOPYSVC_H
