/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTau/TauxAODHelpers.h"

typedef ElementLink<xAOD::TruthParticleContainer> TruthLink_t;

// ==================================================================
const xAOD::TruthParticle* xAOD::TauHelpers::getTruthParticle(const xAOD::IParticle* particle, bool debug)
{
  return getLink<xAOD::TruthParticle>(particle, "truthParticleLink", debug);
}

// ==================================================================
std::vector< ElementLink< xAOD::TrackParticleContainer > > xAOD::TauHelpers::trackParticleLinks(const xAOD::TauJet* tau, xAOD::TauJetParameters::TauTrackFlag flag/*=xAOD::TauJetParameters::TauTrackFlag::classifiedCharged*/){
  xAOD::TauTrack::TrackFlagType mask=1<<flag;
  return trackParticleLinksWithMask(tau, mask);
}

// ==================================================================
std::vector< ElementLink< xAOD::TrackParticleContainer > > xAOD::TauHelpers::trackParticleLinksWithMask(const xAOD::TauJet* tau, xAOD::TauTrack::TrackFlagType mask){
  std::vector< ElementLink< xAOD::TrackParticleContainer > > links;
  std::vector< const xAOD::TauTrack* > allTracks = tau->allTracks();
  for ( const xAOD::TauTrack* trk : allTracks ){
    if(trk->trackLinks().empty()) continue;
    if(trk->flagWithMask(mask)) 
      links.push_back(trk->trackLinks()[0]);    
  }
  return links;
}

// ==================================================================
std::vector<xAOD::TauTrack*> xAOD::TauHelpers::tauTracksNonConstWithMask( const xAOD::TauJet* tau, xAOD::TauTrackContainer* trackCont, xAOD::TauTrack::TrackFlagType mask ){
  std::vector<xAOD::TauTrack*> v;
  for( const ElementLink< xAOD::TauTrackContainer >& link : tau->allTauTrackLinks() ){
    if(link.isValid() && (*link)->flagWithMask(mask) && dynamic_cast<const xAOD::TauTrackContainer*> (link.getStorableObjectPointer()) == trackCont) v.push_back( trackCont->at(link.index()));
  }
  return v;
}

// ==================================================================
std::vector<xAOD::TauTrack*> xAOD::TauHelpers::allTauTracksNonConst( const xAOD::TauJet* tau, xAOD::TauTrackContainer* trackCont ){
  xAOD::TauTrack::TrackFlagType mask=0;
  return tauTracksNonConstWithMask(tau, trackCont, mask);
}



std::vector<const xAOD::IParticle*> 
xAOD::TauHelpers::clusters(const xAOD::TauJet& tau, double dRMax) {
  auto vertexedClusterList = xAOD::TauHelpers::vertexedClusters(tau, dRMax);

  std::vector<const xAOD::IParticle*> clusterList;
  for (const auto& vertexedCluster : vertexedClusterList) {
    const xAOD::CaloCluster& cluster = vertexedCluster.clust();
    clusterList.push_back(&cluster);
  }

  return clusterList;
}



std::vector<xAOD::CaloVertexedTopoCluster> 
xAOD::TauHelpers::vertexedClusters(const xAOD::TauJet& tau, double dRMax) {
  TLorentzVector tauAxis;
  if (tau.vertexLink().isValid()) {
    tauAxis = tau.p4(xAOD::TauJetParameters::IntermediateAxis); 
  }
  else {
    tauAxis = tau.p4(xAOD::TauJetParameters::DetectorAxis);
  }

  auto vertexedClusterList = tau.vertexedClusters();
  
  std::vector<xAOD::CaloVertexedTopoCluster> selectedList;
  for (const auto& vertexedCluster : vertexedClusterList) {
    TLorentzVector clusterP4 = vertexedCluster.p4();
    
    double dR = clusterP4.DeltaR(tauAxis);
    if (dR > dRMax) continue;

    selectedList.push_back(vertexedCluster);
  }

  return selectedList;
}
