/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GEPJETALG_H
#define TRIGGEPPERF_GEPJETALG_H

/*
  This algorithm creates jets from CaloClusters, and writes them out
   as xAOD::Jets. The origin of the clusters maybe via standard ATLS
   code, or by Gep clustering. The jet strategy is
   carried out by helper objects.
   The strategy used is chosen according to string set at configure time. *
*/



#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODCaloEvent/CaloClusterContainer.h"

#include "xAODJet/JetContainer.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/gFexJetRoIContainer.h"

// JetTaggerLRJ maker (owns the runtime config + on-the-fly deltaR LUT)
#include "./JetTaggerLRJMaker.h"

//Athena::Units::GeV
#include "AthenaKernel/Units.h"

#include <string>


class GepJetAlg: public ::AthReentrantAlgorithm {
 public:
  
  GepJetAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ) const override;    
  

 private:

  Gaudi::Property<std::string> m_jetAlgName{this, "jetAlgName", "",
      "Gep jet alg idenfifier"};

  Gaudi::Property<bool> m_configureLRJMaker{this, "EnableLRJMaker", false,
      "Build and configure the JetTaggerLRJ maker in initialize(). Set by "
      "GepJetTaggerLRJAlgCfg; must agree with jetAlgName == 'JetTaggerLRJ'."};

  SG::ReadHandleKey< xAOD::CaloClusterContainer> m_caloClustersKey {
    this, "caloClustersKey", "", "key to read in a CaloCluster constainer"};

  SG::ReadHandleKey<xAOD::jFexSRJetRoIContainer> m_jFexSRJetsKey {
    this, "jFexSRJetRoIs", "L1_jFexSRJetRoISim", "key to read a L1 jet container"};
  
  SG::WriteHandleKey<xAOD::JetContainer> m_outputGepJetsKey{
    this, "outputJetsKey", "",
    "key for xAOD:Jet wrappers for GepJets"};

  // WTAConeMaker parameters
  Gaudi::Property<float> m_WTAConstEtCut{this, "WTAConstEtCut", 2.0,
    "Minimum Et for a tower to be considered as a constituent"};

  Gaudi::Property<float> m_WTASeedEtCut{this, "WTASeedEtCut", 5.0,
      "Minimum Et for a tower to be considered as a seed"};

  Gaudi::Property<float> m_WTAJet_dR{this, "WTAJet_dR", 0.4,
      "Jet radius to determine TOB-Jet association"};

  Gaudi::Property<unsigned int> m_WTAMaxConstN{this, "WTAMaxConstN", 205,
      "Maximum number of constituents per jet"};

  Gaudi::Property<unsigned int> m_WTAMaxSeedSortingN{this, "WTAMaxSeedSortingN", 50,
      "Maximum number of seeds to sort"};

  Gaudi::Property<unsigned int> m_WTABlockN{this, "WTABlockN", 4,
      "Number of blocks to divide the input towers into for parallel processing. Options: 1, 4"};

  Gaudi::Property<std::string> m_WTASeedCleaningName{this, "WTASeedCleaningName", "TwoPass",
      "Seed cleaning algorithm to use. Options: Baseline, TwoPass"};

// ------------------------------------------------------------------
// JetTaggerLRJ (modified seeded-cone large-R jet) algorithm wiring.
// The algorithm body (JetTaggerLRJMaker) currently emits a placeholder
// LRJ per seed; the substructure stages are still being filled in.
// ------------------------------------------------------------------
  SG::ReadHandleKey<xAOD::JetContainer> m_lrjWTAConeSeedsKey {
    this, "LRJWTAConeSeedsKey", "",
    "WTACone small-R jets used as JetTaggerLRJ seeds (xAOD::JetContainer)."};

  SG::ReadHandleKey<xAOD::gFexJetRoIContainer> m_lrjGFexSRJetsKey {
    this, "LRJgFexSRJetRoIs", "L1_gFexSRJetRoISim",
    "gFEX small-R jet RoIs used as JetTaggerLRJ seeds."};

  // jFEX SR-jet seeds reuse m_jFexSRJetsKey above.

  Gaudi::Property<std::string> m_LRJSeedSource{this, "LRJSeedSource", "WTACone",
      "Source of JetTaggerLRJ seeds. Options: WTACone, jFexSRJ, gFexSRJ"};

  Gaudi::Property<std::string> m_LRJConstSource{this, "LRJConstSource", "Towers",
      "Source of JetTaggerLRJ constituents. Options: Towers, WTACone"};

  // ---- preset selector ----
  Gaudi::Property<unsigned int> m_LRJAlgoVersion{this, "LRJAlgoVersion", 3,
      "Algorithm variant / preset: 2 = basic (BasicV2), 3 = advanced (AdvancedV3)."};

  // ---- geometry (concrete defaults; <= 0 : inherit from preset instead) ----
  Gaudi::Property<float> m_LRJJetR{this, "LRJJetR", 1.1,
      "Large-R jet cone radius; r2Cut = LRJJetR^2. Default 1.1 (r2Cut=1.21). <=0 inherits the preset."};

  Gaudi::Property<float> m_LRJDSearch{this, "LRJDSearch", 2.0,
      "Seed-position-optimization search distance (rMergeCut); ignored when v2 (basic). "
      "Default 2.0. <0 inherits the preset (which is 0.001/disabled for basic)."};

  // ---- multiplicity overrides (0 : inherit from preset) ----
  Gaudi::Property<unsigned int> m_LRJNSeedsInput{this, "LRJNSeedsInput", 0,
      "Number of input seeds considered per event. 0 inherits the preset."};

  Gaudi::Property<unsigned int> m_LRJNProtoSeeds{this, "LRJNProtoSeeds", 0,
      "Number of proto-seeds for seed-position optimization. 0 inherits the preset."};

  Gaudi::Property<unsigned int> m_LRJNSeedsOutput{this, "LRJNSeedsOutput", 0,
      "Number of output LRJs per event (typically 2). 0 inherits the preset."};

  Gaudi::Property<unsigned int> m_LRJMaxObjectsConsidered{this, "LRJMaxObjectsConsidered", 0,
      "Maximum number of constituents to load per event. 0 inherits the preset."};

  // ---- digitization overrides: bit lengths (0 : inherit from preset) ----
  Gaudi::Property<unsigned int> m_LRJEtBitLength{this, "LRJEtBitLength", 0,
      "Et field bit length. 0 inherits the preset."};
  Gaudi::Property<unsigned int> m_LRJEtaBitLength{this, "LRJEtaBitLength", 0,
      "Eta field bit length. 0 inherits the preset."};
  Gaudi::Property<unsigned int> m_LRJPhiBitLength{this, "LRJPhiBitLength", 0,
      "Phi field bit length. 0 inherits the preset."};
  Gaudi::Property<unsigned int> m_LRJNumSubjetsLength{this, "LRJNumSubjetsLength", 0,
      "n-subjets field bit length. 0 inherits the preset."};
  Gaudi::Property<unsigned int> m_LRJNSubjetinessBitLength{this, "LRJNSubjetinessBitLength", 0,
      "tau_1 / tau_2 field bit length. 0 inherits the preset."};
  Gaudi::Property<unsigned int> m_LRJMassApproxBitLength{this, "LRJMassApproxBitLength", 0,
      "massApprox field bit length. 0 inherits the preset."};
  Gaudi::Property<unsigned int> m_LRJPsiRBitLength{this, "LRJPsiRBitLength", 0,
      "psi_R / deltaR-LUT field bit length. 0 inherits the preset."};
  Gaudi::Property<unsigned int> m_LRJDeltaRLutLength{this, "LRJDeltaRLutLength", 0,
      "deltaR granularity bit length. 0 inherits the preset."};

  // ---- digitization overrides: physical ranges (-9999 : inherit) ----
  Gaudi::Property<float> m_LRJPhiMin{this, "LRJPhiMin", -9999.0, "phi range minimum. -9999 inherits the preset."};
  Gaudi::Property<float> m_LRJPhiMax{this, "LRJPhiMax", -9999.0, "phi range maximum. -9999 inherits the preset."};
  Gaudi::Property<float> m_LRJEtaMin{this, "LRJEtaMin", -9999.0, "eta range minimum. -9999 inherits the preset."};
  Gaudi::Property<float> m_LRJEtaMax{this, "LRJEtaMax", -9999.0, "eta range maximum. -9999 inherits the preset."};
  Gaudi::Property<float> m_LRJEtMin{this, "LRJEtMin", -9999.0, "Et range minimum (GeV). -9999 inherits the preset."};
  Gaudi::Property<float> m_LRJEtMax{this, "LRJEtMax", -9999.0, "Et range maximum (GeV). -9999 inherits the preset."};
  Gaudi::Property<float> m_LRJMassApproxMax{this, "LRJMassApproxMax", -1.0, "massApprox range maximum (GeV). <=0 inherits the preset."};
  Gaudi::Property<float> m_LRJInputEtToGeV{this, "LRJInputEtToGeV", -1.0, "Input Et (MeV) -> GeV scale. <=0 inherits the preset (1e-3)."};

  // ---- thresholds (<0 : inherit from preset) ----
  Gaudi::Property<float> m_LRJSubjetEtThresholdGeV{this, "LRJSubjetEtThresholdGeV", -1.0,
      "Minimum subjet Et (GeV) for counting nSubjets. <0 inherits the preset."};

  Gaudi::Property<float> m_LRJMinEtSeedPosOptCutGeV{this, "LRJMinEtSeedPosOptCutGeV", -1.0,
      "Minimum proto-seed Et (GeV) for seed-position optimization. <0 inherits the preset."};

  // ---- reserved (per-object Et cuts not yet applied, matching the emulation) ----
  Gaudi::Property<float> m_LRJSeedEtCutGeV{this, "LRJSeedEtCutGeV", 5.0,
      "Minimum seed Et in GeV (reserved; not yet applied)."};
  Gaudi::Property<float> m_LRJConstEtCutGeV{this, "LRJConstEtCutGeV", 2.0,
      "Minimum constituent Et in GeV (reserved; not yet applied)."};

  // ---- flow toggles (always applied; v2 ignores OR / seed-opt) ----
  Gaudi::Property<bool> m_LRJEnableOverlapRemoval{this, "LRJEnableOverlapRemoval", true,
      "Enable overlap removal between the leading two seeds (advanced only)."};
  Gaudi::Property<bool> m_LRJEnableEtWeightedMidpoint{this, "LRJEnableEtWeightedMidpoint", false,
      "Use Et-weighted midpoint in seed-position optimization."};
  Gaudi::Property<bool> m_LRJMinEtSeedPosOptimization{this, "LRJMinEtSeedPosOptimization", true,
      "Apply the minimum proto-seed Et cut in seed-position optimization."};

  // ---- output toggles (always applied) ----
  Gaudi::Property<bool> m_LRJWriteSubstructure{this, "LRJWriteSubstructure", true,
      "Fill psi_R / tau_1 / tau_2 / massApprox / nSubjets on the output LRJ."};
  Gaudi::Property<bool> m_LRJWriteSubjetKinematics{this, "LRJWriteSubjetKinematics", true,
      "Fill per-subjet (et, eta, phi) vectors on the output LRJ."};
  Gaudi::Property<bool> m_LRJWriteConstituentIndices{this, "LRJWriteConstituentIndices", true,
      "Fill constituentsIndices / mergedIndices on the output LRJ."};

  // Configured once in initialize() (builds the deltaR LUT); used read-only in execute().
  Gep::JetTaggerLRJMaker m_lrjMaker;

  StatusCode configureLRJMaker();

  // Load JetTaggerLRJ seeds from an input collection into `seeds`. `kin` maps a
  // collection element to (pt, eta, phi, m), hiding the per-source accessor
  // differences (xAOD::Jet uses pt()/m(); the FEX RoIs use et() and mass 0).
  template <typename Container, typename KinFn>
  StatusCode loadLRJSeeds(const SG::ReadHandleKey<Container>& key,
                          const EventContext& ctx,
                          KinFn&& kin,
                          std::vector<Gep::Jet>& seeds) const;

};

#endif //> !TRIGGEPPERF_GEPJETALG_H
