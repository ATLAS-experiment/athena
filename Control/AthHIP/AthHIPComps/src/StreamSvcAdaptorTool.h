//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHHIPCOMPS_STREAMSVCADAPTORTOOL_H
#define ATHHIPCOMPS_STREAMSVCADAPTORTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

// AthHIP include(s).
#include "AthHIPInterfaces/IStreamSvc.h"
#include "AthHIPInterfaces/IStreamTool.h"

namespace AthHIP {

/// Tool exposing a HIP stream service as a HIP stream tool
///
/// Client components should ideally only use HIP streams through the
/// @c AthHIP::IStreamTool interface. (For possible future flexibility.)
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

  /// Get the HIP stream to use
  ///
  /// @param ctx The event context for which the stream is requested
  /// @returns The HIP stream to use for the current event context
  ///
  virtual hipStream_t stream(const EventContext& ctx) const override;

  /// @}

 private:
  /// Handle to the service providing the HIP streams
  ServiceHandle<IStreamSvc> m_svc{
      this, "StreamSvc", "", "Service providing the 'adapted' HIP streams"};

};  // class StreamSvcAdaptorTool

}  // namespace AthHIP

#endif  // ATHHIPCOMPS_STREAMSVCADAPTORTOOL_H
