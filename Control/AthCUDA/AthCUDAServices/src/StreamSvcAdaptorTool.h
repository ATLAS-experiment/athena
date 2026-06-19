//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDASERVICES_STREAMSVCADAPTORTOOL_H
#define ATHCUDASERVICES_STREAMSVCADAPTORTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

// AthCUDA include(s).
#include "AthCUDAInterfaces/IStreamSvc.h"
#include "AthCUDAInterfaces/IStreamTool.h"

namespace AthCUDA {

/// Tool exposing a CUDA stream service as a CUDA stream tool
///
/// Client components should ideally only use CUDA streams through the
/// @c AthCUDA::IStreamTool interface. (For possible future flexibility.)
/// But, at least initially, streams will just be provided by a relatively
/// simple service.
///
/// This tool allows us to expose such services with a tool interface.
///
class StreamSvcAdaptorTool : public extends<AthAlgTool, IStreamTool> {

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
  virtual cudaStream_t stream(const EventContext& ctx) const override;

  /// @}

 private:
  /// Handle to the service providing the CUDA streams
  ServiceHandle<IStreamSvc> m_svc{
      this, "StreamSvc", "", "Service providing the 'adapted' CUDA streams"};

};  // class StreamSvcAdaptorTool

}  // namespace AthCUDA

#endif  // ATHCUDASERVICES_STREAMSVCADAPTORTOOL_H
