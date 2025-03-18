// This is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PROMPT_DECORATEPLIT_H
#define PROMPT_DECORATEPLIT_H

// Local
#include "DecoratePromptLeptonImproved.h"

// Tools
#include "PathResolver/PathResolver.h"
#include "FlavorTagDiscriminants/SaltModel.h"

// Athena
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "AthContainers/Decorator.h"

// xAOD
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"


namespace Prompt {

  // Definition of the 4-momentum type
  typedef TLorentzVector FourMom_t;

  class DecoratePLIT: public AthReentrantAlgorithm {

  public:
    DecoratePLIT(const std::string &name, ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute (const EventContext&) const override;

  private:
    std::shared_ptr<const FlavorTagDiscriminants::SaltModel> m_saltModel{};
    std::shared_ptr<const FlavorTagDiscriminants::SaltModel> m_saltModel_endcap{};

    int m_num_lepton_features{};
    int m_num_track_features{};

    StatusCode initializeAccessors();

    StatusCode predictElec(const xAOD::Electron &electron,
                           const xAOD::TrackParticleContainer &tracks,
                           const xAOD::CaloClusterContainer &caloclusters,
                           std::vector<SG::WriteDecorHandle<xAOD::ElectronContainer, float>> &dec_el_plit_output,
                           const EventContext& ctx) const;

    StatusCode predictMuon(const xAOD::Muon &muon,
                           const xAOD::TrackParticleContainer &tracks,
                           std::vector<SG::WriteDecorHandle<xAOD::MuonContainer, float>> &dec_mu_plit_output,
                           const EventContext& ctx) const;

    bool passed_r22tracking_cuts(const xAOD::TrackParticle &tp, const EventContext& ctx) const;
  
    StatusCode decorateTrack(const xAOD::TrackParticle& track,
                           float dr_lepton,
                           bool isUsedForElectron,
                           bool isUsedForMuon,
                           const xAOD::TrackParticle* trackLep) const;

    StatusCode fillParticles(std::vector<const xAOD::IParticle *> &parts,
                           const xAOD::IParticle &lepton,
                           const xAOD::TrackParticle *trackLep,
                           const xAOD::TrackParticleContainer &trackContainer,
                           const EventContext& ctx) const;

    // Properties:
    Gaudi::Property<std::string> m_leptonsName {
      this, "LeptonContainerName", "",
      "Container's name of the lepton that you want to decorate. Also need to set ElectronContainerKey or MuonContainerKey accordingly"
    };
    Gaudi::Property<std::string> m_configPath {this, "ConfigPath", "", "Path of the directory containing the onnx files"};
    Gaudi::Property<std::string> m_configFileVersion {this, "ConfigFileVersion", "", "Vector of tagger score files"};
    Gaudi::Property<std::string> m_configFileVersion_endcap {this, "ConfigFileVersion_endcap", "", "Vector of tagger score files for endcap"};
    Gaudi::Property<std::string> m_TaggerName {this, "TaggerName", "", "Tagger name"};
    Gaudi::Property<float> m_maxLepTrackdR{
      this, "maxLepTrackdR", 0.4, "Maximum distance between lepton and track"};
    Gaudi::Property<float> m_lepCalErelConeSize{
      this, "lepCalErelConeSize", 0.15, "Cone size for relative calo cluster energy sum"};
    Gaudi::Property<std::string> m_btagIp_prefix{
      this, "btagIp_prefix", "btagIp_", "Prefix of b-tagging impact parameter variables (w.r.t. primary vertex)"};

    // Read/write handles
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronsKey {
      this, "ElectronContainerKey", "Electrons",
      "Electron container name"
    };
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonsKey {
      this, "MuonContainerKey", "Muons",
      "Muon container name"
    };
    SG::ReadHandleKey<xAOD::JetContainer> m_trackjetsKey {
      this, "TrackJetContainerKey", "AntiKtVR30Rmax4Rmin02PV0TrackJets",
      "VR track jet container name"
    };
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tracksKey {
      this, "TracksContainerKey", "InDetTrackParticles",
      "Tracks container name"
    };
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_caloclustersKey {
      this, "CaloClusterContainerKey", "egammaClusters",
      "Calo cluster container name"
    };

    // Accessor declarations
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_acc_mu_ptvarcone30TTVA {this, "acc_mu_ptvarcone30TTVA",m_muonsKey, "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500"};
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_acc_mu_topoetcone30 {this, "acc_mu_topoetcone30",m_muonsKey, "topoetcone30"};

    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_acc_el_ptvarcone30 {this,"acc_el_ptvarcone30",m_electronsKey, "ptvarcone30"};
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_acc_el_topoetcone30 {this,"acc_el_topoetcone30",m_electronsKey, "topoetcone30"};

    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_acc_trk_dr_lepton {this, "acc_trk_dr_lepton",m_tracksKey, "dr_lepton"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_acc_trk_dr_leptontrack {this, "acc_trk_dr_leptontrack",m_tracksKey, "dr_leptontrack"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_acc_trk_d0 {this,"acc_trk_d0", m_tracksKey, m_btagIp_prefix + "d0"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_acc_trk_z0SinTheta {this, "acc_trk_z0SinTheta", m_tracksKey, m_btagIp_prefix + "z0SinTheta"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_acc_trk_d0Uncertainty {this, "acc_trk_d0Uncertainty", m_tracksKey, m_btagIp_prefix + "d0Uncertainty"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_acc_trk_z0SinThetaUncertainty {this, "acc_trk_z0SinThetaUncertainty", m_tracksKey, m_btagIp_prefix + "z0SinThetaUncertainty"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_acc_trk_muon_track {this, "acc_trk_muon_track", m_tracksKey, "muon_track"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_acc_trk_electron_track {this,"acc_trk_electron_track", m_tracksKey, "electron_track"};

    SG::WriteDecorHandleKeyArray<xAOD::ElectronContainer>  m_dec_el_plit_output{this, "PLITelOutput", {}};
    SG::WriteDecorHandleKeyArray<xAOD::MuonContainer> m_dec_mu_plit_output{this, "PLITmuOutput", {}};

    const SG::Decorator<float> m_dec_trk_dr_lepton{"dr_lepton"};
    const SG::Decorator<char> m_dec_trk_electron_track{"electron_track"};
    const SG::Decorator<char> m_dec_trk_muon_track{"muon_track"};
    const SG::Decorator<float> m_dec_trk_dr_leptontrack{"dr_leptontrack"};
  };
}

#endif
