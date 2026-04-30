//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_HOSTCOPYTOOL_H
#define ATHDEVICECOMPS_HOSTCOPYTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/ICopyTool.h"

// System include(s).
#include <memory>

namespace AthDevice {

/// Tool providing a copy object for host->host copies
class HostCopyTool : public extends<AthAlgTool, ICopyTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c ICopyTool
  /// @{

  virtual vecmem::copy& copy() const override;

  /// @}

 private:
  /// The copy object that this tool provides
  std::unique_ptr<vecmem::copy> m_copy;

};  // class HostCopyTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_HOSTCOPYTOOL_H
