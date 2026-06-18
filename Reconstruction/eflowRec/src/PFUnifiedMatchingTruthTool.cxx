/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFUnifiedMatchingTruthTool.h"
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
#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TruthUtils/MagicNumbers.h"

using namespace eflowSubtract;


PFUnifiedMatchingTruthTool::~PFUnifiedMatchingTruthTool()
= default;

StatusCode PFUnifiedMatchingTruthTool::initialize()
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


  if (!m_caloClusterReadDecorHandleKeyNLeadingTruthParticles.empty()){
    ATH_CHECK(m_caloClusterReadDecorHandleKeyNLeadingTruthParticles.initialize());
  }

  return StatusCode::SUCCESS;

}

unsigned int PFUnifiedMatchingTruthTool::matchAndCreateEflowCaloObj(const EventContext& ctx, PFData &data) const{

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


    const xAOD::TruthParticle* trackMatchedTruthParticle = nullptr;
    typedef ElementLink<xAOD::TruthParticleContainer> TruthLink;

    const static SG::AuxElement::Accessor<TruthLink> truthLinkAccessor("truthParticleLink");
    
    TruthLink truthLink = truthLinkAccessor(*(thisEfRecTrack->getTrack()));
    //if not valid don't print a WARNING because this is an expected condition as discussed here:
    //https://indico.cern.ch/event/795039/contributions/3391771/attachments/1857138/3050771/TruthTrackFTAGWS.pdf
    if (truthLink.isValid()) trackMatchedTruthParticle = *truthLink;

    if (trackMatchedTruthParticle){
      double uniqueID = HepMC::uniqueID(trackMatchedTruthParticle);

      SG::ReadDecorHandle<xAOD::CaloClusterContainer, std::vector< std::pair<unsigned int, double> > > caloClusterReadDecorHandleNLeadingTruthParticles(m_caloClusterReadDecorHandleKeyNLeadingTruthParticles, ctx);
      if (!caloClusterReadDecorHandleNLeadingTruthParticles.isValid()){
        ATH_MSG_WARNING("Failed to retrieve CaloCluster decoration with key " << caloClusterReadDecorHandleNLeadingTruthParticles.key());
      }

      for (auto * thisCluster : data.clusters){
        //accessor for decoration
        //split key into substring to get the name of the decoration

        std::string decorHandleName = m_caloClusterReadDecorHandleKeyNLeadingTruthParticles.key();
        std::string::size_type pos = decorHandleName.find(".");
        std::string decorName = decorHandleName.substr(pos+1);

        SG::AuxElement::Accessor< std::vector< std::pair<unsigned int, double> > > accessor(decorName);

        std::vector<std::pair<unsigned int, double > > uniqueIDTruthPairs = accessor(*(thisCluster->getCluster()));

        for (auto &uniqueIDTruthPair : uniqueIDTruthPairs){
          if (uniqueIDTruthPair.first == uniqueID){
            eflowTrackClusterLink* thisLink = eflowTrackClusterLink::getInstance(thisEfRecTrack, thisCluster, ctx);
            bestClusters.push_back(thisLink);
            break;
          }
        }//loop over calocluster truth pair decorations
      }//loop over caloclusters
      
    }//if have truth particle matched to track
    else ATH_MSG_VERBOSE("Track with pt, eta and phi " << thisEfRecTrack->getTrack()->pt() << ", " << thisEfRecTrack->getTrack()->eta() << " and " << thisEfRecTrack->getTrack()->phi() << " does not have a valid truth pointer");
  
    

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
    for (auto *trkClusLink : bestClusters){

      eflowRecCluster *thisEFRecCluster = trkClusLink->getCluster();

      if (m_recoverSplitShowers){
        // Look up whether this cluster is intended for recovery
        if (std::find(data.clusters.begin(), data.clusters.end(), trkClusLink->getCluster()) == data.clusters.end()) {
          continue;       
        }
      }

      eflowTrackClusterLink *trackClusterLink = eflowTrackClusterLink::getInstance(thisEfRecTrack, thisEFRecCluster, ctx);
      thisEfRecTrack->addClusterMatch(trackClusterLink);

      thisEFRecCluster->addTrackMatch(trackClusterLink);
    }
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
