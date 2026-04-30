/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTH_TAU_DECORATOR_ALG_H
#define TRUTH_TAU_DECORATOR_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "AsgTools/ToolHandle.h"

#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TauAnalysisTools/ITauTruthMatchingTool.h"


namespace FlavorTagDiscriminants {

  /// Match jets to truth taus and decorate with tau properties.
  ///
  /// For each jet, finds the nearest isolated truth tau (classifierParticleType
  /// == IsoTau) within MaxDeltaR using the tau's visible 4-momentum
  /// (pt_vis, eta_vis, phi_vis, m_vis). Decorates jets with:
  ///   - matchedTo{Suffix}:                       char (1/0)
  ///   - isHadronicTauFrom{Suffix}:               int (1/0/-1)
  ///   - decayModeFrom{Suffix}:                   int (decay mode / -1)
  ///   - classifierParticleOutComeFrom{Suffix}:   uint (prong enum / 0)
  ///   - deltaPtTo{Suffix}:                       float (jet pT - tau pt_vis)
  ///   - pt_visFrom{Suffix}:                      double (tau pt_vis)
  ///
  /// Replaces TDD's TruthTauMatcher ca_block at derivation time.
  class TruthTauDecoratorAlg : public AthReentrantAlgorithm {
  public:
    TruthTauDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    using JC = xAOD::JetContainer;
    using TPC = xAOD::TruthParticleContainer;

    SG::ReadHandleKey<JC> m_jetKey{
      this, "JetContainer", "AntiKt4EMPFlowJets", "Jet container"};
    SG::ReadHandleKey<TPC> m_truthTauKey{
      this, "TruthTauContainer", "TruthTaus", "Truth tau container"};

    Gaudi::Property<float> m_maxDeltaR{
      this, "MaxDeltaR", 0.3f, "Max delta R for matching"};

    ToolHandle<TauAnalysisTools::ITauTruthMatchingTool> m_tauTruthTool{
      this, "TauTruthMatchingTool",
      "TauAnalysisTools::TauTruthMatchingTool/TauTruthMatchingTool",
      "Tool to compute truth tau decay mode"};

    // Output decoration keys
    SG::WriteDecorHandleKey<JC> m_dec_matched{
      this, "matchedToKey", m_jetKey, "matchedToTruthTaus",
        "Whether jet is matched to a truth tau"};
    SG::WriteDecorHandleKey<JC> m_dec_isHadTau{
      this, "isHadronicTauKey", m_jetKey, "isHadronicTauFromTruthTaus",
        "Whether matched tau is hadronic"};
    SG::WriteDecorHandleKey<JC> m_dec_decayMode{
      this, "decayModeKey", m_jetKey, "decayModeFromTruthTaus",
        "Decay mode of matched tau"};
    SG::WriteDecorHandleKey<JC> m_dec_deltaPt{
      this, "deltaPtKey", m_jetKey, "deltaPtToTruthTaus",
        "Jet pT minus tau visible pT"};
    SG::WriteDecorHandleKey<JC> m_dec_ptVis{
      this, "ptVisKey", m_jetKey, "pt_visFromTruthTaus",
        "Visible pT of matched tau"};
    SG::WriteDecorHandleKey<JC> m_dec_classifierOutcome{
      this, "classifierOutComeKey", m_jetKey,
        "classifierParticleOutComeFromTruthTaus",
        "Classifier particle outcome of matched tau"};
  };

}

#endif
