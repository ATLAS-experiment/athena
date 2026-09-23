//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDASERVICES_PERCOMPONENTSTREAMTOOL_H
#define ATHCUDASERVICES_PERCOMPONENTSTREAMTOOL_H

// Local include(s).
#include "Stream.h"

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthCUDA include(s).
#include "AthCUDAInterfaces/IStreamTool.h"

// System include(s).
#include <memory>

namespace AthCUDA {

/// Tool providing a separate (single) CUDA stream to the component using it
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

  /// Get the CUDA stream to use
  ///
  /// @param ctx The event context for which the stream is requested
  /// @returns The CUDA stream to use for the current event context
  ///
  virtual void* stream(const EventContext& ctx) const override;

  /// @}

 private:
  /// Single CUDA stream
  std::unique_ptr<const Details::Stream> m_stream;

};  // class PerComponentStreamTool

}  // namespace AthCUDA

#endif  // ATHCUDASERVICES_PERCOMPONENTSTREAMTOOL_H
