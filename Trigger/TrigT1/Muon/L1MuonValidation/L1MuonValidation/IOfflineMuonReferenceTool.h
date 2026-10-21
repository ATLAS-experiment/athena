/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_IOFFLINEMUONREFERENCETOOL_H
#define L1MUONVALIDATION_IOFFLINEMUONREFERENCETOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/StatusCode.h"
#include "xAODMuon/Muon.h"

#include <string>
#include <vector>


namespace L1Muon {

/** @brief Common source of reconstructed offline muons used for validation.
 *
 * The provider deliberately preserves the StoreGate container ordering and
 * performs no physics selection.  Selection, matching and performance cuts
 * belong to downstream ROOT analysis.  Detector-specific extrapolation
 * modules can consume the same provider so that TGC/RPC/MDT reference
 * coordinates remain index-aligned with the common offline-muon branches.
 */
class IOfflineMuonReferenceTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IOfflineMuonReferenceTool, 1, 0);

  /** @brief Return the offline muons in their input-container ordering. */
  virtual StatusCode muons(
      const EventContext& ctx,
      std::vector<const xAOD::Muon*>& references) const = 0;

  /** @brief StoreGate input key used by the reference source. */
  virtual std::string inputKey() const = 0;
};

}  // namespace L1Muon

#endif
