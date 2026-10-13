/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCINTERFACES_ITGCL0TRACKSELECTORTOOL_H
#define L1MUONS1TGCINTERFACES_ITGCL0TRACKSELECTORTOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/TgcSLCandidate.h"
#include "xAODTrigL1Muon/L1TGCCandDataContainer.h"

namespace L1Muon {

/** @brief Interface for selecting TGC candidates and filling L1TGCCandData. */
class ITgcL0TrackSelectorTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITgcL0TrackSelectorTool, 1, 0);

  /**
   * @brief Select candidates and fill the output container.
   * @param candidates Input candidate collection.
   * @param output Output L1TGCCandData container.
   * @param ctx Event context.
   */
  virtual StatusCode select(const TgcSLCandidateContainer& candidates,
                            xAOD::L1TGCCandDataContainer& output,
                            const EventContext& ctx) const = 0;
};

}  // namespace L1Muon

#endif
