/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZdcMCTruthAlg_H
#define ZdcMCTruthAlg_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "ZdcIdentifier/ZdcID.h"

#include <string>

class ZdcMCTruthAlg : public AthAlgorithm {


public:

    ZdcMCTruthAlg(std::string name, ISvcLocator* pSvcLocator);

    virtual ~ZdcMCTruthAlg();

    StatusCode initialize();
    StatusCode execute();

    StatusCode finalize();
    
    

private:
    const ZdcID*  m_zdcID;

    Gaudi::Property<std::string> m_auxSuffix{this, "AuxSuffix", "", "ZdcModuleAuxContainer decoration suffix"};

    SG::ReadHandleKey<CaloCalibrationHitContainer> m_CaloCalibrationHitContainerKey{this, "ZDC_CaloCalibrationHitContainerName", "ZDC_CalibrationHits"};
    std::string m_zdcModuleContainerName;
    std::string m_zdcSumContainerName;
      
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleTruthTotalEnergy{this, "ZdcModuleTruthTotalEnergy"    , "", "ZDC module Total Truth Energy"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleTruthInvisEnergy{this, "ZdcModuleTruthInvisibleEnergy", "", "ZDC module Invisible Truth Energy"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleTruthEMEnergy   {this, "ZdcModuleTruthEMEnergy"       , "", "ZDC module EM Truth Energy"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleTruthNonEMEnergy{this, "ZdcModuleTruthNonEMEnergy"    , "", "ZDC module Non EM Truth Energy"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleTruthEscEnergy  {this, "ZdcModuleTruthEscapedEnergy"  , "", "ZDC module Escaped Truth Energy"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_rpdTileXpositionRelative {this, "xposRel"                      , "", "RPD module tile x position relative to the center of the module"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_rpdTileYpositionRelative {this, "yposRel"                      , "", "RPD module tile y position relative to the center of the module"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_rpdTileRowNumber         {this, "row"                          , "", "RPD channel row number"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_rpdTileColumnNumber      {this, "col"                          , "", "RPD channel column number"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumTruthTotalEnergy   {this, "ZdcSumTruthTotalEnergy"       , "", "ZDC Sum Total Truth Energy"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumTruthInvisEnergy   {this, "ZdcSumTruthInvisibleEnergy"   , "", "ZDC Sum Invisible Truth Energy"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumTruthEMEnergy      {this, "ZdcSumTruthEMEnergy"          , "", "ZDC Sum EM Truth Energy"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumTruthNonEMEnergy   {this, "ZdcSumTruthNonEMEnergy"       , "", "ZDC Sum Non EM Truth Energy"};
    SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumTruthEscEnergy     {this, "ZdcSumTruthEscapedEnergy"     , "", "ZDC Sum Escaped Truth Energy"};
};

#endif

