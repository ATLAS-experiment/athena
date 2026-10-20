/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_ITRUTHMUONREFERENCETOOL_H
#define L1MUONVALIDATION_ITRUTHMUONREFERENCETOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/StatusCode.h"

#include <string>
#include <vector>

namespace L1Muon {

/** @brief Subsystem-independent selected generator-muon record.
 *
 * The record deliberately contains only the generator information needed by
 * common validation output and detector-specific reference extrapolations.
 * Detector surface definitions and extrapolated coordinates do not belong in
 * this common type.
 */
struct TruthMuonReference {
  int pdgId{0};
  int barcode{0};
  double ptMeV{0.0};
  double eta{0.0};
  double phi{0.0};
  int charge{0};
};

/** @brief Common source of the selected truth-muon reference collection.
 *
 * All subsystem validation modules consume the same tool instance.  This
 * centralizes the truth collection key, selection and deterministic ordering,
 * while leaving TGC/RPC/MDT target surfaces entirely subsystem-specific.
 */
class ITruthMuonReferenceTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITruthMuonReferenceTool, 1, 0);

  virtual StatusCode selectedMuons(
      const EventContext& ctx,
      std::vector<TruthMuonReference>& references) const = 0;

  /** @brief StoreGate input key used by the reference source.
   *
   * MuonTester branches use this only to declare their scheduler dependency;
   * event access and selection remain owned by the common reference tool.
   */
  virtual std::string inputKey() const = 0;
};

}  // namespace L1Muon

#endif
