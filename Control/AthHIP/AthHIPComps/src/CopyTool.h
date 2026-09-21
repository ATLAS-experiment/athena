//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHHIPCOMPS_COPYTOOL_H
#define ATHHIPCOMPS_COPYTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/ICopyTool.h"

// VecMem include(s).
#include <vecmem/utils/hip/copy.hpp>

namespace AthHIP {

/// Tool providing a synchronous @c vecmem::copy object for HIP devices
class CopyTool : public extends<AthAlgTool, AthDevice::ICopyTool> {

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
      std::make_shared<const vecmem::hip::copy>()};

};  // class CopyTool

}  // namespace AthHIP

#endif  // ATHHIPCOMPS_COPYTOOL_H
