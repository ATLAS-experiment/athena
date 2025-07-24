/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexTOBSuperCellDecorator  -  description:
//       This algorithm decorates the eFEX TOBs with the SuperCells Energies - suitable info for ML
//                              -------------------
//     begin                : 26 09 2023
//     email                : panagiotis.bellos@cern.ch
//***************************************************************************/

#ifndef EFEXTOBMLDECORATORTOOL_H
#define EFEXTOBMLDECORATORTOOL_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"


#include "L1CaloFEXSim/eFEXTOBEtTool.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"

namespace LVL1 {
    
  class eFexTOBSuperCellDecorator : public AthAlgorithm{
  public:
    eFexTOBSuperCellDecorator(const std::string& name, ISvcLocator* svc);

    // Function initialising the algorithm
    virtual StatusCode initialize();
    // Function executing the algorithm
    virtual StatusCode execute();
    	
  private:
    // Readhandles for eFEX TOBs
    SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFEXegEDMContainerKey{this,"eFexEMRoIContainer","L1_eEMRoI","SG key of the input eFex RoI container"};
    SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFEXtauEDMContainerKey{this,"eFexTauRoIContainer","L1_eTauRoI","SG key of the input eFex Tau RoI container"};

    // WriteDecor handles
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer>  m_SCEtVec_ele { this, "EMDecorKey"  , m_eFEXegEDMContainerKey, "SuperCells", "name of the decoration key for eFexEMRoI"};
    SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer>  m_SCEtVec_tau { this, "TauDecorKey"  ,  m_eFEXtauEDMContainerKey, "SuperCells", "name of the decoration key for eFexTauRoI"};

  
    ToolHandle<eFEXTOBEtTool> m_eFEXTOBEtTool {this, "eFEXTOBEtTool", "LVL1::eFEXTOBEtTool", "Tool for reconstructing TOB ET s"};
  };
}
#endif
