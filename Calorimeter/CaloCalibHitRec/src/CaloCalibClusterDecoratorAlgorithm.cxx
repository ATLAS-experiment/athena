/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCalibClusterDecoratorAlgorithm.h"

//Core classes
#include "StoreGate/WriteDecorHandle.h"

StatusCode CaloCalibClusterDecoratorAlgorithm::initialize(){

  ATH_CHECK(m_mapIdentifierToCalibHitsReadHandleKey.initialize());

  ATH_CHECK(m_caloClusterWriteDecorHandleKeyNLeadingTruthParticles.initialize());

  ATH_CHECK(m_truthAttributerTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode CaloCalibClusterDecoratorAlgorithm::execute(const EventContext& ctx) const{

  SG::ReadHandle<std::map<Identifier,std::vector<const CaloCalibrationHit*> > > mapIdentifierToCalibHitsReadHandle(m_mapIdentifierToCalibHitsReadHandleKey, ctx);
  if(!mapIdentifierToCalibHitsReadHandle.isValid()){
    ATH_MSG_WARNING("Could not retrieve map between Identifier and calibraiton hits from Storegae");
    return StatusCode::FAILURE;
  }  

  SG::WriteDecorHandle<xAOD::CaloClusterContainer, std::vector< std::pair<unsigned int, double> > > caloClusterWriteDecorHandleNLeadingTruthParticles(m_caloClusterWriteDecorHandleKeyNLeadingTruthParticles, ctx);
  
  for (const auto *thisCaloCluster : *caloClusterWriteDecorHandleNLeadingTruthParticles){

    std::vector<std::pair<unsigned int, double > > newTruthIDTruthPairs;
    ATH_CHECK(m_truthAttributerTool->calculateTruthEnergies(*thisCaloCluster, m_numTruthParticles, *mapIdentifierToCalibHitsReadHandle, newTruthIDTruthPairs));
    
    for (const auto& thisPair : newTruthIDTruthPairs) ATH_MSG_DEBUG("Cluster Final loop: Particle with truthID " << thisPair.first << " has truth energy of " <<  thisPair.second << " for cluster with e, eta " << thisCaloCluster->e() << " and " << thisCaloCluster->eta());
    caloClusterWriteDecorHandleNLeadingTruthParticles(*thisCaloCluster) = std::move(newTruthIDTruthPairs);
  }
  
  return StatusCode::SUCCESS;
}

StatusCode CaloCalibClusterDecoratorAlgorithm::finalize(){
  return StatusCode::SUCCESS;
}
