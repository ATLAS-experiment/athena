/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFUnifiedMomentCalculatorTool.h"
#include "PFData.h"
#include "eflowCaloObject.h"
#include "eflowRecCluster.h"

#include "xAODCaloEvent/CaloClusterKineHelper.h"

PFUnifiedMomentCalculatorTool::PFUnifiedMomentCalculatorTool(const std::string& type,const std::string& name,const IInterface* parent) :
  base_class( type, name, parent)
{
}

StatusCode PFUnifiedMomentCalculatorTool::initialize(){

  /* Retrieve the cluster collection tool */
  ATH_CHECK(m_clusterCollectionTool.retrieve());

  /* Retrieve the cluster moments maker */
  ATH_CHECK(m_clusterMomentsMaker.retrieve());
 
  /* Retrieve the cluster calib hit moments maker */
  if (m_useCalibHitTruth) ATH_CHECK(m_clusterCalibHitMomentsMaker2.retrieve());
  else m_clusterCalibHitMomentsMaker2.disable();
 
  return StatusCode::SUCCESS;
}

StatusCode PFUnifiedMomentCalculatorTool::processPFlowData(const EventContext& ctx, PFData &thePFData) const {

  if (!thePFData.caloObjects) {
    ATH_MSG_ERROR("PFData::caloObjects is null; caller must set it before invoking the moment calculator tool");
    return StatusCode::FAILURE;
  }

  eflowCaloObjectContainer *theEflowCaloObjectContainer = thePFData.caloObjects;

  /* Collect all the clusters in a temporary container (with VIEW_ELEMENTS!) */
  bool useNonModifiedClusters = true;
  if (true == m_LCMode) useNonModifiedClusters = false;
  std::unique_ptr<xAOD::CaloClusterContainer> tempClusterContainer = m_clusterCollectionTool->execute(*theEflowCaloObjectContainer, useNonModifiedClusters);

  /* Set the layer energies */
  /* This must be set before the cluster moment calculations, which use the layer energies */
  for (auto cluster : *tempClusterContainer) CaloClusterKineHelper::calculateKine(cluster, true, true);

  /* Remake the cluster moments */
  ATH_CHECK(m_clusterMomentsMaker->execute(ctx, tempClusterContainer.get()));

  if (m_useCalibHitTruth){
    ATH_CHECK(m_clusterCalibHitMomentsMaker2->execute(ctx, tempClusterContainer.get()));
  }

  return StatusCode::SUCCESS;
}

