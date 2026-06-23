//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDASERVICES_SINGLESTREAMSVC_H
#define ATHCUDASERVICES_SINGLESTREAMSVC_H

// Local include(s).
#include "Stream.h"

// Framework include(s).
#include "AthenaBaseComps/AthService.h"

// AthCUDA include(s).
#include "AthCUDAInterfaces/IStreamSvc.h"

// System include(s).
#include <memory>

namespace AthCUDA {

/// Service providing CUDA streams to (reentrant) algorithms
///
/// In a very simple way. By having one CUDA stream. Period.
///
class SingleStreamSvc : public extends<AthService, IStreamSvc> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c IStreamSvc
  /// @{

  /// Get the CUDA stream to use
  ///
  /// @param ctx The event context for which the stream is requested
  /// @returns The CUDA stream to use for the current event context
  ///
  virtual cudaStream_t stream(const EventContext& ctx) const override;

  /// @}

 private:
  /// Single CUDA stream
  std::unique_ptr<const Details::Stream> m_stream;

};  // class SingleStreamSvc

}  // namespace AthCUDA

#endif  // ATHCUDASERVICES_SINGLESTREAMSVC_H
