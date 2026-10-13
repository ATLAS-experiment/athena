/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCINTERFACES_ITGCL0CANDIDATEBUILDERTOOL_H
#define L1MUONS1TGCINTERFACES_ITGCL0CANDIDATEBUILDERTOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/TgcSLCandidate.h"
#include "L1MuonS1TGCToolInterfaces/TgcSLSegment.h"
#include "MuonRDO/TgcRdoContainer.h"

namespace L1Muon {

/** @brief Interface for building pre-Inner-Coincidence TGC candidates. */
class ITgcL0CandidateBuilderTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITgcL0CandidateBuilderTool, 1, 1);

  /**
   * @brief Build candidates from TGC RDO data.
   * @param rdos Input TGC RDO container.
   * @param candidates Output candidate collection.
   * @param ctx Event context.
   */
  virtual StatusCode build(const TgcRdoContainer& rdos,
                           TgcSLCandidateContainer& candidates,
                           const EventContext& ctx) const = 0;

  /**
   * @brief Build candidates and optionally expose projection segments.
   *
   * The default implementation preserves existing candidate-builder tools.
   * Implementations that can expose their native segments may override this
   * overload.  The segment pointer is an optional validation hook and does not
   * change production output.
   */
  virtual StatusCode build(const TgcRdoContainer& rdos,
                           TgcSLCandidateContainer& candidates,
                           TgcSLSegmentContainer* segments,
                           const EventContext& ctx) const {
    static_cast<void>(segments);
    return build(rdos, candidates, ctx);
  }
};

}  // namespace L1Muon

#endif
