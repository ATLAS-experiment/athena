/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PFlowCalibPFODecoratorAlgorithm.h"

//Core classes
#include "StoreGate/WriteDecorHandle.h"

StatusCode PFlowCalibPFODecoratorAlgorithm::initialize(){

  ATH_CHECK(m_mapIdentifierToCalibHitsReadHandleKey.initialize());

  ATH_CHECK(m_pfoWriteDecorHandleKeyNLeadingTruthParticles.initialize());

  ATH_CHECK(m_truthAttributerTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode PFlowCalibPFODecoratorAlgorithm::LinkCalibHitPFO(
							    SG::WriteDecorHandle<xAOD::FlowElementContainer, std::vector< std::pair<unsigned int, double> > >& pfoWriteDecorHandle,
							    SG::ReadHandle<std::map<Identifier,std::vector<const CaloCalibrationHit*> > >& CalibHitReadHandle)const
{
  
  StatusCode sc;
  for (auto thisFE : *pfoWriteDecorHandle){
    // retrieve calo cluster, first as the iparticle we retrieve then cast to calocluster ptr
    const xAOD::IParticle* FE_Iparticle=thisFE->otherObjects().at(0);
    const xAOD::CaloCluster* thisCaloCluster = dynamic_cast<const xAOD::CaloCluster*>(FE_Iparticle);
    if (not thisCaloCluster){
       ATH_MSG_ERROR("Dynamic cast failed in PFlowCalibPFODecoratorAlgorithm::LinkCalibHitPFO");
       return StatusCode::FAILURE;
    }
    std::vector<std::pair<unsigned int, double > > newUniqueIDTruthPairs;
    sc = m_truthAttributerTool->calculateTruthEnergies(*thisCaloCluster, m_numTruthParticles, *CalibHitReadHandle, newUniqueIDTruthPairs);
    if (sc == StatusCode::FAILURE) return sc;
    
    for (const auto& thisPair : newUniqueIDTruthPairs) ATH_MSG_DEBUG("Cluster Final loop: Particle with uniqueID " << thisPair.first << " has truth energy of " <<  thisPair.second << " for cluster with e, eta " << thisCaloCluster->e() << " and " << thisCaloCluster->eta());
    
    pfoWriteDecorHandle(*thisFE) = std::move(newUniqueIDTruthPairs);
  }
  return StatusCode::SUCCESS;
}

StatusCode PFlowCalibPFODecoratorAlgorithm::execute(const EventContext& ctx) const{

  SG::ReadHandle<std::map<Identifier,std::vector<const CaloCalibrationHit*> > > mapIdentifierToCalibHitsReadHandle(m_mapIdentifierToCalibHitsReadHandleKey, ctx);
  if(!mapIdentifierToCalibHitsReadHandle.isValid()){
    ATH_MSG_WARNING("Could not retrieve map between Identifier and calibraiton hits from Storegate");
    return StatusCode::FAILURE;
  }
  
  // pfo linker alg
  SG::WriteDecorHandle<xAOD::FlowElementContainer, std::vector< std::pair<unsigned int, double> > > pfoWriteDecorHandleNLeadingTruthParticles(m_pfoWriteDecorHandleKeyNLeadingTruthParticles, ctx);

  ATH_CHECK(this->LinkCalibHitPFO(
				      pfoWriteDecorHandleNLeadingTruthParticles,
				      mapIdentifierToCalibHitsReadHandle)); // end of check block 
  
  return StatusCode::SUCCESS;
}

StatusCode PFlowCalibPFODecoratorAlgorithm::finalize(){
  return StatusCode::SUCCESS;
}


