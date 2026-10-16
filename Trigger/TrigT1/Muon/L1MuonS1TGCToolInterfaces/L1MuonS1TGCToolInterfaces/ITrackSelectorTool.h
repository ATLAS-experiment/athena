/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCINTERFACES_ITRACKSELECTORTOOL_H
#define L1MUONS1TGCINTERFACES_ITRACKSELECTORTOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/TgcSLCandidate.h"
#include "xAODTrigL1Muon/L1TGCCandDataContainer.h"

namespace L1Muon::L1TGC {

/** @brief Interface for selecting TGC candidates and filling L1TGCCandData. */
class ITrackSelectorTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITrackSelectorTool, 1, 0);

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

}  // namespace L1Muon::L1TGC

#endif
