/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexTOBDecorator  -  description:
//       This algorithm decorates the eFEX TOBs with the recalculated isolation variables
//                              -------------------
//     begin                : 13 02 2023
//     email                : paul.daniel.thompson@cern.ch
//***************************************************************************/

#ifndef EFEXTOBDECORATORTOOL_H
#define EFEXTOBDECORATORTOOL_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"


#include "L1CaloFEXSim/eFEXTOBEtTool.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"

namespace LVL1 {
    
  class eFexTOBDecorator : public AthAlgorithm{
  public:
    eFexTOBDecorator(const std::string& name, ISvcLocator* svc);

    // Function initialising the algorithm
    virtual StatusCode initialize();
    // Function executing the algorithm
    virtual StatusCode execute();
    	
  private:
    // Readhandles for eFEX TOBs
    SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFEXegEDMContainerKey{this,"eFexEMRoIContainer","L1_eEMRoI","SG key of the input eFex RoI container"};
    SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFEXtauEDMContainerKey{this,"eFexTauRoIContainer","L1_eTauRoI","SG key of the input eFex Tau RoI container"};
    
    // WriteDecor handles for the EM RoI decorations
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_RetaCoreDec { this, "RetaCoreDecDecorKey"  , m_eFEXegEDMContainerKey,"RetaCoreDec"  , "Recalculated EM RetaCore" };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_RetaEnvDec  { this, "RetaEnvDecDecorKey"   , m_eFEXegEDMContainerKey,"RetaEnvDec"   , "Recalculated EM RetaEnv"  };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_RhadEMDec   { this, "RetaEMDecDecorKey"    , m_eFEXegEDMContainerKey,"RhadEMDec"    , "Recalculated EM RetaEM"   };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_RhadHadDec  { this, "RhadHadDecDecorKey"   , m_eFEXegEDMContainerKey,"RhadHadDec"   , "Recalculated EM RhadHad"  };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_WstotDenDec { this, "WstotDenDecDecorKey"  , m_eFEXegEDMContainerKey,"WstotDenDec"  , "Recalculated EM WstotDen" };
    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_WstotNumDec { this, "WstotNumDecDecorKey"  , m_eFEXegEDMContainerKey,"WstotNumDec"  , "Recalculated EM WstotNum" };

    SG::WriteDecorHandleKey<xAOD::eFexEMRoIContainer> m_ClusterSCellEtSumsDec { this, "ClusterSCellEtSumsDecorKey"  , m_eFEXegEDMContainerKey,"ClusterSCellEtSums", "name of the decoration key for SCell Ets of the cluster"};

    // WriteDecor handles for the Tau RoI decorations
    SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer> m_RCoreDec   { this, "RCoreDecorKey"   ,m_eFEXtauEDMContainerKey,"RCoreDec"   , "Recalculated Tau RCore" };
    SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer> m_REnvDec    { this, "REnvDecorKey"    , m_eFEXtauEDMContainerKey,"REnvDec"    , "Recalculated Tau REnv" };
    SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer> m_REMCoreDec { this, "REMCoreDecorKey" , m_eFEXtauEDMContainerKey,"REMCoreDec" , "Recalculated Tau REMCore" };
    SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer> m_REMHadDec  { this, "REMHadDecorKey"  , m_eFEXtauEDMContainerKey,"REMHadDec"  , "Recalculated Tau REMHad" };

    ToolHandle<eFEXTOBEtTool> m_eFEXTOBEtTool {this, "eFEXTOBEtTool", "LVL1::eFEXTOBEtTool", "Tool for reconstructing TOB ET sums"};
  };
}
#endif
