/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_TRUTHMUONREFERENCETOOL_H
#define L1MUONVALIDATION_TRUTHMUONREFERENCETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GeneratorObjects/McEventCollection.h"
#include "L1MuonValidation/ITruthMuonReferenceTool.h"
#include "L1MuonValidation/TruthMuonSelection.h"
#include "StoreGate/ReadHandleKey.h"

#include <string>

namespace L1Muon {

/** @brief Common generator-truth reference provider for muon validation. */
class TruthMuonReferenceTool final : public AthAlgTool,
                                     virtual public ITruthMuonReferenceTool {
 public:
  TruthMuonReferenceTool(const std::string& type, const std::string& name,
                         const IInterface* parent);

  StatusCode initialize() override;
  StatusCode selectedMuons(
      const EventContext& ctx,
      std::vector<TruthMuonReference>& references) const override;
  std::string inputKey() const override;

 private:
  SG::ReadHandleKey<McEventCollection> m_truthEventKey{
      this, "TruthEventKey", "TruthEvent", "Input generator truth collection"};

  Gaudi::Property<double> m_minTruthPt{
      this, "MinTruthPt", TruthMuonSelectionConfig{}.minPtMeV,
      "Minimum generator-muon pT in MeV; no acceptance cut by default"};
  Gaudi::Property<double> m_minTruthAbsEta{
      this, "MinTruthAbsEta", TruthMuonSelectionConfig{}.minAbsEta,
      "Minimum absolute generator-muon eta; includes the barrel by default"};
  Gaudi::Property<double> m_maxTruthAbsEta{
      this, "MaxTruthAbsEta", TruthMuonSelectionConfig{}.maxAbsEta,
      "Maximum absolute generator-muon eta; unbounded by default"};
  Gaudi::Property<int> m_requiredTruthStatus{
      this, "RequiredTruthStatus", 1,
      "Generator status required for truth muons"};
  Gaudi::Property<int> m_maxAbsTruthBarcode{
      this, "MaxAbsTruthBarcode", 9999,
      "Maximum absolute generator barcode for truth muons"};
};

}  // namespace L1Muon

#endif
