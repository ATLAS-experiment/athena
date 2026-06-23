/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFUnifiedSubtractionOnlyTruthTool.h"
#include "PFData.h"

#include "eflowCaloObject.h"
#include "eflowCaloObjectMaker.h"
#include "eflowEEtaBinnedParameters.h"
#include "eflowLayerIntegrator.h"
#include "eflowRecTrack.h"
#include "eflowTrackClusterLink.h"
#include "IEFlowCellEOverPTool.h"

#include "StoreGate/ReadDecorHandle.h"
#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "xAODTruth/TruthParticleContainer.h"

using namespace eflowSubtract;

PFUnifiedSubtractionOnlyTruthTool::~PFUnifiedSubtractionOnlyTruthTool()
= default;


StatusCode PFUnifiedSubtractionOnlyTruthTool::initialize()
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

  if (!m_theTruthShowerSimulator.empty()) ATH_CHECK(m_theTruthShowerSimulator.retrieve());


  //Set the level of the helpers to the same as the tool here
  m_pfSubtractionStatusSetter.msg().setLevel(this->msg().level());
  m_pfSubtractionEnergyRatioCalculator.msg().setLevel(this->msg().level());
  m_subtractor.m_facilitator.msg().setLevel(this->msg().level());


  return StatusCode::SUCCESS;

}

StatusCode PFUnifiedSubtractionOnlyTruthTool::processPFlowData(
  const EventContext& ctx,
  PFData &data
) const
{
  if (!data.caloObjects) {
    ATH_MSG_ERROR("PFData::caloObjects is null; caller must set it before invoking the truth subtraction tool");
    return StatusCode::FAILURE;
  }

  performSubtraction(ctx, data.nMatches, 0, data);
  return StatusCode::SUCCESS;
}


void PFUnifiedSubtractionOnlyTruthTool::performSubtraction(const EventContext& ctx, const unsigned int& startingPoint, const unsigned int& nCaloObj, PFData &data ) const{

  //starting point is defined in the base class API and used in other derived API.
  //It is not used here, so we silence compiler warnings about it being unused
  (void)startingPoint;

  ATH_MSG_DEBUG("In performTruthSubtraction");

  const double gaussianRadius = 0.032;
  const double gaussianRadiusError = 1.0e-3;
  const double maximumRadiusSigma = 3.0;

  eflowLayerIntegrator integrator(gaussianRadius, gaussianRadiusError, maximumRadiusSigma, m_isHLLHC);

  /** Start loop from nCaloObj, which should be zero on a first pass */
  if (!m_recoverSplitShowers && 0 != nCaloObj) ATH_MSG_WARNING("Not in Split Showers Mode and already have " << nCaloObj << " eflowCaloObjects");

  //For each eflowCaloObject we calculate the expected energy deposit in the calorimeter and cell ordering for subtraction.
  for (unsigned int iCalo = nCaloObj; iCalo < data.caloObjects->size(); ++iCalo) {
    eflowCaloObject* thisEflowCaloObject = data.caloObjects->at(iCalo);
    thisEflowCaloObject->simulateShower(ctx, &integrator, m_binnedParameters.get(), m_useNNEnergy ? &(*m_NNEnergyPredictorTool) : nullptr, m_useLegacyEBinIndex);
    m_theTruthShowerSimulator->simulateShower(*thisEflowCaloObject);    

  }

  unsigned int nEFCaloObs = data.caloObjects->size();

  for (unsigned int iCalo = 0; iCalo < nEFCaloObs; ++iCalo) {
    eflowCaloObject* thisEflowCaloObject = data.caloObjects->at(iCalo);
    this->performSubtraction(*thisEflowCaloObject);
  }

}

void PFUnifiedSubtractionOnlyTruthTool::performSubtraction(eflowCaloObject& thisEflowCaloObject) const{

  for (unsigned iTrack = 0; iTrack < thisEflowCaloObject.nTracks(); ++iTrack){
    eflowRecTrack *thisEfRecTrack = thisEflowCaloObject.efRecTrack(iTrack);

    //although we are subtracting the truth, to be consistent we only do it if a reco
    //e/p lookup bin exists for this track
    if (!thisEfRecTrack->hasBin()) continue;

    //Similarly we skip tracks in a dense environment
    if (thisEfRecTrack->isInDenseEnvironment()) continue;

    thisEfRecTrack->setSubtracted();

    //get the set of matched clusters
    std::vector<eflowTrackClusterLink *> links = thisEfRecTrack->getClusterMatches();

    for (auto thisLink : links){
      xAOD::CaloCluster *thisCluster = thisLink->getCluster()->getCluster();
      CaloClusterCellLink* theCellLinks = thisCluster->getOwnCellLinks();
      CaloClusterCellLink::iterator theCell = theCellLinks->begin();
      CaloClusterCellLink::iterator lastCell = theCellLinks->end();

      //loop over the cells in this cluster and subtract shower using truth information
      //We can either remove a cell entireley if it has any truth deposit (closer to what the real 
      //reco algorithm does) or reweight the cells contribution based on subtracting the truth
      //energy from the reco cell energy
      //We only advance the iterator, theCell, if we *dont* call removeCell to avoid issues with
      //invalid iterators
      //We also have to reset the lastCell iterator after each call to ensure the loop exits at the end, 
      //instead of being stuck in an infinite loop.
      for (; theCell != lastCell;){
        //get the truth energy for this cell
        double truthEnergy = thisEfRecTrack->getCellTruthEnergy(*theCell);
        //reweight the cell such that energy*weight gives the new energy
        double oldCellEnergy = theCell->energy()*(theCell.weight());
        double subtractedCellWeight = (oldCellEnergy - truthEnergy)/oldCellEnergy;

        if (0.0 != truthEnergy && m_useFullCellTruthSubtraction) {
          thisCluster->removeCell(*theCell);
          lastCell = theCellLinks->end();
        }
        else if (!m_useFullCellTruthSubtraction) {
          theCell.reweight(subtractedCellWeight);
          ++theCell;
        }
        else ++theCell;

      }//cell loop

      float oldEnergy = thisCluster->e();
      CaloClusterKineHelper::calculateKine(thisCluster, true, true);
      if (0.0 != oldEnergy) {
        float energyAdjustment = thisCluster->e() / oldEnergy;
        thisCluster->setRawE(thisCluster->rawE() * energyAdjustment);
        thisCluster->setRawEta(thisCluster->eta());
        thisCluster->setRawPhi(thisCluster->phi());
      }
    }

  }//eflowCaloObject track loop

}
