/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SOFT_ELECTRON_SELECTION_ALG_HH
#define SOFT_ELECTRON_SELECTION_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODBase/IParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODEgamma/ElectronContainer.h"

namespace FlavorTagJetDecorators {

  /// Apply soft electron selection cuts on each jet's FTagElectrons
  /// and write the passing electrons as GhostFTagSelectedElectrons.
  ///
  /// Replicates the 14 cuts from TDD's SoftElectronSelector, using
  /// pre-computed ftag_ decorations (from SoftElectronDecoratorAlg)
  /// instead of dereferencing caloCluster.  This allows egammaClusters
  /// to be dropped from the DAOD output.

  class SoftElectronSelectionAlg : public AthReentrantAlgorithm {
  public:
    SoftElectronSelectionAlg(const std::string& name,
                             ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;

  private:
    bool passedCuts(const xAOD::Jet& jet,
                    const xAOD::Electron& el,
                    float energyOverP,
                    float et,
                    float isoOverPt,
                    float dpop) const;

    // Input
    SG::ReadHandleKey<xAOD::JetContainer> m_jetContainerKey {
      this, "jetContainer", "AntiKt4EMPFlowJets",
      "Key for the input jet collection"};

    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronContainerKey {
      this, "electronContainer", "Electrons",
      "Key for the input electron collection"};

    // Input decoration on jets: FTag ghost-associated electrons
    // (written by the b-tagging chain, a downstream algorithm).
    SG::ReadDecorHandleKey<xAOD::JetContainer> m_ghostElectronsKey {
      this, "ghostElectronsKey", m_jetContainerKey, "FTagElectrons",
      "Input decoration: FTag ghost-associated electron links per jet"};

    // Output decoration on jets
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_selectedElectronsKey {
      this, "selectedElectronsKey", "AntiKt4EMPFlowJets.GhostFTagSelectedElectrons",
      "Output decoration: selected electron links per jet"};

    // Read decorations on electrons (from SoftElectronDecoratorAlg)
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_energyOverPKey {
      this, "energyOverPKey", "Electrons.ftag_energyOverP",
      "E/p decoration on electrons"};

    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_etKey {
      this, "etKey", "Electrons.ftag_et",
      "ET decoration on electrons"};

    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_isoOverPtKey {
      this, "isoOverPtKey", "Electrons.ftag_ptVarCone30OverPt",
      "Isolation/pT decoration on electrons"};

    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_dpopKey {
      this, "dpopKey", "Electrons.ftag_deltaPOverP",
      "deltaPOverP decoration on electrons"};

    // Selection cut thresholds (defaults match GN3_lite.json)
    Gaudi::Property<float> m_ptMinimum {
      this, "ptMinimum", 1000., "Minimum electron pT [MeV]"};
    Gaudi::Property<float> m_ptMaximum {
      this, "ptMaximum", 500000., "Maximum electron pT [MeV]"};
    Gaudi::Property<float> m_absEtaMaximum {
      this, "absEtaMaximum", 2.5, "Maximum |eta|"};
    Gaudi::Property<float> m_d0Maximum {
      this, "d0Maximum", 1., "Maximum |d0| [mm]"};
    Gaudi::Property<float> m_eopMaximum {
      this, "eopMaximum", 30., "Maximum |E/p|"};
    Gaudi::Property<float> m_etMaximum {
      this, "etMaximum", 50000., "Maximum ET [MeV]"};
    Gaudi::Property<float> m_isoptMaximum {
      this, "isoptMaximum", 40., "Maximum |ptVarCone30/pT|"};
    Gaudi::Property<float> m_ptrelMaximum {
      this, "ptrelMaximum", 5000., "Maximum ptrel [MeV]"};
    Gaudi::Property<float> m_rhad1Maximum {
      this, "rhad1Maximum", 4., "Maximum |Rhad1|"};
    Gaudi::Property<float> m_wstotMaximum {
      this, "wstotMaximum", 20., "Maximum |wtots1|"};
    Gaudi::Property<float> m_rphiMaximum {
      this, "rphiMaximum", 2., "Maximum |Rphi|"};
    Gaudi::Property<float> m_retaMaximum {
      this, "retaMaximum", 2., "Maximum |Reta|"};
    Gaudi::Property<float> m_deta1Maximum {
      this, "deta1Maximum", 10., "Maximum |deltaEta1|"};
    Gaudi::Property<float> m_dpopMaximum {
      this, "dpopMaximum", 5., "Maximum |deltaPOverP|"};
  };

}  // namespace FlavorTagJetDecorators

#endif
