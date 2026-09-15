//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHHIPCOMPS_PERCOMPONENTSTREAMTOOL_H
#define ATHHIPCOMPS_PERCOMPONENTSTREAMTOOL_H

// Local include(s).
#include "Stream.h"

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthHIP include(s).
#include "AthHIPInterfaces/IStreamTool.h"

// System include(s).
#include <memory>

namespace AthHIP {

/// Tool providing a separate (single) HIP stream to the component using it
///
/// This means that in algorithms using this tool, operations from different
/// events would end up being executed in a single stream. But operations from
/// different components/algorithms could be executed in parallel.
///
class PerComponentStreamTool : public extends<AthAlgTool, IStreamTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c IStreamTool
  /// @{

  /// Get the HIP stream to use
  ///
  /// @param ctx The event context for which the stream is requested
  /// @returns The HIP stream to use for the current event context
  ///
  virtual hipStream_t stream(const EventContext& ctx) const override;

  /// @}

 private:
  /// Single HIP stream
  std::unique_ptr<const Details::Stream> m_stream;

};  // class PerComponentStreamTool

}  // namespace AthHIP

#endif  // ATHHIPCOMPS_PERCOMPONENTSTREAMTOOL_H
