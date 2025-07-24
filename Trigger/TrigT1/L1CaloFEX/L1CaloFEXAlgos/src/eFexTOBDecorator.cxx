/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexTOBDecorator  -  description
//                              -------------------
//      This algorithm decorates the eFEX TOBs with jet descriminant variables and, for the EM TOBs, with the cluster layer supercell Et sums
//      recalculated from input data using the eFEXTOBEtTool
//      All values are in 25 MeV counts
//      
//     begin                : 03 02 2022
//     email                : paul.daniel.thompson@cern.ch
//***************************************************************************/
#include "eFexTOBDecorator.h"
#include "L1CaloFEXSim/eFEXegTOB.h"

namespace LVL1 {

  eFexTOBDecorator::eFexTOBDecorator(const std::string& name, ISvcLocator* svc) : AthAlgorithm(name, svc){}

  StatusCode eFexTOBDecorator::initialize() {
    ATH_MSG_INFO( "L1CaloFEXTools/eFexTOBDecorator::initialize()");

    // initialise read and decorator write handles
    ATH_CHECK( m_eFEXegEDMContainerKey.initialize() );
    ATH_CHECK( m_eFEXtauEDMContainerKey.initialize() );

    ATH_CHECK( m_RetaCoreDec.initialize() );
    ATH_CHECK( m_RetaEnvDec.initialize() );
    ATH_CHECK( m_RhadEMDec.initialize() );
    ATH_CHECK( m_RhadHadDec.initialize() );
    ATH_CHECK( m_WstotDenDec.initialize() );    
    ATH_CHECK( m_WstotNumDec.initialize() );    

    ATH_CHECK( m_ClusterSCellEtSumsDec.initialize() );

    ATH_CHECK( m_RCoreDec.initialize() );      
    ATH_CHECK( m_REnvDec.initialize() );       
    ATH_CHECK( m_REMCoreDec.initialize() );    
    ATH_CHECK( m_REMHadDec.initialize() );     

    // Retrieve the TOB ET sum tool
    ATH_CHECK( m_eFEXTOBEtTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  StatusCode eFexTOBDecorator::execute() {
    
    // read the TOB containers
    SG::ReadHandle<xAOD::eFexEMRoIContainer> eFEXegEDMContainerObj{m_eFEXegEDMContainerKey};
    if (!eFEXegEDMContainerObj.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve EDM collection");
      return StatusCode::SUCCESS; 
    }
    const xAOD::eFexEMRoIContainer* emEDMConstPtr = eFEXegEDMContainerObj.cptr();

    SG::ReadHandle<xAOD::eFexTauRoIContainer> eFEXtauEDMContainerObj{m_eFEXtauEDMContainerKey};
    if (!eFEXtauEDMContainerObj.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve tau EDM collection");
      return StatusCode::SUCCESS; 
    }
    const xAOD::eFexTauRoIContainer* tauEDMConstPtr = eFEXtauEDMContainerObj.cptr();
    
    //Setup EM Decorator Handlers
    SG::WriteDecorHandle<xAOD::eFexEMRoIContainer, unsigned int >   RetaCoreDec  (m_RetaCoreDec );
    SG::WriteDecorHandle<xAOD::eFexEMRoIContainer, unsigned int >   RetaEnvDec   (m_RetaEnvDec  );
    SG::WriteDecorHandle<xAOD::eFexEMRoIContainer, unsigned int >   RhadEMDec    (m_RhadEMDec   );
    SG::WriteDecorHandle<xAOD::eFexEMRoIContainer, unsigned int >   RhadHadDec   (m_RhadHadDec  );
    SG::WriteDecorHandle<xAOD::eFexEMRoIContainer, unsigned int >   WstotDenDec  (m_WstotDenDec );
    SG::WriteDecorHandle<xAOD::eFexEMRoIContainer, unsigned int >   WstotNumDec  (m_WstotNumDec );

    SG::WriteDecorHandle<xAOD::eFexEMRoIContainer, std::vector<unsigned int > >   ClusterSCellEtSumsDec  (m_ClusterSCellEtSumsDec );

    //looping over EM TOB to decorate them
    for ( const xAOD::eFexEMRoI* emRoI : *emEDMConstPtr ){
              
      float eta = emRoI->eta();
      float phi = emRoI->phi();
      int seed = emRoI->seed();
      int UnD = emRoI->UpNotDown();
      std::vector<unsigned int> ClusterCellETs;
      std::vector<unsigned int> RetaSums;
      std::vector<unsigned int> RhadSums;
      std::vector<unsigned int> WstotSums;

      ATH_CHECK( m_eFEXTOBEtTool->getegSums(eta, phi, seed, UnD, ClusterCellETs, RetaSums, RhadSums, WstotSums) );

      RetaCoreDec (*emRoI) = RetaSums[0];
      RetaEnvDec  (*emRoI) = RetaSums[1];
      RhadEMDec   (*emRoI) = RhadSums[0];
      RhadHadDec  (*emRoI) = RhadSums[1];
      WstotDenDec (*emRoI) = WstotSums[0];
      WstotNumDec (*emRoI) = WstotSums[1];

      ClusterSCellEtSumsDec (*emRoI) = std::move(ClusterCellETs);
    }

    //Setup Tau Decorator Handlers
    SG::WriteDecorHandle<xAOD::eFexTauRoIContainer, unsigned int >   RCoreDec   (m_RCoreDec   );
    SG::WriteDecorHandle<xAOD::eFexTauRoIContainer, unsigned int >   REnvDec    (m_REnvDec    );
    SG::WriteDecorHandle<xAOD::eFexTauRoIContainer, unsigned int >   REMCoreDec (m_REMCoreDec );
    SG::WriteDecorHandle<xAOD::eFexTauRoIContainer, unsigned int >   REMHadDec  (m_REMHadDec  );

    //looping over Tau TOB to decorate them
    for ( const xAOD::eFexTauRoI* tauRoI : *tauEDMConstPtr ){
              
      float eta = tauRoI->eta();
      float phi = tauRoI->phi();
      int seed = tauRoI->seed();
      int UnD = tauRoI->upNotDown();
      std::vector<unsigned int> RcoreSums;
      std::vector<unsigned int> RemSums;

      ATH_CHECK( m_eFEXTOBEtTool->gettauSums(eta, phi, seed, UnD, RcoreSums, RemSums) );

      RCoreDec   (*tauRoI) = RcoreSums[0];
      REnvDec    (*tauRoI) = RcoreSums[1];
      REMCoreDec (*tauRoI) = RemSums[0];
      REMHadDec  (*tauRoI) = RemSums[1];
    }
    
    // Return gracefully
    return StatusCode::SUCCESS;
  }

}
