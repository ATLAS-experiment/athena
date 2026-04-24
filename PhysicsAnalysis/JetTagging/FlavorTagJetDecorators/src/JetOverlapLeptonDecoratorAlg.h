/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVORTAGJETDECORATORS_JETOVERLAPLEPTON_DECORATORALG_H
#define FLAVORTAGJETDECORATORS_JETOVERLAPLEPTON_DECORATORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "MCTruthClassifier/MCTruthClassifierDefs.h"


namespace FlavorTagJetDecorators {

  /// Decorate jets with a flag indicating overlap with a truth lepton
  /// from W/Z/top decay.
  ///
  /// For each jet, checks whether any truth electron or muon satisfies:
  ///   - status == 1
  ///   - classifierParticleOrigin in {WBoson, ZBoson, top}
  ///     (values from MCTruthPartClassifier::ParticleOrigin)
  ///   - pT > PtMinimum
  ///   - |eta| < AbsEtaMaximum
  ///   - DeltaR(jet, lepton) < OverlapDR
  ///
  /// Writes a char decoration (1 = overlap, 0 = no overlap).
  /// Replaces the overlapLepton check in TDD's JetTruthAssociator,
  /// enabling TruthElectrons and TruthMuons removal from the DAOD.
  class JetOverlapLeptonDecoratorAlg : public AthReentrantAlgorithm {
  public:
    JetOverlapLeptonDecoratorAlg(const std::string& name,
                                  ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    // Input containers
    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{
      this, "JetContainer", "AntiKt4EMPFlowJets", "Jet container"};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthElectronKey{
      this, "TruthElectronContainer", "TruthElectrons",
        "Truth electron container"};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthMuonKey{
      this, "TruthMuonContainer", "TruthMuons",
        "Truth muon container"};

    // Input decorations: classifierParticleOrigin on the truth leptons
    // (written by MCTruthClassifier, a downstream algorithm).
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_truthElectronOriginKey{
      this, "TruthElectronOriginKey", m_truthElectronKey,
        "classifierParticleOrigin",
        "MCTruthClassifier origin on truth electrons"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_truthMuonOriginKey{
      this, "TruthMuonOriginKey", m_truthMuonKey,
        "classifierParticleOrigin",
        "MCTruthClassifier origin on truth muons"};

    // Selection properties
    Gaudi::Property<float> m_overlapDR{
      this, "OverlapDR", 0.4f,
        "Maximum DeltaR(jet, lepton) for overlap"};
    Gaudi::Property<float> m_ptMinimum{
      this, "PtMinimum", 2000.f,
        "Minimum lepton pT [MeV]"};
    Gaudi::Property<float> m_absEtaMaximum{
      this, "AbsEtaMaximum", 2.9f,
        "Maximum |eta| for lepton selection"};

    // Output decoration
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_hasOverlapLepton{
      this, "hasOverlapLeptonKey", m_jetKey, "ftag_hasOverlapLepton",
        "Whether jet overlaps with a truth lepton from W/Z/top"};
  };

}

#endif // FLAVORTAGJETDECORATORS_JETOVERLAPLEPTON_DECORATORALG_H
