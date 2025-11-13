/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PFTauFlowElementAssoc.h"
#include "xAODTau/TauTrack.h"
#include "xAODPFlow/FlowElement.h"
#include <map>
#include <utility>

using TauJetLink_t = ElementLink<xAOD::TauJetContainer>;
using FELink_t = ElementLink<xAOD::FlowElementContainer>;

PFTauFlowElementAssoc::PFTauFlowElementAssoc(const std::string& name,
              ISvcLocator* pSvcLocator): AthReentrantAlgorithm(name, pSvcLocator)
{}

PFTauFlowElementAssoc::~PFTauFlowElementAssoc() = default;

StatusCode PFTauFlowElementAssoc::initialize() {

  ATH_MSG_DEBUG("Initializing" << name() << "...");

  ATH_CHECK(m_tauNeutralFEWriteDecorKey.initialize());
  ATH_CHECK(m_tauChargedFEWriteDecorKey.initialize());
  ATH_CHECK(m_neutralFETauWriteDecorKey.initialize());
  ATH_CHECK(m_chargedFETauWriteDecorKey.initialize());

  ATH_CHECK(m_tauJetReadHandleKey.initialize());
  ATH_CHECK(m_neutralFEReadHandleKey.initialize());
  ATH_CHECK(m_chargedFEReadHandleKey.initialize());

  if (!m_jetReadHandleKey.empty()) ATH_CHECK(m_jetReadHandleKey.initialize());

  ATH_MSG_DEBUG("Initialization completed successfully");

  return StatusCode::SUCCESS;
}

/**
  This algorithm:
  1) Accesses the relevant FlowElement and TauJet containers
  2) Loops over the neutral FEs and matches them to the corresponding TauJet clusters, creating the ElementLinks
     and adding them to the JetETMissNeutralParticleFlowObjects container as a decoration
  3) Loops over the charged FEs and matches them to the corresponding TauJet tracks, creating the ElementLinks
     and adding them to the JetETMissChargedParticleFlowObjects container as a decoration
  4) Adds the decoration to the TauJet container containing a vector of ElementLinks to corresponding FEs
**/

StatusCode PFTauFlowElementAssoc::execute(const EventContext &ctx) const {

  // Write decoration handles for linking the TauJet container with the FlowElement container and vice versa
  SG::WriteDecorHandle< xAOD::TauJetContainer, std::vector<FELink_t> > tauNeutralFEWriteDecorHandle (m_tauNeutralFEWriteDecorKey,ctx);
  SG::WriteDecorHandle< xAOD::FlowElementContainer, std::vector<TauJetLink_t> > neutralFETauWriteDecorHandle (m_neutralFETauWriteDecorKey,ctx);
  SG::WriteDecorHandle< xAOD::TauJetContainer, std::vector<FELink_t> > tauChargedFEWriteDecorHandle (m_tauChargedFEWriteDecorKey,ctx);
  SG::WriteDecorHandle< xAOD::FlowElementContainer, std::vector<TauJetLink_t> > chargedFETauWriteDecorHandle (m_chargedFETauWriteDecorKey,ctx);
  // Read handles for the TauJet container and the FlowElement container
  SG::ReadHandle<xAOD::TauJetContainer> tauJetReadHandle (m_tauJetReadHandleKey,ctx);
  SG::ReadHandle<xAOD::FlowElementContainer> neutralFEReadHandle (m_neutralFEReadHandleKey,ctx);
  SG::ReadHandle<xAOD::FlowElementContainer> chargedFEReadHandle (m_chargedFEReadHandleKey,ctx);

  // Initialize flow element link containers
  std::vector<std::vector<FELink_t>> tauNeutralFEVec(tauJetReadHandle->size());
  std::vector<std::vector<FELink_t>> tauChargedFEVec(tauJetReadHandle->size());

  // Prepare <tau,clusters> and <tau,tracks> maps to avoid nested loops
  std::multimap<size_t, size_t> map_tau_clusters, map_tau_tracks;
  // Skip taus that won't be written to AOD - first check the variable exists
  // because older ESD and any AOD used as input do not have this variable.
  static const SG::ConstAccessor<char> acc_passThinning("passThinning");
  for (const xAOD::TauJet* tau : *tauJetReadHandle) {
    if(acc_passThinning.isAvailable(*tau) && !acc_passThinning(*tau)) continue;

    // Vertexed clusters are transiently available in RAWtoALL, retrieve them if available
    static const SG::Accessor< std::vector< xAOD::CaloVertexedTopoCluster > > vertexedClustersAcc( "VertexedClusters" );
    if (vertexedClustersAcc.isAvailable(*tau)) {
      std::vector<xAOD::CaloVertexedTopoCluster> vertexedClusters = tau->vertexedClusters();
      for (const auto& cluster : vertexedClusters) {
	// Check if the cluster is within R = 0.2 of tau axis
	if (cluster.p4().DeltaR(tau->p4(xAOD::TauJetParameters::IntermediateAxis)) > 0.2) continue;
	// Get the index of the cluster associated to the tau
	map_tau_clusters.insert(std::make_pair<size_t,size_t>(tau->index(),cluster.clust().index()));
      }
    }
    else {
      // Recompute vertexed clusters
      // Get tau vertex
      const xAOD::Vertex* tauVertex = tau->vertex();
      // Get the clusters associated to the tau
      std::vector<const xAOD::IParticle*> tauClusters = tau->clusters();
      for (const auto *cluster : tauClusters) {
        const xAOD::CaloCluster* clus = static_cast<const xAOD::CaloCluster*>(cluster);
        TLorentzVector clusterp4 = clus->p4();
        // Correct cluster to tau vertex if it exists
        if (tauVertex != nullptr) {
          xAOD::CaloVertexedTopoCluster vertexedClus(*clus, tauVertex->position());
          clusterp4 = vertexedClus.p4();
        }
        // Check if the cluster is within R = 0.2 of tau axis
        if (clusterp4.DeltaR(tau->p4(xAOD::TauJetParameters::IntermediateAxis)) > 0.2) continue;
	// Get the index of the cluster associated to the tau
	map_tau_clusters.insert(std::make_pair<size_t,size_t>(tau->index(),clus->index()));
      }
    }

    // Get tau tracks associated to the tau
    std::vector<const xAOD::TauTrack*> tauTracks = tau->tracks();
    for (const auto *tauTrack : tauTracks) {
      // Get track associated to the tau track to use for matching
      const xAOD::TrackParticle* tauIDTrack = tauTrack->track();
      // Get the index of the track associated to the tau
      map_tau_tracks.insert(std::make_pair<size_t,size_t>(tau->index(),tauIDTrack->index()));
    }
  }

  ////////////////////////////////////////////
  // Loop over the neutral flow elements
  ////////////////////////////////////////////
  for (const xAOD::FlowElement* FE : *neutralFETauWriteDecorHandle) {
    // Check that the flow element cluster exists and is not null
    if (FE->otherObjects().empty()) continue;
    if (FE->otherObjects().at(0) == nullptr) continue;
    // Get the index of the flow element cluster
    size_t FEClusterIndex = FE->otherObjects().at(0)->index();

    std::vector<TauJetLink_t> FETauJetLinks;

    // Loop over the tau/clusters map
    for (const auto [tauIndex, tauClusterIndex] : map_tau_clusters) {
      // Link the tau and the neutral FE if the cluster indices match
      if (tauClusterIndex == FEClusterIndex) {
	FETauJetLinks.emplace_back(*tauJetReadHandle, tauIndex);
	tauNeutralFEVec.at(tauIndex).emplace_back(*neutralFEReadHandle, FE->index() );
      }
    }

    // Add vector of elements links to the tau jets as a decoration to the FE container
    neutralFETauWriteDecorHandle (*FE) = FETauJetLinks;

  } // end neutral FE loop

  ////////////////////////////////////////////
  // Loop over the charged flow elements
  ////////////////////////////////////////////
  for (const xAOD::FlowElement* FE : *chargedFETauWriteDecorHandle) {
    // Check that the flow element track exists and is not null
    if (FE->chargedObjects().empty()) continue;
    if (FE->chargedObjects().at(0) == nullptr) continue;

    // Get the index of the flow element track
    size_t FETrackIndex = FE->chargedObjects().at(0)->index();

    std::vector<TauJetLink_t> FETauJetLinks;

    // Loop over the tau/tracks map
    for (const auto [tauIndex, tauIDTrackIndex] : map_tau_tracks) {
      // Link the tau and the charged FE if the track indices match
      if (tauIDTrackIndex == FETrackIndex) {
	FETauJetLinks.emplace_back(*tauJetReadHandle, tauIndex);
	tauChargedFEVec.at(tauIndex).emplace_back(*chargedFEReadHandle, FE->index() );
      }
    }

    // Add vector of elements links to the tau jets as a decoration to the FE container
    chargedFETauWriteDecorHandle (*FE) = FETauJetLinks;

  } // end charged FE loop

  ////////////////////////////////////////////
  // Write decorations to TauJet container
  ////////////////////////////////////////////
  // Add vectors of Flow Element (FE) Links as decorations to the TauJet container
  for (const xAOD::TauJet* tau : *tauNeutralFEWriteDecorHandle) {
    tauNeutralFEWriteDecorHandle (*tau) = tauNeutralFEVec.at(tau->index());
    tauChargedFEWriteDecorHandle (*tau) = tauChargedFEVec.at(tau->index());
  }

  ATH_MSG_DEBUG("Execute completed successfully");

  return StatusCode::SUCCESS;
}
