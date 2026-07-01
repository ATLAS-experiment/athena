//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_HOSTCOPYTOOL_H
#define ATHDEVICECOMPS_HOSTCOPYTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/ICopyTool.h"

namespace AthDevice {

/// Tool providing a basic, host-side @c vecmem::copy object
class HostCopyTool : public extends<AthAlgTool, ICopyTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c ICopyTool
  /// @{

  /// Get the provided @c vecmem::copy object
  virtual std::shared_ptr<const vecmem::copy> copy(
      const EventContext& ctx) const override;

  /// @}

 private:
  /// The @c vecmem::copy object provided by this tool
  std::shared_ptr<const vecmem::copy> m_copy{
      std::make_shared<const vecmem::copy>()};

};  // class HostCopyTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_HOSTCOPYTOOL_H
