/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCINTERFACES_ITGCL0TRACKSELECTORTOOL_H
#define L0MUONS1TGCINTERFACES_ITGCL0TRACKSELECTORTOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "L0MuonS1TGCToolInterfaces/TgcL0Candidate.h"
#include "xAODL0MuonCand/TGCCandDataContainer.h"

namespace L0Muon {

/**
 * @brief Interface for final TGC candidate ordering and downstream xAOD output.
 *
 * Implementations consume the transient post-Inner-Coincidence candidates,
 * apply the Track Selector ordering, and fill the supplied
 * xAOD::TGCCandDataContainer.  The output container is owned and recorded by
 * the calling algorithm.  Implementations must not retain event-dependent
 * state between calls.
 */
class ITgcL0TrackSelectorTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITgcL0TrackSelectorTool, 1, 0);

  /**
   * @brief Select candidates and fill the downstream TGC candidate container.
   * @param candidates Input transient post-Inner-Coincidence candidates.
   * @param output Output xAOD container owned by the calling algorithm.
   * @param ctx Event context.
   * @return Success when selection and output conversion completed.
   */
  virtual StatusCode select(const TgcL0CandidateContainer& candidates,
                            xAOD::TGCCandDataContainer& output,
                            const EventContext& ctx) const = 0;
};

}  // namespace L0Muon

#endif
