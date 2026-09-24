//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDASERVICES_PEREVENTSTREAMSVC_H
#define ATHCUDASERVICES_PEREVENTSTREAMSVC_H

// Local include(s).
#include "Stream.h"

// Framework include(s).
#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/SlotSpecificObj.h"

// AthCUDA include(s).
#include "AthCUDAInterfaces/IStreamSvc.h"

// System include(s).
#include <memory>

namespace AthCUDA {

/// Service providing CUDA streams to (reentrant) algorithms
///
/// In a very simple way. By having one CUDA stream per concurrent event (slot),
/// and having algorithms all use a single CUDA stream for a given event.
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

  /// Get the CUDA stream to use
  ///
  /// @param ctx The event context for which the stream is requested
  /// @returns The CUDA stream to use for the current event context
  ///
  virtual void* stream(const EventContext& ctx) const override;

  /// @}

 private:
  /// Slot specific CUDA stream(s)
  std::unique_ptr<
      const SG::SlotSpecificObj<Details::Stream, SG::InvalidSlot::Enabled>>
      m_streams;

};  // class PerEventStreamSvc

}  // namespace AthCUDA

#endif  // ATHCUDASERVICES_PEREVENTSTREAMSVC_H
