/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauGNNEvaluator.h"
#include "tauRecTools/HelperFunctions.h"

#include "PathResolver/PathResolver.h"

#include <algorithm>


TauGNNEvaluator::TauGNNEvaluator(const std::string &name): 
  TauRecToolBase(name) {}

TauGNNEvaluator::~TauGNNEvaluator() {}

StatusCode TauGNNEvaluator::initialize() {
  ATH_MSG_INFO("Initializing TauGNNEvaluator with "<<m_max_tracks.value()<<" tracks, "<<m_max_clusters<<" clusters, and "<<m_max_hits<<" hits...");

  // We can either use an inclussive GNN (e.g. Offline GNTauv0), or a prong-dependent GNN (e.g. HLT GNTau), not both!
  
  if(!m_weightfile_inclusive.empty()) { // Prong-inclusive network
    if(!m_weightfile_0p.empty() || !m_weightfile_1p.empty() || !m_weightfile_2p.empty() || !m_weightfile_3p.empty()) {
      ATH_MSG_ERROR("Cannot load both prong-inclusive and prong-dependent networks!");
      return StatusCode::FAILURE;
    }
    
    ATH_MSG_INFO("Loading prong-inclusive TauID GNN");
    m_net_inclusive = load_network(m_weightfile_inclusive);
    if(!m_net_inclusive) return StatusCode::FAILURE;

  } else { // Prong-dependent networks

    // 0-prong is optional
    if(!m_weightfile_0p.empty()) {
      ATH_MSG_INFO("Loading 0-prong TauID GNN");
      m_net_0p = load_network(m_weightfile_0p);
      if(!m_net_0p) return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Loading 1-prong TauID GNN");
    m_net_1p = load_network(m_weightfile_1p);
    if(!m_net_1p) return StatusCode::FAILURE;

    // 2-prong is optional
    if(!m_weightfile_2p.empty()) {
      ATH_MSG_INFO("Loading 2-prong TauID GNN");
      m_net_2p = load_network(m_weightfile_2p);
      if(!m_net_2p) return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Loading 3-prong TauID GNN");
    m_net_3p = load_network(m_weightfile_3p);
    if(!m_net_3p) return StatusCode::FAILURE;
  }

  if(m_output_discriminant < Discriminant::Disabled || m_output_discriminant > Discriminant::PTau) {
    ATH_MSG_FATAL("Invalid TauGNNEvaluator discriminant setting: " << m_output_discriminant);
  }

  if(!m_tauContainerName.empty()) {
    // We should move to using WriteDecorHandles in the future, but for now
    // we create keys to enforce data-dependencies in the scheduler

    if(m_output_discriminant != Discriminant::Disabled) {
      m_scoreHandleKey = m_tauContainerName + "." + m_output_varname;
      ATH_CHECK(m_scoreHandleKey.initialize());
    }

    m_pTauHandleKey = m_tauContainerName + "." + m_output_ptau;
    ATH_CHECK(m_pTauHandleKey.initialize());

    m_pJetHandleKey = m_tauContainerName + "." + m_output_pjet;
    ATH_CHECK(m_pJetHandleKey.initialize());
  }

  if(!m_tauContainerName.empty() && !m_hitsHandleKey.empty()) {
    m_hits_decor_name = m_hitsHandleKey.key();
    m_hitsHandleKey = m_tauContainerName + "." + m_hitsHandleKey.key();
    ATH_CHECK(m_hitsHandleKey.initialize());
  } else if (m_max_hits > 0) {
    ATH_MSG_ERROR("TauContainerName and HitsHandleKey must be provided to read hits for the GNN evaluation");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

std::unique_ptr<TauGNN> TauGNNEvaluator::load_network(const std::string& network_file) const {
  // Use PathResolver to search for the weight files
  if(network_file.empty()) return nullptr;

  const std::string pr_network_file = find_file(network_file);
  if(pr_network_file.empty()) {
    ATH_MSG_ERROR("Could not find network weights: " << network_file);
    return nullptr;
  }
  
  ATH_MSG_INFO("Using network config: " << pr_network_file);
  
  // Load the weights and create the network
  TauGNNDataLoader::Config config;
  config.nnFile               = pr_network_file;
  config.input_layer_scalar   = m_input_layer_scalar.value();
  config.input_layer_tracks   = m_input_layer_tracks.value();
  config.input_layer_clusters = m_input_layer_clusters.value();
  config.input_layer_hits     = m_input_layer_hits.value();
  config.output_node_tau      = m_outnode_tau.value();
  config.output_node_jet      = m_outnode_jet.value();
  config.n_max_tracks         = m_max_tracks.value();
  config.n_max_clusters       = m_max_clusters.value();
  config.max_dr_cluster       = m_max_cluster_dr.value();
  config.n_max_hits           = m_max_hits.value();
  config.doVertexCorrection   = m_doVertexCorrection.value();
  config.trackClassification  = m_doTrackClassification.value();
  config.useTRT               = m_useTRT.value();
  config.hits_decor_name      = m_hits_decor_name;

  std::unique_ptr<TauGNN> net = std::make_unique<TauGNN>(config);
  if(!net) ATH_MSG_ERROR("No network configured.");

  return net;
}

StatusCode TauGNNEvaluator::execute(xAOD::TauJet &tau) const {
  // Output variable Decorators
  const SG::Accessor<float> output(m_output_varname);
  const SG::Accessor<float> out_ptau(m_output_ptau);
  const SG::Accessor<float> out_pjet(m_output_pjet);
  const SG::Decorator<char> out_trkclass("GNTau_TrackClass");
  // Set default score and overwrite later
  if(m_output_discriminant != Discriminant::Disabled) output(tau) = -1111.0f;
  out_ptau(tau) = -1111.0f;
  out_pjet(tau) = -1111.0f;

  //Skip execution for low-pT taus to save resources
  if (tau.pt() < m_minTauPt) {
    return StatusCode::SUCCESS;
  }

  // save CPU when running PHYS derivations
  if (m_applyLooseTrackSel) {
    if (tau.nTracks()>5) return StatusCode::SUCCESS;
  }

  // save CPU when running in RAWtoALL for tau trigger monitoring purpose
  if (m_applyTightTrackSel) {
    if (tau.nTracks()!=1 && tau.nTracks()!=3) return StatusCode::SUCCESS;
  }

  // Network outputs
  std::map<std::string, float> out_f;
  std::map<std::string, std::vector<char>> out_vc;
  std::map<std::string, std::vector<float>> out_vf;

  // Evaluate networks
  ATH_MSG_DEBUG("Evaluating GNN for tau with nTracks = " << tau.nTracksCharged());
  if(m_net_inclusive) {
    std::tie(out_f, out_vc, out_vf) = m_net_inclusive->compute(tau);
  } else {
    // First we calculate the tau prongness
    int n_tracks = tau.nTracksCharged();
    // in trigger, we need to apply a min pT cut on the tracks to count the prongs, 
    // as no track classification is available
    if (not m_doTrackClassification && m_min_prong_track_pt>0.) {
      auto trks = tau.allTracks();
      const float threshold = m_min_prong_track_pt;
      n_tracks = std::count_if(trks.begin(), trks.end(), 
        [&threshold](const xAOD::TauTrack* trk) { return trk->pt() > threshold; }
      );
    }

    if(n_tracks == 0 && m_net_0p) std::tie(out_f, out_vc, out_vf) = m_net_0p->compute(tau);
    else if(n_tracks == 1) std::tie(out_f, out_vc, out_vf) = m_net_1p->compute(tau);
    else if(n_tracks == 2) {
      if(m_net_2p) std::tie(out_f, out_vc, out_vf) = m_net_2p->compute(tau);
      else std::tie(out_f, out_vc, out_vf) = m_net_3p->compute(tau);
    } else if(n_tracks == 3) std::tie(out_f, out_vc, out_vf) = m_net_3p->compute(tau);
  }

  // Store scores only if the inferences actually ran
  if(out_f.contains(m_outnode_tau)) {
    if(m_output_discriminant == Discriminant::NegLogPJet) {
        output(tau) = std::log10(1/(1-out_f.at(m_outnode_tau)));
    } else if(m_output_discriminant == Discriminant::PTau) {
        output(tau) = out_f.at(m_outnode_tau);
    }

    out_ptau(tau) = out_f.at(m_outnode_tau);
    out_pjet(tau) = out_f.at(m_outnode_jet);
  }

  return StatusCode::SUCCESS;
}

