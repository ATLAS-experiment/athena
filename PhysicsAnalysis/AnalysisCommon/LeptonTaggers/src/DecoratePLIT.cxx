// This is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LeptonTaggers/DecoratePLIT.h"
#include "AthContainers/ConstDataVector.h"
#include "xAODEgamma/ElectronxAODHelpers.h"
#include <AsgDataHandles/ReadDecorHandle.h>


namespace Prompt {

  DecoratePLIT::DecoratePLIT(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode DecoratePLIT::initialize() {
    ATH_MSG_DEBUG("Initializing DecoratePLIT " << name() );
    ATH_MSG_DEBUG("m_leptonsName = " << m_leptonsName);

    ATH_MSG_DEBUG("Initializing " << m_electronsKey);
    ATH_MSG_DEBUG("Initializing " << m_muonsKey);
    ATH_MSG_DEBUG("Initializing " << m_trackjetsKey);
    ATH_MSG_DEBUG("Initializing " << m_tracksKey);
    ATH_MSG_DEBUG("Initializing " << m_caloclustersKey);

    ATH_CHECK(m_electronsKey.initialize(m_leptonsName == "Electrons"));
    ATH_CHECK(m_muonsKey.initialize(m_leptonsName  == "Muons"));
    ATH_CHECK(m_trackjetsKey.initialize());
    ATH_CHECK(m_tracksKey.initialize());
    ATH_CHECK(m_caloclustersKey.initialize());

    // initialise accessors
    ATH_CHECK(initializeAccessors());

    // Load and initialize the neural network model from the given file path.
    if(m_leptonsName == "Electrons") {
        std::string fullPathToOnnxFile = PathResolverFindCalibFile(m_configPath.value() + m_configFileVersion.value());
        m_saltModel = std::make_shared<FlavorTagDiscriminants::SaltModel>(fullPathToOnnxFile);

        std::string fullPathToOnnxFile_endcap = PathResolverFindCalibFile(m_configPath.value() + m_configFileVersion_endcap.value());
        m_saltModel_endcap = std::make_shared<FlavorTagDiscriminants::SaltModel>(fullPathToOnnxFile_endcap);

        m_num_lepton_features = 6;
        m_num_track_features = 18;

        // set up decorators using a dummy query of the onnx model
        std::map<std::string, FlavorTagDiscriminants::Inputs> gnn_input;
        
        std::vector<float> elec_feat(m_num_lepton_features, 0.);
        std::vector<int64_t> elec_feat_dim = {1, static_cast<int64_t>(elec_feat.size())};
        FlavorTagDiscriminants::Inputs elec_info (elec_feat, elec_feat_dim);
        gnn_input.insert({"jet_features", elec_info}); // need to use the "jet_features" keyword as we are borrowing flavour tagging code
        
        std::vector<float> track_feat(m_num_track_features, 0.);
        std::vector<int64_t> track_feat_dim = {1, m_num_track_features};
        FlavorTagDiscriminants::Inputs track_info(track_feat, track_feat_dim);
        gnn_input.insert({"track_features", track_info});
        
        auto [out_f, out_vc, out_vf] = m_saltModel->runInference(gnn_input); // the dummy evaluation

        std::vector<std::string> output_names;
        for (auto& singlefloat : out_f){
          output_names.push_back(m_electronsKey.key()+"." + m_TaggerName + "_" + singlefloat.first);
        }
        ATH_CHECK(m_dec_el_plit_output.assign(output_names));
        ATH_CHECK(m_dec_el_plit_output.initialize());
    }
    else if (m_leptonsName == "Muons") {
        std::string fullPathToOnnxFile = PathResolverFindCalibFile(m_configPath.value() + m_configFileVersion.value());
        m_saltModel = std::make_shared<FlavorTagDiscriminants::SaltModel>(fullPathToOnnxFile);

        m_num_lepton_features = 6;
        m_num_track_features = 19;

        // set up decorators using a dummy query of the onnx model
        std::map<std::string, FlavorTagDiscriminants::Inputs> gnn_input;
        
        std::vector<float> muon_feat(m_num_lepton_features, 0.);
        std::vector<int64_t> muon_feat_dim = {1, static_cast<int64_t>(muon_feat.size())};
        FlavorTagDiscriminants::Inputs muon_info (muon_feat, muon_feat_dim);
        gnn_input.insert({"jet_features", muon_info}); // need to use the "jet_features" keyword as we are borrowing flavour tagging code
        
        std::vector<float> track_feat(m_num_track_features, 0.);
        std::vector<int64_t> track_feat_dim = {1, m_num_track_features};
        FlavorTagDiscriminants::Inputs track_info(track_feat, track_feat_dim);
        gnn_input.insert({"track_features", track_info});
        
        auto [out_f, out_vc, out_vf] = m_saltModel->runInference(gnn_input); // the dummy evaluation

        std::vector<std::string> output_names;
        for (auto& singlefloat : out_f){
          output_names.push_back(m_muonsKey.key()+"." + m_TaggerName + "_" + singlefloat.first);
        }
        ATH_CHECK(m_dec_mu_plit_output.assign(output_names));
        ATH_CHECK(m_dec_mu_plit_output.initialize());
    }
    else {
      ATH_MSG_ERROR("  ==> topology is not recognised! aborting.");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("DecoratePLIT " << name() << " initialization done." );

    return StatusCode::SUCCESS;
  }

  StatusCode DecoratePLIT::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_tracksKey, ctx);
    SG::ReadHandle<xAOD::CaloClusterContainer> caloclusters(m_caloclustersKey, ctx);

    // Make sure all the decorations are added as long as tracks exist.
    if (!tracks->empty()) {
      m_dec_trk_dr_lepton.getDecorationArray (*tracks);
      m_dec_trk_electron_track.getDecorationArray (*tracks);
      m_dec_trk_muon_track.getDecorationArray (*tracks);
      m_dec_trk_dr_leptontrack.getDecorationArray (*tracks);
    }

    if (!m_electronsKey.empty()) {
        // prepare decorators
        // ------------------
        std::vector<SG::WriteDecorHandle<xAOD::ElectronContainer, float>> dec_el_plit_output;
        for (const auto& wdhk: m_dec_el_plit_output) {
          dec_el_plit_output.emplace_back(wdhk, ctx);
        }
        SG::ReadHandle<xAOD::ElectronContainer> electrons(m_electronsKey, ctx);
        for (const xAOD::Electron* elec : *electrons) {
            if (!predictElec(*elec, *tracks, *caloclusters, dec_el_plit_output, ctx)) {
                ATH_MSG_ERROR("DecoratePLIT::execute - failed to predict electron");
                return StatusCode::FAILURE;
            }
        }
    } else if (!m_muonsKey.empty()) {
        // prepare decorators
        // ------------------
        std::vector<SG::WriteDecorHandle<xAOD::MuonContainer, float>> dec_mu_plit_output;
        for (const auto& wdhk: m_dec_mu_plit_output) {
          dec_mu_plit_output.emplace_back(wdhk, ctx);
        }
        SG::ReadHandle<xAOD::MuonContainer> muons(m_muonsKey, ctx);
        for (const xAOD::Muon* muon : *muons) {
            if (!predictMuon(*muon, *tracks, dec_mu_plit_output, ctx)) {
                ATH_MSG_ERROR("DecoratePLIT::execute - failed to predict muon");
                return StatusCode::FAILURE;
            }
        }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode DecoratePLIT::initializeAccessors() {

    ATH_CHECK(m_acc_mu_ptvarcone30TTVA.initialize(!m_muonsKey.empty()));
    ATH_CHECK(m_acc_mu_topoetcone30.initialize(!m_muonsKey.empty()));

    ATH_CHECK(m_acc_el_ptvarcone30.initialize(!m_electronsKey.empty()));
    ATH_CHECK(m_acc_el_topoetcone30.initialize(!m_electronsKey.empty()));

    ATH_CHECK(m_acc_trk_dr_lepton.initialize());
    ATH_CHECK(m_acc_trk_dr_leptontrack.initialize());
    ATH_CHECK(m_acc_trk_d0.initialize());
    ATH_CHECK(m_acc_trk_z0SinTheta.initialize());
    ATH_CHECK(m_acc_trk_d0Uncertainty.initialize());
    ATH_CHECK(m_acc_trk_z0SinThetaUncertainty.initialize());
    ATH_CHECK(m_acc_trk_muon_track.initialize());
    ATH_CHECK(m_acc_trk_electron_track.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode DecoratePLIT::predictMuon(
    const xAOD::Muon &muon,
    const xAOD::TrackParticleContainer &tracks,
    std::vector<SG::WriteDecorHandle<xAOD::MuonContainer, float>> &dec_mu_plit_output,
    const EventContext& ctx) const {
    // set up accessors
    // ---------------
    SG::ReadDecorHandle<xAOD::MuonContainer, float> acc_ptvarcone30TTVA{m_acc_mu_ptvarcone30TTVA, ctx};
    SG::ReadDecorHandle<xAOD::MuonContainer, float> acc_topoetcone30{m_acc_mu_topoetcone30, ctx};

    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_dr_lepton{m_acc_trk_dr_lepton, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_dr_leptontrack{m_acc_trk_dr_leptontrack, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_d0{m_acc_trk_d0, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_z0SinTheta{m_acc_trk_z0SinTheta, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_d0Uncertainty{m_acc_trk_d0Uncertainty, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_z0SinThetaUncertainty{m_acc_trk_z0SinThetaUncertainty, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, char> acc_muon_track{m_acc_trk_muon_track, ctx};

    // prepare input
    // -------------
    std::map<std::string, FlavorTagDiscriminants::Inputs> gnn_input;

    // collect muon features
    float muon_pt = muon.pt();
    float muon_eta = muon.eta();
    float muon_phi = muon.phi();

    float muon_ptvarcone30TTVARel = acc_ptvarcone30TTVA(muon) / muon_pt;
    float muon_topoetcone30Rel = acc_topoetcone30(muon) / muon_pt;

    float muon_caloClusterERel = -99;
    const xAOD::CaloCluster* cluster = muon.cluster();
    if (cluster) {
      float energyloss = 0;
      if (!muon.parameter(energyloss,xAOD::Muon::MeasEnergyLoss)) {
        ATH_MSG_WARNING("DecoratePLIT::execute - failed to retrieve energy loss");
        return StatusCode::FAILURE;
      }
      float calE = cluster->calE();
      if (std::abs(energyloss) > 0)
        muon_caloClusterERel = calE / energyloss;
    }

    // package muon features for inference
    std::vector<float> muon_feat = {
      muon_pt, 
      muon_eta, 
      muon_phi, 
      muon_ptvarcone30TTVARel, 
      muon_topoetcone30Rel,
      muon_caloClusterERel};
    std::vector<int64_t> muon_feat_dim = {1, static_cast<int64_t>(muon_feat.size())};

    // need to use the "jet_features" keyword as we are borrowing flavour tagging code
    FlavorTagDiscriminants::Inputs muon_info (muon_feat, muon_feat_dim);
    gnn_input.insert({"jet_features", muon_info});

    // decorate and fill track particles around the muon
    const xAOD::TrackParticle *muonTrack = muon.primaryTrackParticle();
    std::vector<const xAOD::IParticle *> parts;

    if(!fillParticles(parts, muon, muonTrack, tracks, ctx)) {
      ATH_MSG_ERROR("DecoratePLIT::execute - failed to fill particles");
      return StatusCode::FAILURE;
    }

    // extract track features from track particles
    std::vector<float> track_feat;
    track_feat.reserve(parts.size() * static_cast<int64_t>(muon_feat.size())); 
    
    // loop over parts and fill track_feat vector
    for (const xAOD::IParticle *part: parts) {
      const xAOD::TrackParticle *track = dynamic_cast<const xAOD::TrackParticle*>(part);

      // deta_lepton
      float deta_lepton = track->p4().Eta() - muon.eta();
      // dphi_lepton
      float dphi_lepton = track->p4().DeltaPhi(muon.p4());
      // qOverP
      float qoverp = track->qOverP();
      // btagIp_d0
      float d0 = acc_d0(*track);
      // btagIp_z0SinTheta
      float z0SinTheta = acc_z0SinTheta(*track);
      // btagIp_d0Uncertainty
      float d0Uncertainty = acc_d0Uncertainty(*track);
      // btagIp_z0SinThetaUncertainty
      float z0SinThetaUncertainty = acc_z0SinThetaUncertainty(*track);
      // btagIp_d0_significance
      float d0_significance = -99;  
      if (abs(d0Uncertainty) > 0) d0_significance = d0 / d0Uncertainty;
      // btagIp_z0SinTheta_significance
      float z0SinTheta_significance = -99;
      if (std::abs(z0SinThetaUncertainty) > 0) z0SinTheta_significance = z0SinTheta / z0SinThetaUncertainty;
      // numberOfPixelHits
      uint8_t pix_hits = 0;
      if(!track->summaryValue(pix_hits,xAOD::numberOfPixelHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfPixelHits");
        return StatusCode::FAILURE;
      }
      // numberOfInnermostPixelLayerHits
      uint8_t pix_innermosthits = 0;
      if(!track->summaryValue(pix_innermosthits,xAOD::numberOfInnermostPixelLayerHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfInnermostPixelLayerHits");
        return StatusCode::FAILURE;
      }
      // numberOfNextToInnermostPixelLayerHits
      uint8_t pix_nextinnermosthits = 0;
      if(!track->summaryValue(pix_nextinnermosthits,xAOD::numberOfNextToInnermostPixelLayerHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfNextToInnermostPixelLayerHits");
        return StatusCode::FAILURE;
      }
      // numberOfInnermostPixelLayerSharedHits
      uint8_t pix_innermostsharedhits = 0;
      if(!track->summaryValue(pix_innermostsharedhits,xAOD::numberOfInnermostPixelLayerSharedHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfInnermostPixelLayerSharedHits");
        return StatusCode::FAILURE;
      }
      // numberOfInnermostPixelLayerSplitHits
      uint8_t pix_innermostsplithits = 0;
      if(!track->summaryValue(pix_innermostsplithits,xAOD::numberOfInnermostPixelLayerSplitHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfInnermostPixelLayerSplitHits");
        return StatusCode::FAILURE;
      }
      // numberOfPixelSharedHits
      uint8_t pix_shared = 0;
      if(!track->summaryValue(pix_shared,xAOD::numberOfPixelSharedHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfPixelSharedHits");
        return StatusCode::FAILURE;
      }
      // numberOfPixelSplitHits
      uint8_t pix_split = 0;
      if(!track->summaryValue(pix_split,xAOD::numberOfPixelSplitHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfPixelSplitHits");
        return StatusCode::FAILURE;
      }
      // numberOfSCTHits
      uint8_t sct_hits = 0;
      if(!track->summaryValue(sct_hits,xAOD::numberOfSCTHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfSCTHits");
        return StatusCode::FAILURE;
      }
      // numberOfSCTSharedHits
      uint8_t sct_shared = 0;
      if(!track->summaryValue(sct_shared,xAOD::numberOfSCTSharedHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfSCTSharedHits");
        return StatusCode::FAILURE;
      }
      // muon_track
      char muon_track = acc_muon_track(*track);
      
      track_feat.push_back(deta_lepton);
      track_feat.push_back(dphi_lepton);
      track_feat.push_back(qoverp);
      track_feat.push_back(d0);
      track_feat.push_back(z0SinTheta);
      track_feat.push_back(d0Uncertainty);
      track_feat.push_back(z0SinThetaUncertainty);
      track_feat.push_back(d0_significance);
      track_feat.push_back(z0SinTheta_significance);
      track_feat.push_back(pix_hits);
      track_feat.push_back(pix_innermosthits);
      track_feat.push_back(pix_nextinnermosthits);
      track_feat.push_back(pix_innermostsharedhits);
      track_feat.push_back(pix_innermostsplithits);
      track_feat.push_back(pix_shared);
      track_feat.push_back(pix_split);
      track_feat.push_back(sct_hits);
      track_feat.push_back(sct_shared);
      track_feat.push_back(muon_track);
    }

    // prepare track features for inference
    int num_cnsts = parts.size();
    std::vector<int64_t> track_feat_dim = {num_cnsts, m_num_track_features};

    FlavorTagDiscriminants::Inputs track_info(track_feat, track_feat_dim);
    gnn_input.insert({"track_features", track_info});

    if (msgLvl(MSG::VERBOSE)) {
      ATH_MSG_VERBOSE("gnn_input size = " << gnn_input.size());
      for (auto& inp : gnn_input){
        ATH_MSG_VERBOSE(" " + inp.first + " dim = ");
        for (auto & dim: inp.second.second) {
          ATH_MSG_VERBOSE("  " + std::to_string(dim));
        }
        ATH_MSG_VERBOSE(" " + inp.first + " content = ");
        for (auto & con: inp.second.first) {
          ATH_MSG_VERBOSE("  " + std::to_string(con));
        }
      }
    }

    // run inference
    // -------------
    auto [out_f, out_vc, out_vf] = m_saltModel->runInference(gnn_input);
    if (msgLvl(MSG::VERBOSE)) {
      ATH_MSG_VERBOSE("runInference done.");
    
      ATH_MSG_VERBOSE("Output Float(s):");
      for (auto& singlefloat : out_f){
        ATH_MSG_VERBOSE(singlefloat.first + " = " << singlefloat.second);
      }
      ATH_MSG_VERBOSE("Output vector char(s):");
      for (auto& vecchar : out_vc){
        ATH_MSG_VERBOSE(vecchar.first + " = ");
        for (auto& cc : vecchar.second){
          ATH_MSG_VERBOSE(cc);
        }
      }
      ATH_MSG_VERBOSE("Output vector float(s):");
      for (auto& vecfloat : out_vf){
        ATH_MSG_VERBOSE(vecfloat.first + " = ");
        for (auto& ff : vecfloat.second){
          ATH_MSG_VERBOSE(ff);
        }
      }
    }
    // filling the tagger scores
    auto it_dec_mu_plit_output = dec_mu_plit_output.begin();
    for (auto& singlefloat : out_f){
      (*it_dec_mu_plit_output)(muon) = singlefloat.second;
      ++it_dec_mu_plit_output;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode DecoratePLIT::predictElec(
    const xAOD::Electron &electron,
    const xAOD::TrackParticleContainer &tracks,
    const xAOD::CaloClusterContainer &caloclusters,
    std::vector<SG::WriteDecorHandle<xAOD::ElectronContainer, float>> &dec_el_plit_output,
    const EventContext& ctx) const {
    // prepare input
    // -------------
    std::map<std::string, FlavorTagDiscriminants::Inputs> gnn_input;

    // accessors
    // ---------
    SG::ReadDecorHandle<xAOD::ElectronContainer, float> acc_ptvarcone30{m_acc_el_ptvarcone30, ctx};
    SG::ReadDecorHandle<xAOD::ElectronContainer, float> acc_topoetcone30{m_acc_el_topoetcone30, ctx};

    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_dr_lepton{m_acc_trk_dr_lepton, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_dr_leptontrack{m_acc_trk_dr_leptontrack, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_d0{m_acc_trk_d0, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_z0SinTheta{m_acc_trk_z0SinTheta, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_d0Uncertainty{m_acc_trk_d0Uncertainty, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_z0SinThetaUncertainty{m_acc_trk_z0SinThetaUncertainty, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, char> acc_electron_track{m_acc_trk_electron_track, ctx};

    // collect electron features
    float elec_pt = electron.pt();
    float elec_eta = electron.eta();
    float elec_ptvarcone30Rel = acc_ptvarcone30(electron) / elec_pt;
    float elec_topoetcone30Rel = acc_topoetcone30(electron) / elec_pt;

    // compute electron calorimeter cluster information
    float elec_caloClusterSumEtRel = 0.0;
    float sumCoreEt_large = 0.0;
    if (electron.caloCluster()) {
      float elec_calEta = electron.caloCluster()->eta(); 
      float elec_calPhi = electron.caloCluster()->phi();
  
      for (const xAOD::CaloCluster *cluster: caloclusters) {
        float deta = elec_calEta - cluster->eta();
        float dphi = TVector2::Phi_mpi_pi(elec_calPhi - cluster->phi());
        float dr   = std::sqrt(deta*deta + dphi*dphi);

        if (dr < m_lepCalErelConeSize) {
          sumCoreEt_large += cluster->pt();
        }
      } 
    }
    elec_caloClusterSumEtRel = sumCoreEt_large / elec_pt;  

    // collect best matched GSF electron track kinematics
    const xAOD::TrackParticle *electronTrack = nullptr;
    const xAOD::TrackParticle *bestmatchedGSFElTrack = electron.trackParticle(0);
    if (bestmatchedGSFElTrack) {
      electronTrack = xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(bestmatchedGSFElTrack);
    }
    float elec_pt_track = -99;
    float elec_eta_track = -99;
    float elec_phi_track = -99;
    if (electronTrack) {
      elec_pt_track = electronTrack->pt();
      elec_eta_track = electronTrack->eta();
      elec_phi_track = electronTrack->phi();
    }

    std::vector<float> electron_feat = {
      elec_pt_track, 
      elec_eta_track, 
      elec_phi_track, 
      elec_ptvarcone30Rel, 
      elec_topoetcone30Rel, 
      elec_caloClusterSumEtRel};
    std::vector<int64_t> electron_feat_dim = {1, static_cast<int64_t>(electron_feat.size())};

    // need to use the "jet_features" keyword as we are borrowing flavour tagging code
    FlavorTagDiscriminants::Inputs electron_info (electron_feat, electron_feat_dim);
    gnn_input.insert({"jet_features", electron_info});

    // decorate and fill track particles around the electron
    std::vector<const xAOD::IParticle *> parts;
    if (!fillParticles(parts, electron, electronTrack, tracks, ctx)) {
      ATH_MSG_ERROR("DecoratePLIT::execute - failed to fill particles");
      return StatusCode::FAILURE;
    }

    // collect track features from track particles
    std::vector<float> track_feat;
    track_feat.reserve(parts.size() * static_cast<int64_t>(electron_feat.size())); 

    for (const xAOD::IParticle *part: parts) {
      const xAOD::TrackParticle *track = dynamic_cast<const xAOD::TrackParticle*>(part);
      if (!track) {
        ATH_MSG_ERROR("DecoratePLIT::execute - null track pointer");
        continue;
      }
      
      // dr_lepton
      float dr_lepton = acc_dr_lepton(*track);
      // deta_lepton
      float deta_lepton = track->p4().Eta() - electron.eta();
      // dphi_lepton
      float dphi_lepton = track->p4().DeltaPhi(electron.p4());
      // qOverP
      float qoverp = track->qOverP();
      // btagIp_d0
      float d0 = acc_d0(*track);
      // btagIp_z0SinTheta
      float z0SinTheta = acc_z0SinTheta(*track);
      // btagIp_d0_significance
      float d0Uncertainty = acc_d0Uncertainty(*track);
      float d0_significance = -99;
      if (std::abs(d0Uncertainty) > 0) d0_significance = d0 / d0Uncertainty;
      // btagIp_z0SinTheta_significance
      float z0SinThetaUncertainty = acc_z0SinThetaUncertainty(*track);
      float z0SinTheta_significance = -99;
      if (std::abs(z0SinThetaUncertainty) > 0) z0SinTheta_significance = z0SinTheta / z0SinThetaUncertainty;
      // numberOfInnermostPixelLayerHits
      uint8_t pix_innermosthits = 0;
      if(!track->summaryValue(pix_innermosthits,xAOD::numberOfInnermostPixelLayerHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfInnermostPixelLayerHits");
        return StatusCode::FAILURE;
      }
      // numberOfNextToInnermostPixelLayerHits
      uint8_t pix_nextinnermosthits = 0;
      if(!track->summaryValue(pix_nextinnermosthits,xAOD::numberOfNextToInnermostPixelLayerHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfNextToInnermostPixelLayerHits");
        return StatusCode::FAILURE;
      }
      // numberOfInnermostPixelLayerSharedHits
      uint8_t pix_innermostsharedhits = 0;
      if(!track->summaryValue(pix_innermostsharedhits,xAOD::numberOfInnermostPixelLayerSharedHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfInnermostPixelLayerSharedHits");
        return StatusCode::FAILURE;
      }
      // numberOfInnermostPixelLayerSplitHits
      uint8_t pix_innermostsplithits = 0;
      if(!track->summaryValue(pix_innermostsplithits,xAOD::numberOfInnermostPixelLayerSplitHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfInnermostPixelLayerSplitHits");
        return StatusCode::FAILURE;
      }
      // numberOfPixelHits
      uint8_t pix_hits = 0;
      if(!track->summaryValue(pix_hits,xAOD::numberOfPixelHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfPixelHits");
        return StatusCode::FAILURE;
      }
      // numberOfPixelSharedHits
      uint8_t pix_shared = 0;
      if(!track->summaryValue(pix_shared,xAOD::numberOfPixelSharedHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfPixelSharedHits");
        return StatusCode::FAILURE;
      }
      // numberOfPixelSplitHits
      uint8_t pix_split = 0;
      if(!track->summaryValue(pix_split,xAOD::numberOfPixelSplitHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfPixelSplitHits");
        return StatusCode::FAILURE;
      }
      // numberOfSCTHits
      uint8_t sct_hits = 0;
      if(!track->summaryValue(sct_hits,xAOD::numberOfSCTHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfSCTHits");
        return StatusCode::FAILURE;
      }
      // numberOfSCTSharedHits
      uint8_t sct_shared = 0;
      if(!track->summaryValue(sct_shared,xAOD::numberOfSCTSharedHits)){
        ATH_MSG_ERROR("DecoratePLIT::execute - failed to retrieve xAOD::numberOfSCTSharedHits");
        return StatusCode::FAILURE;
      }
      // electron_track
      char electron_track = acc_electron_track(*track);


      track_feat.push_back(dr_lepton);
      track_feat.push_back(deta_lepton);
      track_feat.push_back(dphi_lepton);
      track_feat.push_back(qoverp);
      track_feat.push_back(d0);
      track_feat.push_back(z0SinTheta);
      track_feat.push_back(d0_significance);
      track_feat.push_back(z0SinTheta_significance);
      track_feat.push_back(pix_innermosthits);
      track_feat.push_back(pix_nextinnermosthits);    
      track_feat.push_back(pix_innermostsharedhits);
      track_feat.push_back(pix_innermostsplithits);
      track_feat.push_back(pix_hits);
      track_feat.push_back(pix_shared);
      track_feat.push_back(pix_split);
      track_feat.push_back(sct_hits);
      track_feat.push_back(sct_shared);
      track_feat.push_back(electron_track);
    }

    // prepare track features for inference
    int num_cnsts = parts.size();
    std::vector<int64_t> track_feat_dim = {num_cnsts, m_num_track_features};

    FlavorTagDiscriminants::Inputs track_info (track_feat, track_feat_dim);
    gnn_input.insert({"track_features", track_info});

    if (msgLvl(MSG::VERBOSE)) {
      ATH_MSG_VERBOSE("gnn_input size = " << gnn_input.size());
      for (auto& inp : gnn_input){
        ATH_MSG_VERBOSE(" " + inp.first + " dim = ");
        for (auto & dim: inp.second.second) {
          ATH_MSG_VERBOSE("  " + std::to_string(dim));
        }
        ATH_MSG_VERBOSE(" " + inp.first + " content = ");
        for (auto & con: inp.second.first) {
          ATH_MSG_VERBOSE("  " + std::to_string(con));
        }
      }
    }

    // run inference
    // -------------
    // use different model for endcap electrons
    auto [out_f, out_vc, out_vf] = (std::abs(elec_eta) < 1.37) ? m_saltModel->runInference(gnn_input) : m_saltModel_endcap->runInference(gnn_input);
    if (msgLvl(MSG::VERBOSE)) {
      ATH_MSG_VERBOSE("runInference done.");
      
      ATH_MSG_VERBOSE("Output Float(s):");
      for (auto& singlefloat : out_f){
        ATH_MSG_VERBOSE(singlefloat.first + " = " << singlefloat.second);
      }
      ATH_MSG_VERBOSE("Output vector char(s):");
      for (auto& vecchar : out_vc){
        ATH_MSG_VERBOSE(vecchar.first + " = ");
        for (auto& cc : vecchar.second){
          ATH_MSG_VERBOSE(cc);
        }
      }
      ATH_MSG_VERBOSE("Output vector float(s):");
      for (auto& vecfloat : out_vf){
        ATH_MSG_VERBOSE(vecfloat.first + " = ");
        for (auto& ff : vecfloat.second){
          ATH_MSG_VERBOSE(ff);
        }
      }
    }
    // filling the tagger scores
    auto it_dec_el_plit_output = dec_el_plit_output.begin();
    for (auto& singlefloat : out_f){
      (*it_dec_el_plit_output)(electron) = singlefloat.second;
      ++it_dec_el_plit_output;
    }

    return StatusCode::SUCCESS;
  }

  bool DecoratePLIT::passed_r22tracking_cuts(const xAOD::TrackParticle& tp, const EventContext& ctx) const
  {
    // r22 default track selection for flavour tagging GN2 algorithm
    constexpr float pt_minimum = 500; // MeV
    constexpr float abs_eta_maximum = 2.5;
    constexpr float d0_maximum = 3.5;
    constexpr float z0_maximum= 5.0;
    constexpr unsigned char si_hits_minimum = 8;
    constexpr unsigned char si_shared_maximum = 1;
    constexpr unsigned char si_holes_maximum = 2;
    constexpr unsigned char pix_holes_maximum = 1;

    // accessors
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_d0{m_acc_trk_d0, ctx};
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> acc_z0SinTheta{m_acc_trk_z0SinTheta, ctx};

    // get hit pixel info
    uint8_t pix_shared = 0;
    if(!tp.summaryValue(pix_shared,xAOD::numberOfPixelSharedHits)){
      ATH_MSG_ERROR("DecoratePLIT::passed_r22tracking_cuts - failed to retrieve xAOD::numberOfPixelSharedHits");
      return false;
    }
    uint8_t sct_shared = 0;
    if(!tp.summaryValue(sct_shared,xAOD::numberOfSCTSharedHits)){
      ATH_MSG_ERROR("DecoratePLIT::passed_r22tracking_cuts - failed to retrieve xAOD::numberOfSCTSharedHits");
      return false;
    }
    uint8_t pix_hits = 0;
    if(!tp.summaryValue(pix_hits,xAOD::numberOfPixelHits)){
      ATH_MSG_ERROR("DecoratePLIT::passed_r22tracking_cuts - failed to retrieve xAOD::numberOfPixelHits");
      return false;
    }
    uint8_t sct_hits = 0;
    if(!tp.summaryValue(sct_hits,xAOD::numberOfSCTHits)){
      ATH_MSG_ERROR("DecoratePLIT::passed_r22tracking_cuts - failed to retrieve xAOD::numberOfSCTHits");
      return false;
    }
    uint8_t pix_dead = 0;
    if(!tp.summaryValue(pix_dead,xAOD::numberOfPixelDeadSensors)){
      ATH_MSG_ERROR("DecoratePLIT::passed_r22tracking_cuts - failed to retrieve xAOD::numberOfPixelDeadSensors");
      return false;
    }
    uint8_t sct_dead = 0;
    if(!tp.summaryValue(sct_dead,xAOD::numberOfSCTDeadSensors)){
      ATH_MSG_ERROR("DecoratePLIT::passed_r22tracking_cuts - failed to retrieve xAOD::numberOfSCTDeadSensors");
      return false;
    }
    uint8_t pix_holes = 0;
    if(!tp.summaryValue(pix_holes,xAOD::numberOfPixelHoles)){
      ATH_MSG_ERROR("DecoratePLIT::passed_r22tracking_cuts - failed to retrieve xAOD::numberOfPixelHoles");
      return false;
    }
    uint8_t sct_holes = 0;
    if(!tp.summaryValue(sct_holes,xAOD::numberOfSCTHoles)){
      ATH_MSG_ERROR("DecoratePLIT::passed_r22tracking_cuts - failed to retrieve xAOD::numberOfSCTHoles");
      return false;
    }

    if (std::abs(tp.eta()) > abs_eta_maximum)
            return false;
    double n_module_shared = (pix_shared + sct_shared / 2);
    if (n_module_shared > si_shared_maximum)
      return false;
    if (tp.pt() <= pt_minimum)
      return false;
    if (std::isfinite(d0_maximum) &&
        std::abs(acc_d0(tp)) >= d0_maximum)
      return false;
    if (std::isfinite(z0_maximum) &&
        std::abs(acc_z0SinTheta(tp)) >= z0_maximum)
      return false;
    if (pix_hits + pix_dead + sct_hits + sct_dead < si_hits_minimum)
      return false;
    if ((pix_holes + sct_holes) > si_holes_maximum)
      return false;
    if (pix_holes > pix_holes_maximum)
      return false;
    return true;
  }

  StatusCode DecoratePLIT::fillParticles (
      std::vector<const xAOD::IParticle *> &parts,
      const xAOD::IParticle &lepton,
      const xAOD::TrackParticle *trackLep,
      const xAOD::TrackParticleContainer &trackContainer,
      const EventContext& ctx) const
  {
    // get lepton four momentum
    const FourMom_t lepton_p4 = lepton.p4();

    // Precompute tracks used for reconstruction
    std::set<const xAOD::TrackParticle*> tracksUsedForElectron;
    std::set<const xAOD::TrackParticle*> tracksUsedForMuon;
    if (const auto* elec = dynamic_cast<const xAOD::Electron*>(&lepton)) {
        tracksUsedForElectron = xAOD::EgammaHelpers::getTrackParticles(elec, true); // useBremAssoc = true
    } else if (const auto* muon = dynamic_cast<const xAOD::Muon*>(&lepton)) {
        if (muon->muonType() == xAOD::Muon::Combined && muon->inDetTrackParticleLink().isValid()) {
            tracksUsedForMuon.insert(muon->primaryTrackParticle()); // in the case of a muon, this is always 1 only
        }
    }

    // Loop over tracks and store them
    for (const xAOD::TrackParticle *track: trackContainer) {
      if (!track) {
        ATH_MSG_ERROR("DecoratePLIT::fillParticles - null track pointer");
        continue;
      }
      // check if track passed selection
      if (!passed_r22tracking_cuts(*track, ctx)) continue;

      // decorate track
      float dr_lepton = (lepton.p4().Pt() > 0.) ? track->p4().DeltaR(lepton.p4()) : -99;

      // do not even waste time to decorate something which is not used
      if (dr_lepton > m_maxLepTrackdR && m_maxLepTrackdR>=0) {continue;}

      bool isUsedForElectron = tracksUsedForElectron.count(track);
      bool isUsedForMuon = tracksUsedForMuon.count(track);

      if (!decorateTrack(*track, dr_lepton, isUsedForElectron, isUsedForMuon, trackLep)) {
        ATH_MSG_ERROR("DecoratePLIT::fillParticles - failed to decorate track");
        return StatusCode::FAILURE;
      }

      parts.push_back(track);
    }

    // Sort tracks by dR distance to lepton
    auto SORT_TRACKLEP = [&lepton_p4](const xAOD::IParticle* a, const xAOD::IParticle* b) {
      return a->p4().DeltaR(lepton_p4) < b->p4().DeltaR(lepton_p4);
    };
    std::sort(parts.begin(), parts.end(), SORT_TRACKLEP);

    return StatusCode::SUCCESS;
  }

  StatusCode DecoratePLIT::decorateTrack(
      const xAOD::TrackParticle& track,
      float dr_lepton,
      bool isUsedForElectron,
      bool isUsedForMuon,
      const xAOD::TrackParticle* trackLep) const
  {
      // Apply values to decorators
      m_dec_trk_dr_lepton(track) = dr_lepton;
      m_dec_trk_electron_track(track) = static_cast<char>(isUsedForElectron);
      m_dec_trk_muon_track(track) = static_cast<char>(isUsedForMuon);

      float dr_leptontrack = -99;
      if (trackLep) {
          if (trackLep->pt() > 0.) {
              dr_leptontrack = track.p4().DeltaR(trackLep->p4());
          }
      }
      m_dec_trk_dr_leptontrack(track) = dr_leptontrack;

      return StatusCode::SUCCESS;
  }
}
