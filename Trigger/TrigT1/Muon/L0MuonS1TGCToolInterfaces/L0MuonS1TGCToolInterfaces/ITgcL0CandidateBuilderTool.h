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

/** @brief Interface for building pre-Inner-Coincidence TGC candidates. */
class ITgcL0CandidateBuilderTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITgcL0CandidateBuilderTool, 1, 0);

  /**
   * @brief Build candidates from TGC RDO data.
   * @param rdos Input TGC RDO container.
   * @param candidates Output candidate collection.
   * @param ctx Event context.
   */
  virtual StatusCode build(const TgcRdoContainer& rdos,
                           TgcL0CandidateContainer& candidates,
                           const EventContext& ctx) const = 0;
};

}  // namespace L0Muon

#endif
