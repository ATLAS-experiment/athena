/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCalibHitRec/CaloCalibClusterDecoratorTool.h"
//Core classes
#include "StoreGate/WriteDecorHandle.h"
#include <typeinfo>

CaloCalibClusterDecoratorTool::CaloCalibClusterDecoratorTool(
    const std::string& type,
    const std::string& name,
    const IInterface* parent)
  : AthAlgTool(type, name, parent)
{}

StatusCode CaloCalibClusterDecoratorTool::initialize() {
  ATH_MSG_INFO("Decor key container: " << m_caloClusterWriteDecorHandleKeyNLeadingTruthParticles.key());
  ATH_MSG_DEBUG("Recording truth map with key: " << m_mapIdentifierToCalibHitsReadHandleKey.key());
  ATH_CHECK(m_mapIdentifierToCalibHitsReadHandleKey.initialize());
  ATH_CHECK(m_truthAttributerTool.retrieve());
  ATH_CHECK(m_caloClusterWriteDecorHandleKeyNLeadingTruthParticles.initialize());

  return StatusCode::SUCCESS;
}

StatusCode CaloCalibClusterDecoratorTool::execute(
    const EventContext& ctx,
    xAOD::CaloClusterContainer* theClusColl) const 
{
  SG::ReadHandle<std::map<Identifier,std::vector<const CaloCalibrationHit*> > > mapIdentifierToCalibHitsReadHandle(
      m_mapIdentifierToCalibHitsReadHandleKey, ctx);
  
  if (!mapIdentifierToCalibHitsReadHandle.isValid()) {
    ATH_MSG_WARNING("Could not retrieve map between Identifier and calibraiton hits from Storegae");
    return StatusCode::FAILURE;
  }  

  SG::WriteDecorHandle<xAOD::CaloClusterContainer, 
                       std::vector<std::pair<unsigned int, double> > > 
      caloClusterWriteDecorHandleNLeadingTruthParticles(
          m_caloClusterWriteDecorHandleKeyNLeadingTruthParticles, ctx);

  for (const xAOD::CaloCluster* thisCaloCluster : *theClusColl) {

    std::vector<std::pair<unsigned int, double> > newTruthIDTruthPairs;
    ATH_CHECK(
      m_truthAttributerTool->calculateTruthEnergies(
        *thisCaloCluster,
        m_numTruthParticles,
        *mapIdentifierToCalibHitsReadHandle,
        newTruthIDTruthPairs
      )
    );

    for (const auto& thisPair : newTruthIDTruthPairs) 
      ATH_MSG_DEBUG(
          "Cluster Final loop: Particle with truthID " << thisPair.first
          << " has truth energy of " << thisPair.second
          << " for cluster with e, eta " << thisCaloCluster->e()
          << " and " << thisCaloCluster->eta()
      );

    caloClusterWriteDecorHandleNLeadingTruthParticles(*thisCaloCluster) = std::move(newTruthIDTruthPairs);
  }
  
  return StatusCode::SUCCESS;
}

StatusCode CaloCalibClusterDecoratorTool::finalize() {
  return StatusCode::SUCCESS;
}