/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauGNNEvaluator.h"
#include "tauRecTools/TauGNN.h"
#include "tauRecTools/HelperFunctions.h"

#include "PathResolver/PathResolver.h"

#include <algorithm>


TauGNNEvaluator::TauGNNEvaluator(const std::string &name): 
  TauRecToolBase(name),
  m_net(nullptr){
    
  declareProperty("NetworkFile", m_weightfile = "");
  declareProperty("OutputVarname", m_output_varname = "GNTauScore");
  declareProperty("OutputPTau", m_output_ptau = "GNTauProbTau");
  declareProperty("OutputPJet", m_output_pjet = "GNTauProbJet");
  declareProperty("MaxTracks", m_max_tracks = 30);
  declareProperty("MaxClusters", m_max_clusters = 20);
  declareProperty("MaxClusterDR", m_max_cluster_dr = 1.0f);
  declareProperty("VertexCorrection", m_doVertexCorrection = true);
  declareProperty("DecorateTracks", m_decorateTracks = false);
  declareProperty("TrackClassification", m_doTrackClassification = true);
  declareProperty("MinTauPt", m_minTauPt = 0.);

  // Naming conventions for the network weight files:
  declareProperty("InputLayerScalar", m_input_layer_scalar = "tau_vars");
  declareProperty("InputLayerTracks", m_input_layer_tracks = "track_vars");
  declareProperty("InputLayerClusters", m_input_layer_clusters = "cluster_vars");
  declareProperty("NodeNameTau", m_outnode_tau = "GN2TauNoAux_pb");
  declareProperty("NodeNameJet", m_outnode_jet = "GN2TauNoAux_pu");
  }

TauGNNEvaluator::~TauGNNEvaluator() {}

StatusCode TauGNNEvaluator::initialize() {
  ATH_MSG_INFO("Initializing TauGNNEvaluator");
  
  std::string weightfile("");

  // Use PathResolver to search for the weight files
  if (!m_weightfile.empty()) {
    weightfile = find_file(m_weightfile);
    if (weightfile.empty()) {
      ATH_MSG_ERROR("Could not find network weights: " << m_weightfile);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO("Using network config: " << weightfile);
    }
  }

  // Set the layer and node names in the weight file
  TauGNN::Config config;
  config.input_layer_scalar = m_input_layer_scalar;
  config.input_layer_tracks = m_input_layer_tracks;
  config.input_layer_clusters = m_input_layer_clusters;
  config.output_node_tau = m_outnode_tau;
  config.output_node_jet = m_outnode_jet;

  // Load the weights and create the network
  if (!weightfile.empty()) {
    m_net = std::make_unique<TauGNN>(weightfile, config);
    if (!m_net) {
      ATH_MSG_ERROR("No network configured.");
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode TauGNNEvaluator::execute(xAOD::TauJet &tau) const {
  // Output variable Decorators
  const SG::AuxElement::Accessor<float> output(m_output_varname);
  const SG::AuxElement::Accessor<float> out_ptau(m_output_ptau);
  const SG::AuxElement::Accessor<float> out_pjet(m_output_pjet);
  const SG::AuxElement::Decorator<char> out_trkclass("GNTau_TrackClass");
  // Set default score and overwrite later
  output(tau) = -1111.0f;
  out_ptau(tau) = -1111.0f;
  out_pjet(tau) = -1111.0f;

  //Skip execution for low-pT taus to save resources
  if (tau.pt() < m_minTauPt) {
    return StatusCode::SUCCESS;
  }

  // Get input objects
  std::vector<const xAOD::TauTrack *> tracks;
  ATH_CHECK(get_tracks(tau, tracks));
  std::vector<xAOD::CaloVertexedTopoCluster> clusters;
  ATH_CHECK(get_clusters(tau, clusters));

  // Truncate tracks
  int numTracksMax = std::min(m_max_tracks, tracks.size());
  std::vector<const xAOD::TauTrack *> trackVec(tracks.begin(), tracks.begin()+numTracksMax);
  // Evaluate networks
  if (m_net) {
    auto [out_f, out_vc, out_vf] = m_net->compute(tau, trackVec, clusters);
    output(tau)=std::log10(1/(1-out_f.at(m_outnode_tau)));
    out_ptau(tau)=out_f.at(m_outnode_tau);
    out_pjet(tau)=out_f.at(m_outnode_jet);
    if (m_decorateTracks){
      for(unsigned int i=0;i<tracks.size();i++){
        if(i<out_vc.at("track_class").size()){out_trkclass(*tracks.at(i))=out_vc.at("track_class").at(i);}
        else{out_trkclass(*tracks.at(i))='9';} //Dummy value for tracks outside range of out_vc
      }
    }
  }
  
  return StatusCode::SUCCESS;
}

const TauGNN* TauGNNEvaluator::get_gnn() const {
  return m_net.get();
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
  if (clusters.size() > m_max_clusters) {
    clusters.resize(m_max_clusters, clusters[0]);
  }

  return StatusCode::SUCCESS;
}
