/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCINTERFACES_IINNERCOINCIDENCETOOL_H
#define L1MUONS1TGCINTERFACES_IINNERCOINCIDENCETOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/TgcSLCandidate.h"

namespace L1Muon::L1TGC {

/** @brief Interface for applying Inner Coincidence to TGC candidates. */
class IInnerCoincidenceTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IInnerCoincidenceTool, 1, 0);

  /**
   * @brief Update candidates with Inner-Coincidence results.
   * @param candidates Candidate collection to update.
   * @param ctx Event context.
   */
  virtual StatusCode apply(TgcSLCandidateContainer& candidates,
                           const EventContext& ctx) const = 0;
};

}  // namespace L1Muon::L1TGC

#endif
