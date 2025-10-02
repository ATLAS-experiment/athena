/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexTOBSuperCellDecorator  -  description
//                              -------------------
//      This algorithm decorates the eFEX TOBs with the SuperCells Energies - suitable info for ML
//      
//     begin                : 26 09 2023
//     email                : panagiotis.bellos@cern.ch
//***************************************************************************/
#include "eFexTOBSuperCellDecorator.h"
#include "L1CaloFEXSim/eFEXegTOB.h"

namespace LVL1 {

  eFexTOBSuperCellDecorator::eFexTOBSuperCellDecorator(const std::string& name, ISvcLocator* svc) : AthAlgorithm(name, svc){}

  StatusCode eFexTOBSuperCellDecorator::initialize() {
    ATH_MSG_INFO( "L1CaloFEXTools/eFexTOBSuperCellDecorator::initialize()");

    // initialise read and decorator write handles
    ATH_CHECK( m_eFEXegEDMContainerKey.initialize() );
    ATH_CHECK( m_eFEXtauEDMContainerKey.initialize() );

    ATH_CHECK( m_SCEtVec_ele.initialize() );
    ATH_CHECK( m_SCEtVec_tau.initialize() );
   
    // Retrieve the TOB ET sum tool
    ATH_CHECK( m_eFEXTOBEtTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  StatusCode eFexTOBSuperCellDecorator::execute() {
    
    // read the TOB containers
    SG::ReadHandle<xAOD::eFexEMRoIContainer> eFEXegEDMContainerObj{m_eFEXegEDMContainerKey};
    if (!eFEXegEDMContainerObj.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve EDM collection: "<<m_eFEXegEDMContainerKey);
      return StatusCode::SUCCESS; 
    }
    const xAOD::eFexEMRoIContainer* emEDMConstPtr = eFEXegEDMContainerObj.cptr();

    SG::ReadHandle<xAOD::eFexTauRoIContainer> eFEXtauEDMContainerObj{m_eFEXtauEDMContainerKey};
    if (!eFEXtauEDMContainerObj.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve tau EDM collection: "<<m_eFEXtauEDMContainerKey);
      return StatusCode::SUCCESS; 
    }
    const xAOD::eFexTauRoIContainer* tauEDMConstPtr = eFEXtauEDMContainerObj.cptr();

    //Setup EM Decorator Handlers
    SG::WriteDecorHandle<xAOD::eFexEMRoIContainer, std::vector<unsigned int>>   SCEt_e  ( m_SCEtVec_ele);
	
    SG::WriteDecorHandle<xAOD::eFexTauRoIContainer, std::vector<unsigned int>>   SCEt_t  ( m_SCEtVec_tau);
    
    std::vector<unsigned int > ClusterCellETs;  //
    
    ////looping over EM TOB to decorate them
    for ( const xAOD::eFexEMRoI* emRoI : *emEDMConstPtr ){
              
      float eta = emRoI->eta();
      float phi = emRoI->phi();

      ATH_CHECK( m_eFEXTOBEtTool->getTOBCellEnergies(eta, phi, ClusterCellETs));

      SCEt_e (*emRoI) = ClusterCellETs;
      ClusterCellETs.clear();
    }

    //looping over Tau TOB to decorate them
    for ( const xAOD::eFexTauRoI* tauRoI : *tauEDMConstPtr ){
              
      float eta = tauRoI->eta();
      float phi = tauRoI->phi();
     
      ATH_CHECK( m_eFEXTOBEtTool->getTOBCellEnergies(eta, phi, ClusterCellETs));
	  
      SCEt_t (*tauRoI) = ClusterCellETs;
      ClusterCellETs.clear();
    }
	
    // Return gracefully
    return StatusCode::SUCCESS; 
  }
}
