/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACK_LEPTON_DECORATOR_ALG_HH
#define TRACK_LEPTON_DECORATOR_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "ElectronPhotonSelectorTools/AsgElectronLikelihoodTool.h"



namespace FlavorTagDiscriminants {


  class TrackLeptonDecoratorAlg: public AthReentrantAlgorithm {
  public:
    TrackLeptonDecoratorAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ) const override;

  private:

    // electron ID tool
    Gaudi::Property<std::string> m_electronID_wp {
      this, "electronID_wp", "VeryLooseLHElectron",
        "Likelihood working point for electron ID selection"};
    AsgElectronLikelihoodTool m_electronID_tool;

    // Input Containers
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_TrackContainerKey {
      this, "trackContainer", "InDetTrackParticles",
        "Key for the input track collection"};
    SG::ReadHandleKey< xAOD::ElectronContainer > m_ElectronContainerKey {
      this, "electronContainer", "Electrons",
        "Key for the input electron collection"};
    SG::ReadHandleKey< xAOD::MuonContainer > m_MuonContainerKey {
      this, "muonContainer", "Muons",
        "Key for the input muon collection"};

    // Decorators for tracks
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_lepton_id {
      this, "leptonID", "leptonID", "pdgID of reconstruction lepton "};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_muon_quality {
      this, "muon_quality", "muon_quality", "Quality of the reconstructed muon: (0=Tight, 1=Medium, 2=Loose, 3=Veryloose, 4=HighPt, 5=LowPtEfficiency)"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_muon_qOverPratio {
      this, "muon_qOverPratio", "muon_qOverPratio", "Ratio between q/p reconstructed by the ID and the MS"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_muon_momentumBalanceSignificance {
      this, "muon_momentumBalanceSignificance", "muon_momentumBalanceSignificance", "Significance of the momentum balance between ID and MS"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_muon_scatteringNeighbourSignificance {
      this, "muon_scatteringNeighbourSignificance", "muon_scatteringNeighbourSignificance", "Significance of the azimuthal angular difference between the two half tracks ending/starting at each of adjacent hist along the track"};

  };

}

#endif
