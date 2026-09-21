//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHHIPCOMPS_PEREVENTANDCOMPONENTSTREAMTOOL_H
#define ATHHIPCOMPS_PEREVENTANDCOMPONENTSTREAMTOOL_H

// Local include(s).
#include "Stream.h"

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/SlotSpecificObj.h"

// AthHIP include(s).
#include "AthHIPInterfaces/IStreamTool.h"

// System include(s).
#include <memory>

namespace AthHIP {

/// Tool providing a separate, per-event HIP streams to the component using it
///
/// So that all components would use separate streams, and separate in all
/// events as well. Creating/using a large number of streams as a result.
///
class PerEventAndComponentStreamTool : public extends<AthAlgTool, IStreamTool> {

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
  /// Slot specific HIP stream(s)
  std::unique_ptr<
      const SG::SlotSpecificObj<Details::Stream, SG::InvalidSlot::Enabled>>
      m_streams;

};  // class PerEventAndComponentStreamTool

}  // namespace AthHIP

#endif  // ATHHIPCOMPS_PEREVENTANDCOMPONENTSTREAMTOOL_H
