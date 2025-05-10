/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JTOWERMAKERFROMSUPERCELLS_H
#define JTOWERMAKERFROMSUPERCELLS_H

// STL
#include <string>


// Athena/Gaudi
#include "StoreGate/WriteHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/jTowerBuilder.h"
#include "L1CaloFEXSim/jSuperCellTowerMapper.h"

#include "xAODTrigL1Calo/TriggerTowerContainer.h"


namespace LVL1 {

class jTowerMakerFromSuperCells : public AthAlgorithm
{
    public:

        jTowerMakerFromSuperCells(const std::string& name, ISvcLocator* svc);

        virtual StatusCode initialize();
        virtual StatusCode execute();

    private:

        SG::WriteHandleKey<LVL1::jTowerContainer> m_jTowerContainerSGKey {this, "MyJTowers", "jTowerContainer", "MyJTowers"};

        ToolHandle<IjTowerBuilder> m_jTowerBuilderTool {this, "jTowerBuilderTool", "LVL1::jTowerBuilder", "Tool that builds jTowers for simulation"};
        ToolHandle<IjSuperCellTowerMapper> m_jSuperCellTowerMapperTool {this, "jSuperCellTowerMapperTool", "LVL1::jSuperCellTowerMapper", "Tool that maps supercells to jTowers"};




};

} // end of LVL1 namespace
#endif
