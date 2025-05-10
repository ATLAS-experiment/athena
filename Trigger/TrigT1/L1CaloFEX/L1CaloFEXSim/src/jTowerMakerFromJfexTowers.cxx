/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jTowerMakerFromJfexTowers  -  description
//                     ---------------------------------------------------------
//      This algorithm is meant to create from the xAOD::jFexTowerContainer (EDM) a jTowerContainer (a SG object the jFEX simulation needs)
//      It needs to get both inputs available:
//          * Decoded jFex input data, known as L1_jFexDataTowers (It can be empty, since it is pre-scaled)
//          * Emulated jFex input data, known as L1_jFexEmulatedTowers
//      
//     begin                : 01 06 2023
//     email                : sergi.rodriguez@cern.ch
//     
//***************************************************************************/

#include "xAODTrigL1Calo/jFexTowerContainer.h"

#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerBuilder.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "./jTowerMakerFromJfexTowers.h"
#include "L1CaloFEXSim/jFEXCompression.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

namespace LVL1 {
    
jTowerMakerFromJfexTowers::jTowerMakerFromJfexTowers(const std::string& name, ISvcLocator* pSvcLocator)
    :  AthReentrantAlgorithm(name, pSvcLocator)
{}


StatusCode jTowerMakerFromJfexTowers::initialize()
{
    ATH_MSG_DEBUG("isMC: " << m_isMC );
    ATH_MSG_DEBUG("UseEmulated: " << m_UseEmulated );
    ATH_CHECK( m_DataTowerKey.initialize(!m_isMC) );
    ATH_CHECK( m_EmulTowerKey.initialize(m_UseEmulated) );
    ATH_CHECK( m_jSuperCellTowerMapperTool.retrieve() );
    ATH_CHECK( m_jTowerBuilderTool.retrieve() );
    ATH_CHECK( m_jTowerContainerSGKey.initialize() );

    return StatusCode::SUCCESS;

}


StatusCode jTowerMakerFromJfexTowers::execute(const EventContext& ctx) const
{
    ATH_MSG_DEBUG("Executing " << name() << ", processing event number " );
    
    //Reading the decoded Data jTower container
    SG::ReadHandle<xAOD::jFexTowerContainer> jDataTowerContainer;
    bool jDataTowerFilled = false;
    if(!m_isMC){
      jDataTowerContainer = SG::ReadHandle<xAOD::jFexTowerContainer>(m_DataTowerKey, ctx);
        if(!jDataTowerContainer.isValid()) {
            ATH_MSG_ERROR("Could not retrieve collection " << jDataTowerContainer.key() );
            return StatusCode::FAILURE;
        }      
        jDataTowerFilled = !jDataTowerContainer->empty();
    }
    
    //Reading the Emulated jTower container
    SG::ReadHandle<xAOD::jFexTowerContainer> jEmulatedTowerContainer;
    
    if(m_UseEmulated){
      jEmulatedTowerContainer = SG::ReadHandle<xAOD::jFexTowerContainer>(m_EmulTowerKey, ctx);
        if(!jEmulatedTowerContainer.isValid()) {
            ATH_MSG_ERROR("Could not retrieve collection " << jEmulatedTowerContainer.key() );
            return StatusCode::FAILURE;
        }           
    }


    // STEP 0 - Make a fresh local jTowerContainer
    std::unique_ptr<jTowerContainer> local_jTowerContainerRaw = std::make_unique<jTowerContainer>();

    // STEP 1 - Make some jTowers and fill the local container (This is the one the simulation reads)
    m_jTowerBuilderTool->init(local_jTowerContainerRaw);
    
    // STEP 2 - Mapping jFexTowers with decoded Energies
    if( jDataTowerFilled || m_UseEmulated ) {
        
        SG::ReadHandle<xAOD::jFexTowerContainer> * data_jTowerContainer;
        
        if(m_UseEmulated && !jDataTowerFilled){
            data_jTowerContainer = &jEmulatedTowerContainer;
        }
        else{
            data_jTowerContainer = &jDataTowerContainer;
        }
        
        ATH_MSG_DEBUG("Collection used to build the jTower for simulation: " << (*data_jTowerContainer).key() << " with size: "<<(*data_jTowerContainer)->size());

        for(const xAOD::jFexTower* my_jTower : *(*data_jTowerContainer) ) {

            unsigned int TTID = my_jTower->jFEXtowerID();

            LVL1::jTower *targetTower;
            if( (targetTower = local_jTowerContainerRaw->findTower(TTID)) ) {
                
                targetTower->setiEta( my_jTower->globalEta() );
                targetTower->setiPhi( my_jTower->globalPhi() );
                targetTower->setCentreEta( my_jTower->eta() );
                targetTower->setCentrePhi( my_jTower->phi() );
                targetTower->setOnlineID( my_jTower->OnlineID() );
                
                uint8_t source = my_jTower->Calosource(); // Values: EMB:0 Tile:1 EMEC:2 HEC:3 FCAL1:4 FCAL2:5 FCAL3:6

                // Set up for Tile energies
                if(source == 1) {
                    // Always in the hadronic layer = 1
                    targetTower->set_Et(1,(my_jTower->et_count()).at(0)*500);// conversion factor of 500
                    
                    //setting the saturaion 
                    if( (my_jTower->isjTowerSat()).at(0) ) targetTower->setHADSat();
                }
                else { //Now we do LATOME

                    int layer = 1; // Hadronic layer
                    if(source == 0 or source == 2 or source == 4 ) layer = 0; // Electromagnetic layer

                    // LATOME encoded Et needs to be converted to MeV
                    targetTower->set_Et(layer, jFEXCompression::Expand( (my_jTower->et_count()).at(0) )  );
                    
                    //Setting the saturation
                    if( (my_jTower->isjTowerSat()).at(0) ){
                        if( layer == 1 ){
                            targetTower->setHADSat();
                        }
                        else{
                            targetTower->setEMSat();
                        }
                    }
                }
            }
            else {
                ATH_MSG_ERROR("Tower ID is officially unknown - it will be ignored. (Needs investigation).  Please report this!" );
                continue;
            }
        }
    }
    else{
        ATH_MSG_ERROR("Falling into the legacy path. This should never happen!. Use EmulatedTowers or DataTowers - Contact L1Calo offline experts");
        return StatusCode::FAILURE;
        
        // Prefer to keep this for debugging purposes - Experts only
        // It does reconstruct the jTowers using eta/phi coordinates and internal parameters from the CaloCell and TriggerTower containers.
        // EmulatedTowers and DataTowers are xAOD objects, so they can saved into an AOD file.
        ATH_CHECK(m_jSuperCellTowerMapperTool->AssignSuperCellsToTowers(local_jTowerContainerRaw));
        ATH_CHECK(m_jSuperCellTowerMapperTool->AssignTriggerTowerMapper(local_jTowerContainerRaw));        
    }
    
    // STEP 3 -  Once coordinates energies and all are filled, then we fill pileup and noise values
    ATH_CHECK(m_jTowerBuilderTool->AssignPileupAndNoiseValues(local_jTowerContainerRaw));
    
    // STEP 4 - Write the completed jTowerContainer into StoreGate (move the local copy in memory)
    SG::WriteHandle<LVL1::jTowerContainer> jTowerContainerSG(m_jTowerContainerSGKey, ctx);
    ATH_CHECK(jTowerContainerSG.record(std::move( local_jTowerContainerRaw ) ) );

    // STEP 5 - Close and clean the event
    m_jTowerBuilderTool->reset();

    return StatusCode::SUCCESS;
}


} // end of LVL1 namespace
