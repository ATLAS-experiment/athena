//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHHIPCOMPS_PEREVENTSTREAMSVC_H
#define ATHHIPCOMPS_PEREVENTSTREAMSVC_H

// Local include(s).
#include "Stream.h"

// Framework include(s).
#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/SlotSpecificObj.h"

// AthHIP include(s).
#include "AthHIPInterfaces/IStreamSvc.h"

// System include(s).
#include <memory>

namespace AthHIP {

/// Service providing HIP streams to (reentrant) algorithms
///
/// In a very simple way. By having one HIP stream per concurrent event (slot),
/// and having algorithms all use a single HIP stream for a given event.
///
class PerEventStreamSvc : public extends<AthService, IStreamSvc> {

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

  /// Get the HIP stream to use
  ///
  /// @param ctx The event context for which the stream is requested
  /// @returns The HIP stream to use for the current event context
  ///
  virtual hipStream_t stream(const EventContext& ctx) const override;

  /// @}

 private:
  /// Slot specific HIP stream(s)
  std::unique_ptr<
      const SG::SlotSpecificObj<Details::Stream, SG::InvalidSlot::Enabled>>
      m_streams;

};  // class PerEventStreamSvc

}  // namespace AthHIP

#endif  // ATHHIPCOMPS_PEREVENTSTREAMSVC_H
