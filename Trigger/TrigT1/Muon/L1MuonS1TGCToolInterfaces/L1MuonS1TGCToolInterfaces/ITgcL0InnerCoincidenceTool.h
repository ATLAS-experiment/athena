/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCINTERFACES_ITGCL0INNERCOINCIDENCETOOL_H
#define L1MUONS1TGCINTERFACES_ITGCL0INNERCOINCIDENCETOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/TgcL0Candidate.h"

namespace L0Muon {

/** @brief Interface for applying Inner Coincidence to TGC candidates. */
class ITgcL0InnerCoincidenceTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITgcL0InnerCoincidenceTool, 1, 0);

  /**
   * @brief Update candidates with Inner-Coincidence results.
   * @param candidates Candidate collection to update.
   * @param ctx Event context.
   */
  virtual StatusCode apply(TgcL0CandidateContainer& candidates,
                           const EventContext& ctx) const = 0;
};

}  // namespace L0Muon

#endif
