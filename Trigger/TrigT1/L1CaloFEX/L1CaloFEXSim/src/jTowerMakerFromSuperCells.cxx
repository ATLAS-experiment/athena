/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "xAODTrigL1Calo/TriggerTowerContainer.h"

#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerBuilder.h"

#include "L1CaloFEXSim/jSuperCellTowerMapper.h"
#include "L1CaloFEXSim/jTowerMakerFromSuperCells.h"

#include <fstream>

namespace LVL1 {

jTowerMakerFromSuperCells::jTowerMakerFromSuperCells(const std::string& name, ISvcLocator* svc):  AthAlgorithm(name, svc) {}

StatusCode jTowerMakerFromSuperCells::initialize()
{

    ATH_CHECK( m_jTowerBuilderTool.retrieve() );
    ATH_CHECK( m_jSuperCellTowerMapperTool.retrieve() );
    ATH_CHECK( m_jTowerContainerSGKey.initialize() );

    return StatusCode::SUCCESS;
}


StatusCode jTowerMakerFromSuperCells::execute() {


    // STEP 0 - Make a fresh local jTowerContainer
    std::unique_ptr<jTowerContainer> local_jTowerContainerRaw = std::make_unique<jTowerContainer>();

    // STEP 1 - Make some jTowers and fill the local container
    m_jTowerBuilderTool->init(local_jTowerContainerRaw);


    // STEP 2 - Do the supercell-tower mapping - put this information into the jTowerContainer
    ATH_CHECK(m_jSuperCellTowerMapperTool->AssignSuperCellsToTowers(local_jTowerContainerRaw));
    ATH_CHECK(m_jSuperCellTowerMapperTool->AssignTriggerTowerMapper(local_jTowerContainerRaw));

    //STEP 2.5 - Set up a file mapping if necessary (should only need to be done if the mapping changes, which should never happen unless major changes to the simulation are required)
    // Only used for simulation experts. Contact one of us first
    // With just one event should be enough to generate the file
    if(false) {

        ATH_MSG_INFO("Writting the mapping for jFEX");
        std::ofstream sc_tower_map;
        sc_tower_map.open("./new_jfex_SCID.txt");
        sc_tower_map << "# Simulation ID, 12 Scells (EMB or EMEC or FCAL1 layer) + 1 Scell (HEC or FCAL2/3 layer)" << "\n";

        for(const LVL1::jTower* jtower : *local_jTowerContainerRaw) {
            sc_tower_map << jtower->OfflineID() << " ";

            std::vector<Identifier> vEM = jtower->getEMSCIDs();
            for(const auto& SCellID : vEM) {
                sc_tower_map << SCellID << " ";
            }
            for(unsigned int i=0; i<(12-vEM.size()); i++) {
                sc_tower_map << "0xffffffffffffffff" << " ";
            }

            std::vector<Identifier> vHAD = jtower->getHADSCIDs();
            for(const auto& SCellID : vHAD) {
                sc_tower_map << SCellID << " ";
            }
            if(vHAD.size()==0) {
                sc_tower_map << "0xffffffffffffffff" << " ";
            }
            sc_tower_map << "\n";
        }

        sc_tower_map.close();
    }

    // STEP 3 - Write the completed jTowerContainer into StoreGate (move the local copy in memory)
    SG::WriteHandle<LVL1::jTowerContainer> jTowerContainerSG(m_jTowerContainerSGKey /*, ctx*/);
    ATH_CHECK(jTowerContainerSG.record(std::move( local_jTowerContainerRaw ) ) );

    // STEP 4 - Close and clean the event
    m_jSuperCellTowerMapperTool->reset();
    m_jTowerBuilderTool->reset();

    return StatusCode::SUCCESS;
}

} // end of LVL1 namespace
