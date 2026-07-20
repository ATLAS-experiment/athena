/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCINTERFACES_ITGCL0INNERCOINCIDENCETOOL_H
#define L0MUONS1TGCINTERFACES_ITGCL0INNERCOINCIDENCETOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "L0MuonS1TGCToolInterfaces/TgcL0Candidate.h"

namespace L0Muon {

/**
 * @brief Interface for applying Inner Coincidence to transient TGC candidates.
 *
 * Implementations update the supplied candidates in place using information
 * from detectors inside the magnetic field.  Inner Coincidence may reassign
 * the operative TGC pT threshold and update coincidence or quality state; it
 * does not perform the final Track Selector ordering.  Implementations must
 * not retain event-dependent state between calls.
 */
class ITgcL0InnerCoincidenceTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITgcL0InnerCoincidenceTool, 1, 0);

  /**
   * @brief Apply Inner Coincidence to the transient candidates of one event.
   * @param candidates Candidates to update in place.
   * @param ctx Event context.
   * @return Success when the candidates were processed and remain valid for
   *         the Track Selector stage.
   */
  virtual StatusCode apply(TgcL0CandidateContainer& candidates,
                           const EventContext& ctx) const = 0;
};

}  // namespace L0Muon

#endif
