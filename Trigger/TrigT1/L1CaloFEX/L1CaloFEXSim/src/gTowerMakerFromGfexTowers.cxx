/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gTowerMakerFromGfexTowers  -  description
//                     ---------------------------------------------------------
//      This algorithm is meant to create from the xAOD::gFexTowerContainer (EDM) a gTowerContainer (a SG object the gFEX simulation needs)
//      It needs to get both inputs available:
//          * Decoded gFex input data, known as L1_gFexDataTowers
//          * Emulated gFex input data, known as L1_gFexEmulatedTowers (to be confirmed if this is the name in gFex)
//      
//     begin                : 14 07 2023
//     email                : cecilia.tosciri@cern.ch
//     
//***************************************************************************/

#include "xAODTrigL1Calo/gFexTowerContainer.h"

#include "L1CaloFEXSim/gTower.h"
#include "L1CaloFEXSim/gTowerBuilder.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/gTowerMakerFromGfexTowers.h"
#include "L1CaloFEXSim/gFEXCompression.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"


namespace LVL1 {
    
gTowerMakerFromGfexTowers::gTowerMakerFromGfexTowers(const std::string& name, ISvcLocator* pSvcLocator)
    :  AthAlgorithm(name, pSvcLocator)
{}


StatusCode gTowerMakerFromGfexTowers::initialize()
{
    ATH_CHECK( m_gDataTowerKey.initialize(!m_isMC) );
    // ATH_CHECK( m_gDataTower50Key.initialize(!m_isMC) );
    ATH_CHECK( m_gEmulTowerKey.initialize(m_UseEmulated) );
    ATH_CHECK( m_gSuperCellTowerMapperTool.retrieve() );
    ATH_CHECK( m_gTowerBuilderTool.retrieve() );
    ATH_CHECK( m_gTowerContainerSGKey.initialize() );

    return StatusCode::SUCCESS;

}


StatusCode gTowerMakerFromGfexTowers::execute() 
{
    ATH_MSG_DEBUG("Executing " << name() << ", input: " << m_gDataTowerKey.key() << ", output: " << m_gTowerContainerSGKey.key());
    
    const EventContext& ctx = Gaudi::Hive::currentContext();

    //Reading the decoded Data gTower container
    SG::ReadHandle<xAOD::gFexTowerContainer> gDataTowerContainer;
    bool gDataTowerFilled = false;
    if(!m_isMC){
        gDataTowerContainer = SG::ReadHandle<xAOD::gFexTowerContainer>(m_gDataTowerKey, ctx);
        if(!gDataTowerContainer.isValid()) {
            ATH_MSG_FATAL("Could not retrieve collection " << gDataTowerContainer.key() );
            return StatusCode::FAILURE;
        }      
        gDataTowerFilled = !gDataTowerContainer->empty();
    }
    
    //Reading the Emulated gTower container
    SG::ReadHandle<xAOD::gFexTowerContainer> gEmulatedTowerContainer;
    
    if(m_UseEmulated){
        gEmulatedTowerContainer = SG::ReadHandle<xAOD::gFexTowerContainer>(m_gEmulTowerKey, ctx);
        if(!gEmulatedTowerContainer.isValid()) {
            ATH_MSG_FATAL("Could not retrieve collection " << gEmulatedTowerContainer.key() );
            return StatusCode::FAILURE;
        }           
    }
     



    // STEP 0 - Make a fresh local gTowerContainer
    std::unique_ptr<gTowerContainer> local_gTowerContainerRaw = std::make_unique<gTowerContainer>();

    // STEP 1 - Make some gTowers and fill the local container (This is the one the simulation reads)
    m_gTowerBuilderTool->init(local_gTowerContainerRaw);
    
    // STEP 2 - Mapping gFexTowers with decoded Energies
    if( !m_isMC && (gDataTowerFilled || m_UseEmulated) ) {
        
        SG::ReadHandle<xAOD::gFexTowerContainer> * data_gTowerContainer = &gDataTowerContainer;
        
        if(m_UseEmulated){
            // If we allow emulated input data, then we need to check if decoded data is available, otherwise use emulated
            data_gTowerContainer = gDataTowerContainer->empty() ? &gEmulatedTowerContainer : &gDataTowerContainer;
        }
        
        ATH_MSG_DEBUG("Collection used to build the gTower for simulation: " << (*data_gTowerContainer).key() << " with size: "<<(*data_gTowerContainer)->size() << ". Expected towers 1152");

        for(const xAOD::gFexTower* my_gTower : *(*data_gTowerContainer) ) {

            unsigned int TTID = my_gTower->gFEXtowerID(); //This is the simulation tower ID

            // Find the simulation ID from this firmware ID
            int ID = local_gTowerContainerRaw->getIDfromFWID(TTID);
            if (ID == -1) {
                ATH_MSG_WARNING("Cannot find simulation ID from firmware ID - This gFexTower will be ignored. (Needs investigation).  Please report this!" );
                continue;
            }

            LVL1::gTower *targetTower;
            if( (targetTower = local_gTowerContainerRaw->findTower(ID)) ) {

                targetTower->setTotalEt(my_gTower->towerEt());
                targetTower->setIsSaturated(my_gTower->isSaturated());
                
            }
            else {
                ATH_MSG_WARNING("Tower ID is officially unknown - it will be ignored. (Needs investigation).  Please report this!" );
                continue;
            }
        }
    }
    else{
        ATH_MSG_DEBUG("Falling into the legacy path");
        ATH_CHECK(m_gSuperCellTowerMapperTool->AssignSuperCellsToTowers(local_gTowerContainerRaw));
        ATH_CHECK(m_gSuperCellTowerMapperTool->AssignTriggerTowerMapper(local_gTowerContainerRaw));        
    }

    // STEP 3 - Write the completed gTowerContainer into StoreGate (move the local copy in memory)
    SG::WriteHandle<LVL1::gTowerContainer> gTowerContainerSG(m_gTowerContainerSGKey, ctx);
    ATH_CHECK(gTowerContainerSG.record(std::move( local_gTowerContainerRaw ) ) );

    // STEP 4 - Close and clean the event
    m_gTowerBuilderTool->reset();

    return StatusCode::SUCCESS;
}


} // end of LVL1 namespace
