/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef jTowerMakerFromJfexTowers_H
#define jTowerMakerFromJfexTowers_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "xAODTrigL1Calo/jFexTowerContainer.h"
#include "jSuperCellTowerMapper.h"
#include "jTowerBuilder.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

class CaloIdManager;

namespace LVL1 {

class jTowerMakerFromJfexTowers : public AthReentrantAlgorithm
{
    public:

        jTowerMakerFromJfexTowers(const std::string& name, ISvcLocator* pSvcLocator);
        ~jTowerMakerFromJfexTowers() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;


    private:
        
        // Decoded input data
        SG::ReadHandleKey<xAOD::jFexTowerContainer> m_DataTowerKey {this, "InputDataTowers", "L1_jFexDataTowers", "jfexTowers (use L1_jFexEmulatedTowers for built from SC, or L1_jFexDataTowers for efex readout"};
        
        //Emulated input data
        // the use-case for this second input tower collection is in monitoring when running in prescaled readout mode
        // we want to use the input data readout when we have it, but otherwise fallback to the calo readout
        SG::ReadHandleKey<xAOD::jFexTowerContainer> m_EmulTowerKey {this, "InputEmulatedTowers", "L1_jFexEmulatedTowers", "If specified, will fallback to this collection of towers if the first collection is incomplete/empty"};
        
        //Gaudi properties
        Gaudi::Property<bool> m_UseEmulated {this, "UseEmulated", true, "It switches off the CaloCell -> Jtower path. It uses instead L1_jFexDataTowers and L1_jFexEmulatedTowers "};
        Gaudi::Property<bool> m_isMC {this, "IsMC", false, "Is used to know when we run on data. So L1_jFexDataTowers can be present"};

        
        // SG object for the jFEX simulation input
        SG::WriteHandleKey<LVL1::jTowerContainer> m_jTowerContainerSGKey {this, "MyJTowers", "jTowerContainer", "MyJTowers"};
        
        ToolHandle<IjTowerBuilder> m_jTowerBuilderTool {this, "jTowerBuilderTool", "LVL1::jTowerBuilder", "Tool that builds jTowers for simulation"};
        ToolHandle<IjSuperCellTowerMapper> m_jSuperCellTowerMapperTool {this, "jSuperCellTowerMapperTool", "LVL1::jSuperCellTowerMapper", "Tool that maps supercells to jTowers"};
};

} // end of LVL1 namespace
#endif
