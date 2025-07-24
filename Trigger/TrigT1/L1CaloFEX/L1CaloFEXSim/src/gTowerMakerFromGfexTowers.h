/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef gTowerMakerFromGfexTowers_H
#define gTowerMakerFromGfexTowers_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "gTowerBuilder.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "xAODTrigL1Calo/gFexTowerContainer.h"
#include "gSuperCellTowerMapper.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

class CaloIdManager;

namespace LVL1 {

class gTowerMakerFromGfexTowers : public AthAlgorithm
{
    public:

        gTowerMakerFromGfexTowers(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~gTowerMakerFromGfexTowers() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute() override;

    private:
        
        // Decoded input data
        SG::ReadHandleKey<xAOD::gFexTowerContainer> m_gDataTowerKey {this, "InputDataTowers", "L1_gFexDataTowers", "gfexTowers with 200 MeV resolution (default) (use L1_gFexEmulatedTowers for built from SC, or L1_gFexDataTowers for efex readout"};

        // SG object for the gFEX simulation input
        SG::WriteHandleKey<LVL1::gTowerContainer> m_gTowerContainerSGKey {this, "MyGTowers", "gTowerContainer", "MyGTowers"};
        
        ToolHandle<IgTowerBuilder> m_gTowerBuilderTool {this, "gTowerBuilderTool", "LVL1::gTowerBuilder", "Tool that builds gTowers for simulation"};
        ToolHandle<IgSuperCellTowerMapper> m_gSuperCellTowerMapperTool {this, "gSuperCellTowerMapperTool", "LVL1::gSuperCellTowerMapper", "Tool that maps supercells to gTowers"};
};

} // end of LVL1 namespace
#endif
