/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCINTERFACES_ITGCL0CANDIDATEBUILDERTOOL_H
#define L0MUONS1TGCINTERFACES_ITGCL0CANDIDATEBUILDERTOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "L0MuonS1TGCToolInterfaces/TgcL0Candidate.h"
#include "MuonRDO/TgcRdoContainer.h"

namespace L0Muon {

/**
 * @brief Interface for constructing transient pre-Inner-Coincidence TGC
 *        candidates.
 *
 * Implementations consume the TGC RDO data for one event and append the
 * candidates reconstructed before Inner Coincidence to the supplied transient
 * container.  The transient candidates are internal to the L0MuonS1TGC
 * processing chain and are not the downstream xAOD::TGCCandData output.
 * Implementations must not retain event-dependent state between calls.
 */
class ITgcL0CandidateBuilderTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITgcL0CandidateBuilderTool, 1, 0);

  /**
   * @brief Build transient pre-Inner-Coincidence candidates from TGC RDO data.
   * @param rdos Input TGC RDO container for the current event.
   * @param candidates Container to which reconstructed candidates are appended.
   * @param ctx Event context.
   * @return Success when the input was processed and the output container is
   *         valid.
   */
  virtual StatusCode build(const TgcRdoContainer& rdos,
                           TgcL0CandidateContainer& candidates,
                           const EventContext& ctx) const = 0;
};

}  // namespace L0Muon

#endif
