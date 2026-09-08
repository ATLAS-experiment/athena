//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHHIPCOMPS_ASYNCCOPYTOOL_H
#define ATHHIPCOMPS_ASYNCCOPYTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/ICopyTool.h"

// AthHIP include(s).
#include "AthHIPInterfaces/IStreamTool.h"

namespace AthHIP {

/// Tool providing an asynchronous @c vecmem::copy object for HIP devices
class AsyncCopyTool : public extends<AthAlgTool, AthDevice::ICopyTool> {

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

  /// Get the provided @c vecmem::copy object
  virtual std::shared_ptr<const vecmem::copy> copy(
      const EventContext& ctx) const override;

  /// @}

 private:
  /// Tool to get the current HIP stream from
  ToolHandle<IStreamTool> m_streamTool{
      this, "StreamTool", "", "Tool to get the current HIP stream from"};

};  // class AsyncCopyTool

}  // namespace AthHIP

#endif  // ATHHIPCOMPS_ASYNCCOPYTOOL_H
