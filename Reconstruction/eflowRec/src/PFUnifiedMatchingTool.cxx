/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFUnifiedMatchingTool.h"
#include "PFData.h"

#include "eflowCaloObject.h"
#include "eflowCaloObjectMaker.h"
#include "eflowEEtaBinnedParameters.h"
#include "eflowLayerIntegrator.h"
#include "eflowRecTrack.h"
#include "eflowTrackClusterLink.h"
#include "IEFlowCellEOverPTool.h"
#include "PFClusterFiller.h"
#include "PFTrackFiller.h"

#include "StoreGate/ReadDecorHandle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

#include <sstream>

using namespace eflowSubtract;

PFUnifiedMatchingTool::PFUnifiedMatchingTool(const std::string &type, const std::string &name, const IInterface *parent) : base_class(type, name, parent),
                                                                                                                   m_binnedParameters(std::make_unique<eflowEEtaBinnedParameters>())
{
}

PFUnifiedMatchingTool::~PFUnifiedMatchingTool()
= default;

StatusCode PFUnifiedMatchingTool::initialize()
{

  ATH_CHECK(m_theEOverPTool.retrieve());

  ATH_CHECK(m_theEOverPTool->fillBinnedParameters(m_binnedParameters.get()));

  m_trkpos.reset(dynamic_cast<PFMatch::TrackEtaPhiInFixedLayersProvider *>(PFMatch::TrackPositionFactory::Get("EM2EtaPhi").release()));
  if (!m_trkpos)
  {
    ATH_MSG_ERROR("Failed to get TrackPositionProvider for cluster preselection!");
    return StatusCode::FAILURE;
  }

  //Retrieve track-cluster matching tools
  ATH_CHECK(m_theMatchingTool.retrieve());
  ATH_CHECK(m_theMatchingToolForPull_015.retrieve());
  ATH_CHECK(m_theMatchingToolForPull_02.retrieve());

  return StatusCode::SUCCESS;

}

StatusCode PFUnifiedMatchingTool::processPFlowData(
  const EventContext& ctx,
  PFData &data
) const
{

  if (!data.caloObjects) {
    ATH_MSG_ERROR("PFData::caloObjects is null; caller must set it before invoking the matching tool");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Executing");

  if (m_recoverSplitShowers){
    // reset tracks in data and refill only with those needed for split showers
    data.tracks.clear();
    PFTrackFiller::fillTracksToRecover(data);
  }

  if (m_recoverSplitShowers){
    // reset clusters in data and refill only with those needed for split showers
    data.clusters.clear();
    PFClusterFiller::fillClustersToRecover(data);
  }

  ATH_MSG_DEBUG("This event has " << data.tracks.size() << " tracks " << data.clusters.size() << " clusters ");

  if (msgLvl(MSG::DEBUG)) printAllClusters(data.clusters);

  unsigned int numMatches = matchAndCreateEflowCaloObj(ctx, data);

  // store numMatches for later use in subtraction tool
  if (m_recoverSplitShowers) data.nOrigCaloObj = numMatches;
  else data.nMatches = numMatches;


  return StatusCode::SUCCESS;
}

unsigned int PFUnifiedMatchingTool::matchAndCreateEflowCaloObj(const EventContext& ctx, PFData &data) const{

  //Counts up how many tracks found at least 1 calorimeter cluster matched to it.
  unsigned int nMatches(0);

  /* Cache the original number of eflowCaloObjects, if there were any */
  const unsigned int nCaloObj = data.caloObjects->size();

  /* loop tracks in data.tracks and do matching */
  for (auto *thisEfRecTrack : data.tracks)
  {
    /** No point to do anything if e/p reference bin does not exist */
    if (!thisEfRecTrack->hasBin()) {
      std::unique_ptr<eflowCaloObject> thisEflowCaloObject = std::make_unique<eflowCaloObject>();
      thisEflowCaloObject->addTrack(thisEfRecTrack);
      data.caloObjects->push_back(std::move(thisEflowCaloObject));
      continue;
    }

    if (msgLvl(MSG::DEBUG))
    {
      const xAOD::TrackParticle *track = thisEfRecTrack->getTrack();
      ATH_MSG_DEBUG("Matching track with e,pt, eta and phi " << track->e() << ", " << track->pt() << ", " << track->eta() << " and " << track->phi());
    }

    std::vector<eflowTrackClusterLink*> bestClusters;
    std::vector<float> deltaRPrime;

    if (!m_recoverSplitShowers){
      /** Add cluster matches needed for pull calculation (in eflowCaloObject::simulateShowers) which is used to determine whether to run the charged shower subtraction or not.
      / Clusters in both a cone of 0.15 and 0.2 are needed for this.
      / The clusters in a cone of 0.2 are also used as the matched cluster list for recover split showers mode.    
      **/
      std::vector<std::pair<eflowRecCluster *, float>> bestClusters_02 = m_theMatchingToolForPull_02->doMatches(thisEfRecTrack, data.clusters, -1);
      for (auto &matchpair : bestClusters_02)
      {
        eflowRecCluster *theCluster = matchpair.first;
        float distancesq = matchpair.second;
        eflowTrackClusterLink *trackClusterLink = eflowTrackClusterLink::getInstance(thisEfRecTrack, theCluster, ctx);
        if (distancesq < 0.15 * 0.15)
        {
          // Narrower cone is a subset of the selected clusters
          // Distance returned is deltaR^2
          thisEfRecTrack->addAlternativeClusterMatch(trackClusterLink, "cone_015");
        }
        thisEfRecTrack->addAlternativeClusterMatch(trackClusterLink, "cone_02");
      }//loop over bestClusters_02

      //This matching scheme is used to match the calorimeter cluster(s) to be used in the charged showers subtraction for this track.
      std::vector<std::pair<eflowRecCluster *, float>> matchedClusters = m_theMatchingTool->doMatches(thisEfRecTrack, data.clusters,m_nClusterMatchesToUse);    
      for (auto thePair : matchedClusters) {
        bestClusters.push_back(eflowTrackClusterLink::getInstance(thisEfRecTrack, thePair.first, ctx));     
        if (m_addCPData) deltaRPrime.push_back(std::sqrt(thePair.second));
      }
    }
    else {
      const std::vector<eflowTrackClusterLink*>* matchedClusters_02 = thisEfRecTrack->getAlternativeClusterMatches("cone_02");
      if (!matchedClusters_02) continue;
      else bestClusters = *matchedClusters_02;
    }

    if (bestClusters.empty()) continue;

    if (msgLvl(MSG::DEBUG))
    {
      for (auto *thisClusterLink : bestClusters ) {
        xAOD::CaloCluster* thisCluster = thisClusterLink->getCluster()->getCluster();
        ATH_MSG_DEBUG("Matched this track to cluster with e,pt, eta and phi " << thisCluster->e() << ", " << thisCluster->pt() << ", " << thisCluster->eta() << " and " << thisCluster->phi());
      }
    }

    nMatches++;

    //loop over the matched calorimeter clusters and associate tracks and clusters to each other as needed.
    unsigned int linkIndex = 0;
    for (auto *trkClusLink : bestClusters){

      eflowRecCluster *thisEFRecCluster = trkClusLink->getCluster();

      if (m_recoverSplitShowers){
        // Look up whether this cluster is intended for recovery
        if (std::find(data.clusters.begin(), data.clusters.end(), trkClusLink->getCluster()) == data.clusters.end()) {
          linkIndex++;
          continue;       
        }
      }

      eflowTrackClusterLink *trackClusterLink = eflowTrackClusterLink::getInstance(thisEfRecTrack, thisEFRecCluster, ctx);
      thisEfRecTrack->addClusterMatch(trackClusterLink);
      if (m_addCPData) {
        thisEfRecTrack->addDeltaRPrime(deltaRPrime[linkIndex]);
      }
      thisEFRecCluster->addTrackMatch(trackClusterLink);
    }
     linkIndex++;
  }

  /* Create 3 types eflowCaloObjects: track-only, cluster-only, track-cluster-link */
  std::vector<eflowRecCluster *> clusters(data.clusters.begin(), data.clusters.end());
  if (m_recoverSplitShowers) std::sort(clusters.begin(), clusters.end(), eflowRecCluster::SortDescendingPt());
  unsigned int nCaloObjects = eflowCaloObjectMaker::makeTrkCluCaloObjects(data.tracks, clusters, data.caloObjects);
  ATH_MSG_DEBUG("Created  " << nCaloObjects << " eflowCaloObjects.");
  if (msgLvl(MSG::DEBUG)){
    for (auto thisEFlowCaloObject : *(data.caloObjects)){
      ATH_MSG_DEBUG("This eflowCaloObject has " << thisEFlowCaloObject->nTracks() << " tracks and " << thisEFlowCaloObject->nClusters() << " clusters ");
      for (unsigned int count = 0; count < thisEFlowCaloObject->nTracks(); count++){
        const xAOD::TrackParticle* thisTrack = thisEFlowCaloObject->efRecTrack(count)->getTrack();
        ATH_MSG_DEBUG("Have track with e, pt, eta and phi of " << thisTrack->e() << ", " << thisTrack->pt() << ", " << thisTrack->eta() << " and " << thisTrack->phi());
      }
      for (unsigned int count = 0; count < thisEFlowCaloObject->nClusters(); count++){
        const xAOD::CaloCluster* thisCluster = thisEFlowCaloObject->efRecCluster(count)->getCluster();
        ATH_MSG_DEBUG("Have cluster with e, pt, eta and phi of " << thisCluster->e() << ", " << thisCluster->pt() << ", " << thisCluster->eta() << " and " << thisCluster->phi());
      }
    }
  }

  if (!m_recoverSplitShowers) return nMatches;
  else return nCaloObj;
}


std::string PFUnifiedMatchingTool::printTrack(const xAOD::TrackParticle* track) {
  std::stringstream result;
  result << " track with E, eta and phi "<< track->e() << ", " << track->eta() << " and " << track->phi();
  return result.str();
}

std::string PFUnifiedMatchingTool::printCluster(const xAOD::CaloCluster* cluster) {
  std::stringstream result;
  result << " cluster with E, eta and phi of " << cluster->e() << ", " << cluster->eta() << " and " << cluster->phi();
  return result.str();
}

void PFUnifiedMatchingTool::printAllClusters(const std::vector<eflowRecCluster *>& recClusterVector) const {

  for ( const auto *thisEFRecCluster : recClusterVector){
    if (thisEFRecCluster->getTrackMatches().empty()) {
      ATH_MSG_DEBUG("Isolated" << printCluster(thisEFRecCluster->getCluster()));
    } else {
      ATH_MSG_DEBUG("Matched" << printCluster(thisEFRecCluster->getCluster()));
      std::vector<eflowTrackClusterLink*> theTrackLinks = thisEFRecCluster->getTrackMatches();
      for ( auto *thisTrack : theTrackLinks){
       ATH_MSG_DEBUG("Matched" << printTrack(thisTrack->getTrack()->getTrack()));
      }
    }
  }
}

