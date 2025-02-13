/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauGNNEvaluator.h"
#include "tauRecTools/HelperFunctions.h"

#include "PathResolver/PathResolver.h"

#include <algorithm>


TauGNNEvaluator::TauGNNEvaluator(const std::string &name): 
  TauRecToolBase(name),
  m_net_inclusive(nullptr),
  m_net_0p(nullptr), m_net_1p(nullptr), m_net_2p(nullptr), m_net_3p(nullptr) {
    
  declareProperty("NetworkFileInclusive", m_weightfile_inclusive = "");
  declareProperty("NetworkFile0P", m_weightfile_0p = "");
  declareProperty("NetworkFile1P", m_weightfile_1p = "");
  declareProperty("NetworkFile2P", m_weightfile_2p = "");
  declareProperty("NetworkFile3P", m_weightfile_3p = "");

  declareProperty("OutputVarname", m_output_varname = "GNTauScore");
  declareProperty("OutputPTau", m_output_ptau = "GNTauProbTau");
  declareProperty("OutputPJet", m_output_pjet = "GNTauProbJet");
  declareProperty("OutputDiscriminant", m_output_discriminant = Discriminant::NegLogPJet, 
    "Discriminant used to calculate the output score: 0 -> -log(PJet), 1 -> PTau");

  declareProperty("MaxTracks", m_max_tracks = 30);
  declareProperty("MaxClusters", m_max_clusters = 20);
  declareProperty("MaxClusterDR", m_max_cluster_dr = 1.0f);

  declareProperty("VertexCorrection", m_doVertexCorrection = true);
  declareProperty("DecorateTracks", m_decorateTracks = false);
  declareProperty("TrackClassification", m_doTrackClassification = true);
  declareProperty("MinTauPt", m_minTauPt = 0.);

  // Prongness selection minimum track pT
  declareProperty("MinProngTrackPt", m_min_prong_track_pt = 0);

  // Naming conventions for the network weight files:
  declareProperty("InputLayerScalar", m_input_layer_scalar = "tau_vars");
  declareProperty("InputLayerTracks", m_input_layer_tracks = "track_vars");
  declareProperty("InputLayerClusters", m_input_layer_clusters = "cluster_vars");
  declareProperty("NodeNameTau", m_outnode_tau = "GN2TauNoAux_pb");
  declareProperty("NodeNameJet", m_outnode_jet = "GN2TauNoAux_pu");
  }

TauGNNEvaluator::~TauGNNEvaluator() {}

StatusCode TauGNNEvaluator::initialize() {
  ATH_MSG_INFO("Initializing TauGNNEvaluator with "<<m_max_tracks<<" tracks and "<<m_max_clusters<<" clusters...");

  // Set the layer and node names in the weight file
  TauGNN::Config config;
  config.input_layer_scalar = m_input_layer_scalar;
  config.input_layer_tracks = m_input_layer_tracks;
  config.input_layer_clusters = m_input_layer_clusters;
  config.output_node_tau = m_outnode_tau;
  config.output_node_jet = m_outnode_jet;

  // We can either use an inclussive GNN (e.g. Offline GNTauv0), or a prong-dependent GNN (e.g. HLT GNTau), not both!
  
  if(!m_weightfile_inclusive.empty()) { // Prong-inclusive network
    if(!m_weightfile_0p.empty() || !m_weightfile_1p.empty() || !m_weightfile_2p.empty() || !m_weightfile_3p.empty()) {
      ATH_MSG_ERROR("Cannot load both prong-inclusive and prong-dependent networks!");
      return StatusCode::FAILURE;
    }
    
    ATH_MSG_INFO("Loading prong-inclusive TauID GNN");
    m_net_inclusive = load_network(m_weightfile_inclusive, config);
    if(!m_net_inclusive) return StatusCode::FAILURE;

  } else { // Prong-dependent networks

    // 0-prong is optional
    if(!m_weightfile_0p.empty()) {
      ATH_MSG_INFO("Loading 0-prong TauID GNN");
      m_net_0p = load_network(m_weightfile_0p, config);
      if(!m_net_0p) return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Loading 1-prong TauID GNN");
    m_net_1p = load_network(m_weightfile_1p, config);
    if(!m_net_1p) return StatusCode::FAILURE;

    // 2-prong is optional
    if(!m_weightfile_2p.empty()) {
      ATH_MSG_INFO("Loading 2-prong TauID GNN");
      m_net_2p = load_network(m_weightfile_2p, config);
      if(!m_net_2p) return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Loading 3-prong TauID GNN");
    m_net_3p = load_network(m_weightfile_3p, config);
    if(!m_net_3p) return StatusCode::FAILURE;
  }

  if(m_output_discriminant < Discriminant::NegLogPJet || m_output_discriminant > Discriminant::PTau) {
    ATH_MSG_FATAL("Invalid TauGNNEvaluator discriminant setting: " << m_output_discriminant);
  }
  
  return StatusCode::SUCCESS;
}

std::unique_ptr<TauGNN> TauGNNEvaluator::load_network(const std::string& network_file, const TauGNN::Config& config) const {
  // Use PathResolver to search for the weight files
  if(network_file.empty()) return nullptr;

  const std::string pr_network_file = find_file(network_file);
  if(pr_network_file.empty()) {
    ATH_MSG_ERROR("Could not find network weights: " << network_file);
    return nullptr;
  }
  
  ATH_MSG_INFO("Using network config: " << pr_network_file);
  
  // Load the weights and create the network
  std::unique_ptr<TauGNN> net = std::make_unique<TauGNN>(pr_network_file, config);
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
  output(tau) = -1111.0f;
  out_ptau(tau) = -1111.0f;
  out_pjet(tau) = -1111.0f;

  //Skip execution for low-pT taus to save resources
  if (tau.pt() < m_minTauPt) {
    return StatusCode::SUCCESS;
  }

  // Get input objects
  ATH_MSG_DEBUG("Fetching Tracks");
  std::vector<const xAOD::TauTrack *> tracks;
  ATH_CHECK(get_tracks(tau, tracks));
  ATH_MSG_DEBUG("Fetching clusters");
  std::vector<xAOD::CaloVertexedTopoCluster> clusters;
  ATH_CHECK(get_clusters(tau, clusters));
  ATH_MSG_DEBUG("Constituent fetching done...");

  // Truncate tracks
  int numTracksMax = std::min(m_max_tracks, static_cast<int>(tracks.size()));
  std::vector<const xAOD::TauTrack *> trackVec(tracks.begin(), tracks.begin()+numTracksMax);

  // Network outputs
  std::map<std::string, float> out_f;
  std::map<std::string, std::vector<char>> out_vc;
  std::map<std::string, std::vector<float>> out_vf;

  // Evaluate networks
  if(m_net_inclusive) {
    std::tie(out_f, out_vc, out_vf) = m_net_inclusive->compute(tau, trackVec, clusters);
  } else {
    // First we calculate the tau prongness
    int n_tracks = tau.nTracksCharged();
    if(m_min_prong_track_pt) {
      n_tracks = 0;
      for(const xAOD::TauTrack* track : tracks) {
        if(track->pt() > m_min_prong_track_pt) n_tracks++;
      }
    }
    ATH_MSG_DEBUG("Tau prongness: " << n_tracks);

    if(n_tracks == 0 && m_net_0p) std::tie(out_f, out_vc, out_vf) = m_net_0p->compute(tau, trackVec, clusters);
    else if(n_tracks == 1) std::tie(out_f, out_vc, out_vf) = m_net_1p->compute(tau, trackVec, clusters);
    else if(n_tracks == 2) {
      if(m_net_2p) std::tie(out_f, out_vc, out_vf) = m_net_2p->compute(tau, trackVec, clusters);
      else std::tie(out_f, out_vc, out_vf) = m_net_3p->compute(tau, trackVec, clusters);
    } else if(n_tracks == 3) std::tie(out_f, out_vc, out_vf) = m_net_3p->compute(tau, trackVec, clusters);
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

    if(m_decorateTracks) {
      for(size_t i = 0; i < tracks.size(); i++) {
        if(i < out_vc.at("track_class").size()) out_trkclass(*tracks.at(i)) = out_vc.at("track_class").at(i);
        else out_trkclass(*tracks.at(i)) = '9'; //Dummy value for tracks outside range of out_vc
      }
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode TauGNNEvaluator::get_tracks(const xAOD::TauJet &tau, std::vector<const xAOD::TauTrack *> &out) const {
  std::vector<const xAOD::TauTrack*> tracks = tau.allTracks();

  // Skip unclassified tracks:
  // - the track is a LRT and classifyLRT = false
  // - the track is not among the MaxNtracks highest-pt tracks in the track classifier
  // - track classification is not run (trigger)
  if(m_doTrackClassification) {
    std::vector<const xAOD::TauTrack*>::iterator it = tracks.begin();
    while(it != tracks.end()) {
      if((*it)->flag(xAOD::TauJetParameters::unclassified)) {
	it = tracks.erase(it);
      }
      else {
	++it;
      }
    }
  }

  // Sort by descending pt
  auto cmp_pt = [](const xAOD::TauTrack *lhs, const xAOD::TauTrack *rhs) {
    return lhs->pt() > rhs->pt();
  };
  std::sort(tracks.begin(), tracks.end(), cmp_pt);
  out = std::move(tracks);

  return StatusCode::SUCCESS;
}

StatusCode TauGNNEvaluator::get_clusters(const xAOD::TauJet &tau, std::vector<xAOD::CaloVertexedTopoCluster> &clusters) const {

  TLorentzVector tauAxis = tauRecTools::getTauAxis(tau, m_doVertexCorrection);

  for (const xAOD::CaloVertexedTopoCluster& vertexedCluster : tau.vertexedClusters()) {
    TLorentzVector clusterP4 = vertexedCluster.p4();
    if (clusterP4.DeltaR(tauAxis) > m_max_cluster_dr) continue;
      
    clusters.push_back(vertexedCluster);
  }

  // Sort by descending et
  auto et_cmp = [](const xAOD::CaloVertexedTopoCluster& lhs,
		   const xAOD::CaloVertexedTopoCluster& rhs) {
    return lhs.p4().Et() > rhs.p4().Et();
  };
  std::sort(clusters.begin(), clusters.end(), et_cmp);

  // Truncate clusters
  if (static_cast<int>(clusters.size()) > m_max_clusters) {
    clusters.resize(m_max_clusters, clusters[0]);
  }

  return StatusCode::SUCCESS;
}
