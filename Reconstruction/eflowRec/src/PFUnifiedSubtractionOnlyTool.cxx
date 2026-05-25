/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFUnifiedSubtractionOnlyTool.h"
#include "PFData.h"

#include "eflowCaloObject.h"
#include "eflowCaloObjectMaker.h"
#include "eflowEEtaBinnedParameters.h"
#include "eflowLayerIntegrator.h"
#include "eflowRecTrack.h"
#include "eflowTrackClusterLink.h"
#include "IEFlowCellEOverPTool.h"

#include "StoreGate/ReadDecorHandle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

using namespace eflowSubtract;

PFUnifiedSubtractionOnlyTool::PFUnifiedSubtractionOnlyTool(const std::string &type, const std::string &name, const IInterface *parent) : base_class(type, name, parent),
                                                                                                                   m_binnedParameters(std::make_unique<eflowEEtaBinnedParameters>())
{
}

PFUnifiedSubtractionOnlyTool::~PFUnifiedSubtractionOnlyTool()
= default;

StatusCode PFUnifiedSubtractionOnlyTool::initialize()
{

  ATH_CHECK(m_theEOverPTool.retrieve());

  ATH_CHECK(m_theEOverPTool->fillBinnedParameters(m_binnedParameters.get()));

  m_trkpos.reset(dynamic_cast<PFMatch::TrackEtaPhiInFixedLayersProvider *>(PFMatch::TrackPositionFactory::Get("EM2EtaPhi").release()));
  if (!m_trkpos)
  {
    ATH_MSG_ERROR("Failed to get TrackPositionProvider for cluster preselection!");
    return StatusCode::FAILURE;
  }

  if (!m_NNEnergyPredictorTool.empty()) ATH_CHECK(m_NNEnergyPredictorTool.retrieve());


  //Set the level of the helpers to the same as the tool here
  m_pfSubtractionStatusSetter.msg().setLevel(this->msg().level());
  m_pfSubtractionEnergyRatioCalculator.msg().setLevel(this->msg().level());
  m_subtractor.m_facilitator.msg().setLevel(this->msg().level());


  return StatusCode::SUCCESS;

}

StatusCode PFUnifiedSubtractionOnlyTool::processPFlowData(
  const EventContext& ctx,
  PFData &data
) const
{

  if (!data.caloObjects) {
    ATH_MSG_ERROR("PFData::caloObjects is null; caller must set it before invoking the subtraction tool");
    return StatusCode::FAILURE;
  }

  unsigned int numMatches;
  unsigned int nCaloObj;

  if (m_recoverSplitShowers){
    numMatches = data.nOrigCaloObj;
    nCaloObj = data.nOrigCaloObj;
  }
  else{
    numMatches = data.nMatches;
    nCaloObj = 0;
  }

  if (!m_calcEOverP){
    if (!m_recoverSplitShowers) performSubtraction(ctx, 0, nCaloObj, data);
    else performSubtraction(ctx, numMatches, nCaloObj, data);
  }
  //in e/p mode we want to simulate showers for all eflowCaloObjects and then decorate the eflowTrack with the relevant e/p information, but not perform any subtraction.
  //in particular simulateShowers calculates the LHED which is used for the binning of e/p when we measure it.
  else simulateShowers(ctx, *(data.caloObjects), nCaloObj);

  return StatusCode::SUCCESS;
}

void PFUnifiedSubtractionOnlyTool::simulateShowers(const EventContext& ctx, eflowCaloObjectContainer& eflowCaloObjects, const unsigned int& nCaloObj) const {

  const double gaussianRadius = 0.032;
  const double gaussianRadiusError = 1.0e-3;
  const double maximumRadiusSigma = 3.0;

  eflowLayerIntegrator integrator(gaussianRadius, gaussianRadiusError, maximumRadiusSigma, m_isHLLHC);

  //For each eflowCaloObject we calculate the expected energy deposit in the calorimeter and cell ordering for subtraction.
  for (unsigned int iCalo = nCaloObj; iCalo < eflowCaloObjects.size(); ++iCalo) {
    eflowCaloObject* thisEflowCaloObject = eflowCaloObjects.at(iCalo);
    thisEflowCaloObject->simulateShower(ctx, &integrator, m_binnedParameters.get(), m_useNNEnergy ? &(*m_NNEnergyPredictorTool) : nullptr, m_useLegacyEBinIndex);
  }
}

void PFUnifiedSubtractionOnlyTool::performSubtraction(const EventContext& ctx, const unsigned int& startingPoint, const unsigned int& nCaloObj, PFData &data ) const{

  /** Start loop from nCaloObj, which should be zero on a first pass */
  if (!m_recoverSplitShowers && 0 != nCaloObj) ATH_MSG_WARNING("Not in Split Showers Mode and already have " << nCaloObj << " eflowCaloObjects");

  simulateShowers(ctx, *(data.caloObjects), nCaloObj);

  unsigned int nEFCaloObs = data.caloObjects->size();
  for (unsigned int iCalo = startingPoint; iCalo < nEFCaloObs; ++iCalo) {
    eflowCaloObject* thisEflowCaloObject = data.caloObjects->at(iCalo);
    this->performSubtraction(*thisEflowCaloObject);
  }
}

void PFUnifiedSubtractionOnlyTool::performSubtraction(eflowCaloObject& thisEflowCaloObject) const{

  ATH_MSG_DEBUG("In performSubtraction");

  unsigned int nClusters = thisEflowCaloObject.nClusters();
  unsigned int nTrackMatches = thisEflowCaloObject.nTracks();

  ATH_MSG_DEBUG("Have got an eflowCaloObject with " << nClusters << " clusters and " << nTrackMatches << " track matches");

  if (msgLevel(MSG::DEBUG)){
    for (unsigned int iTrack = 0; iTrack < nTrackMatches; ++iTrack){
       eflowRecTrack* thisTrack = thisEflowCaloObject.efRecTrack(iTrack);
       ATH_MSG_DEBUG("eflowCaloObject has track with E, pt and eta " << thisTrack->getTrack()->e() << ", " << thisTrack->getTrack()->pt() << " and " << thisTrack->getTrack()->eta());
    }
  }
  
  //To keep historical behaviour when in recover split showers mode allow tracks with no cluster matches to proceed.
  if (!m_recoverSplitShowers && nClusters < 1) return;  

  //Need at least one track in this eflowCaloObject to continue.
  if (nTrackMatches < 1) return;

  double expectedEnergy = thisEflowCaloObject.getExpectedEnergy();
  double clusterEnergy = thisEflowCaloObject.getClusterEnergy();
  double expectedSigma = sqrt(thisEflowCaloObject.getExpectedVariance());

  /* Check e/p, if on first pass - return if e/p not consistent with expected e/p */
  if (!m_recoverSplitShowers){
    if (isEOverPFail(expectedEnergy, expectedSigma, clusterEnergy)) return;
  }
  
  const std::vector<std::pair<eflowTrackClusterLink *, std::pair<float, float>>> &matchedTrackList = thisEflowCaloObject.efRecLink();

  ATH_MSG_DEBUG("Matched Track List has size " << matchedTrackList.size());

  if (msgLevel(MSG::DEBUG))
  {
    for (unsigned int iTrack = 0; iTrack < nTrackMatches; ++iTrack)
    {
      const xAOD::TrackParticle *thisTrack = thisEflowCaloObject.efRecTrack(iTrack)->getTrack();      
      ATH_MSG_DEBUG("eflowCaloObject has track match with E, pt and eta " << thisTrack->e() << ", " << thisTrack->pt() << " and " << thisTrack->eta());
    }
  }

  ATH_MSG_DEBUG("About to perform subtraction for this eflowCaloObject");

  bool wasAnnihilated = false;

  //First deal with non-split showers mode
  if (!m_recoverSplitShowers){
    /* Check if we can annihilate right away - true if matched cluster has only the expected energy deposit */
    if (canAnnihilate(expectedEnergy, expectedSigma, clusterEnergy)){

      wasAnnihilated = true;

      std::vector<std::pair<xAOD::CaloCluster *, bool>> clusterList;
      std::map<xAOD::CaloCluster *, double> clusterEnergyMap;
      unsigned nCluster = thisEflowCaloObject.nClusters();
      for (unsigned iCluster = 0; iCluster < nCluster; ++iCluster){
        clusterList.emplace_back(thisEflowCaloObject.efRecCluster(iCluster)->getCluster(), false);
      }

      ATH_MSG_DEBUG("We are going to annihilate. ExpectedEnergy, expectedSigma and clusterEnergy are " << expectedEnergy << ", " << expectedSigma << " and " << clusterEnergy);
      if (msgLevel(MSG::DEBUG))
        for (auto thisPair : clusterList)
          ATH_MSG_DEBUG("Annihilating cluster with E and eta " << thisPair.first->e() << " and " << thisPair.first->eta());

      m_pfSubtractionStatusSetter.markAllTracksAnnihStatus(thisEflowCaloObject);

      //before we remove all the cells, we create a list of the removed cells if in doCPData mode
      if (m_addCPData) this->addSubtractedCells(thisEflowCaloObject, clusterList);

      Subtractor::annihilateClusters(clusterList);

      if (msgLevel(MSG::DEBUG))
        for (auto thisPair : clusterList)
          ATH_MSG_DEBUG("Have Annihilated cluster with E and eta " << thisPair.first->e() << " and " << thisPair.first->eta());
      
      /* Flag all tracks in this system as subtracted */
      for (unsigned iTrack = 0; iTrack < thisEflowCaloObject.nTracks(); ++iTrack){
        eflowRecTrack *thisEfRecTrack = (matchedTrackList[iTrack].first)->getTrack();
        if (!thisEfRecTrack->isSubtracted()) thisEfRecTrack->setSubtracted();
      }

    }//if can annihilate this track-cluster systems matched cluster
  }//split shower recovery mode or regular mode where above annihilation was not triggered
  if (m_recoverSplitShowers || !wasAnnihilated){

    for (unsigned iTrack = 0; iTrack < thisEflowCaloObject.nTracks(); ++iTrack){

      eflowRecTrack *thisEfRecTrack = thisEflowCaloObject.efRecTrack(iTrack);

      ATH_MSG_DEBUG("About to subtract track with e, pt, eta and phi of " << thisEfRecTrack->getTrack()->e() << ", " << thisEfRecTrack->getTrack()->pt() << ", " << thisEfRecTrack->getTrack()->eta() << " and "
                                                                       << thisEfRecTrack->getTrack()->eta());
      
      if (!thisEfRecTrack->hasBin()) continue;

      ATH_MSG_DEBUG("Have bin for this eflowCaloObject");

      if (thisEfRecTrack->isInDenseEnvironment() && !m_recoverSplitShowers) continue;

      ATH_MSG_DEBUG("Am not in dense environment for this eflowCaloObject");

      /* Get matched cluster via Links */
      std::vector<eflowRecCluster *> matchedClusters;
      const std::vector<eflowTrackClusterLink *>& links = thisEfRecTrack->getClusterMatches();
      matchedClusters.reserve(links.size());
      for (auto* thisEFlowTrackClusterLink : links)
        matchedClusters.push_back(thisEFlowTrackClusterLink->getCluster());
      if (m_recoverSplitShowers)
        std::sort(matchedClusters.begin(),
                  matchedClusters.end(),
                  eflowRecCluster::SortDescendingPt());

      if (msgLvl(MSG::DEBUG)) {
        for (auto* thisClus : matchedClusters)
          ATH_MSG_DEBUG(
            "Haved matched cluster "
            << thisClus->getCluster()->index() << " with e,pt, eta and phi of "
            << thisClus->getCluster()->e() << ", "
            << thisClus->getCluster()->pt() << ", "
            << thisClus->getCluster()->eta() << " and "
            << thisClus->getCluster()->phi() << " will be subtracted");
      }

      /* Do subtraction */
      std::vector<std::pair<xAOD::CaloCluster *, bool>> clusterSubtractionList;
      clusterSubtractionList.reserve(matchedClusters.size());
      std::map<xAOD::CaloCluster *, double> clusterEnergyMap;
      for (auto *thisEFlowRecCluster : matchedClusters){
        xAOD::CaloCluster *thisCluster = thisEFlowRecCluster->getCluster();
        clusterSubtractionList.emplace_back(thisCluster, false);
        clusterEnergyMap[thisCluster] = thisCluster->e();
      }

      ATH_MSG_DEBUG("Have filled clusterSubtractionList for this eflowCaloObject");

      unsigned int trackIndex = thisEfRecTrack->getTrack()->index();

      //Previously we only checked this in recover split showers, but makes sense to check it in both passes.
      auto sumClusEnergy = [](double accumulator, std::pair<xAOD::CaloCluster *, bool> thisPair){ return accumulator += thisPair.first->e();};
      double totalClusterEnergy = std::accumulate(clusterSubtractionList.begin(),clusterSubtractionList.end(),0.0,sumClusEnergy);      

      /* Check if we can annihilate right away - true if matched cluster has only the expected energy deposit */
      if(canAnnihilate(thisEfRecTrack->getEExpect(),sqrt(thisEfRecTrack->getVarEExpect()),totalClusterEnergy)){
        
        if (msgLevel(MSG::DEBUG))
          for (auto thisPair : clusterSubtractionList)
            ATH_MSG_DEBUG("Annihilating cluster with E and eta " << thisPair.first->e() << " and " << thisPair.first->eta());

        //before we remove all the cells, we create a list of the removed cells if in doCPData mode
        if (m_addCPData) this->addSubtractedCells(thisEflowCaloObject, clusterSubtractionList);

        Subtractor::annihilateClusters(clusterSubtractionList);
        //Now we should mark all of these clusters as being subtracted
        //Now need to mark which clusters were modified in the subtraction procedure
        std::vector<std::pair<float, float>> clusterSubtractedEnergyRatios;
        m_pfSubtractionEnergyRatioCalculator.calculateSubtractedEnergyRatiosForAnnih(clusterSubtractionList, clusterEnergyMap, clusterSubtractedEnergyRatios);
        m_pfSubtractionStatusSetter.markSubtractionStatus(clusterSubtractionList, clusterSubtractedEnergyRatios, thisEflowCaloObject, trackIndex);
      }
      else
      {

        /* Subtract the track from all matched clusters */
        m_subtractor.subtractTracksFromClusters(thisEfRecTrack, clusterSubtractionList, m_addCPData);

        //recalculate total cluster energy from the clusters afer subtraction
        totalClusterEnergy = std::accumulate(clusterSubtractionList.begin(),clusterSubtractionList.end(),0.0,sumClusEnergy);        

        /* Annihilate the cluster(s) if the remnant is small (i.e. below k*sigma) */
        if (canAnnihilate(0.0,sqrt(thisEfRecTrack->getVarEExpect()), totalClusterEnergy)){

          if (msgLevel(MSG::DEBUG))
          for (auto thisPair : clusterSubtractionList){
            ATH_MSG_DEBUG("Annihilating remnant cluster with E and eta " << thisPair.first->e() << " and " << thisPair.first->eta());
          }
          eflowSubtract::Subtractor::annihilateClusters(clusterSubtractionList);
          //Now we should mark all of these clusters as being subtracted
          std::vector<std::pair<float, float>> clusterSubtractedEnergyRatios;
          m_pfSubtractionEnergyRatioCalculator.calculateSubtractedEnergyRatiosForAnnih(clusterSubtractionList, clusterEnergyMap, clusterSubtractedEnergyRatios);
          m_pfSubtractionStatusSetter.markSubtractionStatus(clusterSubtractionList, clusterSubtractedEnergyRatios, thisEflowCaloObject, trackIndex);
        }//if remove the remnant after cell by cell subtraction
        else
        { 
          std::vector<std::pair<float, float>> clusterSubtractedEnergyRatios;
          m_pfSubtractionEnergyRatioCalculator.calculateSubtractedEnergyRatios(clusterSubtractionList, clusterEnergyMap, clusterSubtractedEnergyRatios);
          m_pfSubtractionStatusSetter.markSubtractionStatus(clusterSubtractionList, clusterSubtractedEnergyRatios, thisEflowCaloObject, trackIndex);
        }//if don't remove the remnant after cell by cell subtraction

      }//if not annihilating, and instead subtracting cell by cell

      ATH_MSG_DEBUG("Have subtracted charged shower for this eflowRecTrack");

      /* Flag tracks as subtracted */
      if (!thisEfRecTrack->isSubtracted()) thisEfRecTrack->setSubtracted();

    }//loop over tracks in eflowCaloObject
  }//cell by cell subtraction

}

bool PFUnifiedSubtractionOnlyTool::isEOverPFail(double expectedEnergy, double sigma, double clusterEnergy) const
{
  if ((expectedEnergy == 0) && (clusterEnergy > 0)) return false;
  return clusterEnergy < expectedEnergy - m_consistencySigmaCut * sigma;
}

bool PFUnifiedSubtractionOnlyTool::canAnnihilate(double expectedEnergy, double sigma, double clusterEnergy) const
{
  return clusterEnergy - expectedEnergy < m_subtractionSigmaCut * sigma;
}

void PFUnifiedSubtractionOnlyTool::addSubtractedCells(eflowCaloObject& thisEflowCaloObject, const std::vector<std::pair<xAOD::CaloCluster *, bool> >& clusterList) const{

  unsigned int numTracks = thisEflowCaloObject.nTracks();

  for (unsigned int iTrack = 0; iTrack < numTracks; ++iTrack){
    eflowRecTrack* thisTrack = thisEflowCaloObject.efRecTrack(iTrack);
    for (const auto& thisPair : clusterList){
      xAOD::CaloCluster* thisCluster = thisPair.first;
      const CaloClusterCellLink* theCellLink = thisCluster->getCellLinks();
      CaloClusterCellLink::const_iterator theCell = theCellLink->begin();
      CaloClusterCellLink::const_iterator lastCell = theCellLink->end();
      for (; theCell != lastCell; ++theCell) thisTrack->addSubtractedCaloCell(ElementLink<CaloCellContainer>("AllCalo",theCell.index()),theCell.weight()/numTracks);
    }
  }
}
