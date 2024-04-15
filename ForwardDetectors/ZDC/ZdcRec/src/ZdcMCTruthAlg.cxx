/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcRec/ZdcMCTruthAlg.h"

#include "xAODForward/ZdcModule.h"
#include "ZdcConditions/ZdcLucrodMapRun3.h"
#include <AsgDataHandles/WriteDecorHandle.h>

ZdcMCTruthAlg::ZdcMCTruthAlg(std::string name, ISvcLocator* pSvcLocator)
   : AthAlgorithm(name, pSvcLocator),
     m_zdcID(nullptr)
{

}


ZdcMCTruthAlg::~ZdcMCTruthAlg()
{;}


////////////////   INITIALIZE   ///////////////////////
StatusCode ZdcMCTruthAlg::initialize()
{
   ATH_MSG_INFO("Initializing " << name());

  if (detStore()->retrieve( m_zdcID ).isFailure() ) {
    ATH_MSG_ERROR("execute: Could not retrieve ZdcID object from the detector store");
    return StatusCode::FAILURE;
  }else{
    ATH_MSG_DEBUG("execute: retrieved ZdcID");
  }

  //Initialize input hit container read handles
	ATH_CHECK( m_CaloCalibrationHitContainerKey.initialize() );

	//Initialize output ZdcModuleContainer read handles
  m_zdcModuleContainerName = "ZdcModules";
  m_zdcSumContainerName = "ZdcSums";

	//Per module AuxDecor handles
  m_zdcModuleTruthTotalEnergy = m_zdcModuleContainerName+".TruthTotalEnergy"+m_auxSuffix;
  ATH_CHECK( m_zdcModuleTruthTotalEnergy.initialize());
  m_zdcModuleTruthInvisEnergy   = m_zdcModuleContainerName+".TruthInvisibleEnergy"+m_auxSuffix; 
  ATH_CHECK( m_zdcModuleTruthInvisEnergy.initialize());
  m_zdcModuleTruthEMEnergy    = m_zdcModuleContainerName+".TruthEMEnergy"+m_auxSuffix; 
  ATH_CHECK( m_zdcModuleTruthEMEnergy.initialize());
  m_zdcModuleTruthNonEMEnergy = m_zdcModuleContainerName+".TruthNonEMEnergy"+m_auxSuffix; 
  ATH_CHECK( m_zdcModuleTruthNonEMEnergy.initialize());
  m_zdcModuleTruthEscEnergy = m_zdcModuleContainerName + ".TruthEscapedEnergy" + m_auxSuffix;
  ATH_CHECK( m_zdcModuleTruthEscEnergy.initialize());

  //RPD decorations required for processing
  m_rpdTileXpositionRelative = m_zdcModuleContainerName+".xposRel"+m_auxSuffix; 
  ATH_CHECK( m_rpdTileXpositionRelative.initialize());
  m_rpdTileYpositionRelative = m_zdcModuleContainerName+".yposRel"+m_auxSuffix; 
  ATH_CHECK( m_rpdTileYpositionRelative.initialize());
  m_rpdTileRowNumber = m_zdcModuleContainerName+".row"+m_auxSuffix; 
  ATH_CHECK( m_rpdTileRowNumber.initialize());
  m_rpdTileColumnNumber = m_zdcModuleContainerName+".col"+m_auxSuffix; 
  ATH_CHECK( m_rpdTileColumnNumber.initialize());

  //Sums container auxDecor handles
  m_zdcSumTruthTotalEnergy    = m_zdcSumContainerName+".TruthTotalEnergy"+m_auxSuffix; 
  ATH_CHECK( m_zdcSumTruthTotalEnergy.initialize());
  m_zdcSumTruthInvisEnergy      = m_zdcSumContainerName+".TruthInvisibleEnergy"+m_auxSuffix; 
  ATH_CHECK( m_zdcSumTruthInvisEnergy.initialize());
  m_zdcSumTruthEMEnergy       = m_zdcSumContainerName+".TruthEMEnergy"+m_auxSuffix; 
  ATH_CHECK( m_zdcSumTruthEMEnergy.initialize());
  m_zdcSumTruthNonEMEnergy    = m_zdcSumContainerName+".TruthNonEMEnergy"+m_auxSuffix; 
  ATH_CHECK( m_zdcSumTruthNonEMEnergy.initialize());
  m_zdcSumTruthEscEnergy      = m_zdcSumContainerName + ".TruthEscapedEnergy" + m_auxSuffix;
  ATH_CHECK( m_zdcSumTruthEscEnergy.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ZdcMCTruthAlg::execute()
{

  /******************************************
   * Get the CaloCalibrationHitContainer (input)
  ******************************************/
  SG::ReadHandle<CaloCalibrationHitContainer> calibHitContainer(m_CaloCalibrationHitContainerKey, getContext());
  if (!calibHitContainer.isValid()) {
    ATH_MSG_ERROR("Could not get Calibration hit container " << calibHitContainer.name() << " from store " << calibHitContainer.store());
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG ("--> ZDC: ZdcMCTruthAlg execute starting on "
                 << getContext().evt()
                 << "th event");

  /******************************************
   * Get the ZdcModuleContainer and 
   * ZdcModuleSumContainer (output)
  ******************************************/
  xAOD::ZdcModuleContainer const* moduleContainer = nullptr;
  ATH_CHECK(evtStore()->retrieve(moduleContainer, m_zdcModuleContainerName));

  xAOD::ZdcModuleContainer const* moduleSumContainer = nullptr;
  ATH_CHECK(evtStore()->retrieve(moduleSumContainer, m_zdcSumContainerName));

  //Create local variables to hold the per module and per side sums
  float totalMod[2][7]={{0.0},{0.0}}, invisMod[2][7]={{0.0},{0.0}}, emMod[2][7]={{0.0},{0.0}}, nonEMMod[2][7]={{0.0},{0.0}}, escMod[2][7]={{0.0},{0.0}}; 
  float totalSide[2]={0.0},   invisSide[2]={0.0},   emSide[2]={0.0},   nonEMSide[2]={0.0},   escSide[2]={0.0};
  bool foundMod[2][7] = {{false},{false}};

  /******************************************
   * Sum all hits in each module
  ******************************************/
  for (const auto hit : *calibHitContainer){
    Identifier id = hit->cellID();
    int side = (m_zdcID->side(id) > 0) ? 1 : 0;
    int mod  =  m_zdcID->module(id);
    
    emMod[side][mod]    += hit->energyEM();
    nonEMMod[side][mod] += hit->energyNonEM();
    invisMod[side][mod] += hit->energyInvisible();
    totalMod[side][mod] += hit->energyTotal();
    escMod[side][mod]   += hit->energyEscaped();
  }

  /******************************************
   * Decorate the ZdcModuleContainer with the
   * per module sums and add the per module
   * sums to the per side sums
  ******************************************/
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleTruthTotalEnergy(m_zdcModuleTruthTotalEnergy);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleTruthInvisEnergy(m_zdcModuleTruthInvisEnergy);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleTruthEMEnergy   (m_zdcModuleTruthEMEnergy);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleTruthNonEMEnergy(m_zdcModuleTruthNonEMEnergy);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleTruthEscEnergy  (m_zdcModuleTruthEscEnergy);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdTileXpositionRelative (m_rpdTileXpositionRelative);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdTileYpositionRelative (m_rpdTileYpositionRelative);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,uint16_t> rpdTileRowNumber      (m_rpdTileRowNumber);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,uint16_t> rpdTileColumnNumber   (m_rpdTileColumnNumber);
  for (const auto zdcModule : *moduleContainer){
    if(zdcModule->zdcSide() == 0) continue;
    int side = (zdcModule->zdcSide() > 0) ? 1 : 0;
    int mod  =  zdcModule->zdcModule();
    foundMod[side][mod] = true;

    //Add row/column info for RPD processing
    if(mod == 4){
      int lucrodId = (side == 0) ? 4 : 2;
      int rpdChannel = zdcModule->zdcChannel();
      int lucrodChannel = rpdChannel%8;
      if(rpdChannel > 7)
        lucrodId ++;
      
      ATH_MSG_DEBUG("Accessing LUCROD ID " << lucrodId << 
                    ", lucrodChannel=" << lucrodChannel << 
                    " for RPD side " << zdcModule->zdcSide() <<
                    ", channel " << rpdChannel);

      float xpos = ZdcLucrodMapRun3::getInstance()->getLucrod(lucrodId)["x"][lucrodChannel];
      float ypos = ZdcLucrodMapRun3::getInstance()->getLucrod(lucrodId)["y"][lucrodChannel];
      unsigned int row = ZdcLucrodMapRun3::getInstance()->getLucrod(lucrodId)["row"][lucrodChannel];
      unsigned int col = ZdcLucrodMapRun3::getInstance()->getLucrod(lucrodId)["col"][lucrodChannel];
      rpdTileXpositionRelative(*zdcModule) = xpos;
      rpdTileYpositionRelative(*zdcModule) = ypos;
      rpdTileRowNumber        (*zdcModule) = row;
      rpdTileColumnNumber     (*zdcModule) = col;
    }

    //Only save RPD truth in channel 0
    if(mod == 4 && zdcModule->zdcChannel() != 0) continue;

    zdcModuleTruthTotalEnergy(*zdcModule) = totalMod[side][mod];
    zdcModuleTruthInvisEnergy(*zdcModule) = invisMod[side][mod];
    zdcModuleTruthEMEnergy   (*zdcModule) = emMod   [side][mod];
    zdcModuleTruthNonEMEnergy(*zdcModule) = nonEMMod[side][mod];
    zdcModuleTruthEscEnergy  (*zdcModule) = escMod  [side][mod];

    totalSide[side] += totalMod[side][mod];
    invisSide[side] += invisMod[side][mod];
    emSide[side]    += emMod   [side][mod];
    nonEMSide[side] += nonEMMod[side][mod];
    escSide[side]   += escMod  [side][mod];
  }

  for(int iside : {0,1}){
    for(int mod = 0; mod < 7; ++mod){
      ATH_MSG_DEBUG("ZDC " << iside << ":" << mod << " total energy = " << totalMod[iside][mod]);
      if(!foundMod[iside][mod]){
	      ATH_MSG_DEBUG("Failed to find " << iside << ":" << mod);
      }
    }
  }

  ATH_MSG_DEBUG("ZDC A total energy = " << totalSide[1] << 
                ", invisible energy = " << invisSide[1] << 
                ", EM energy = " << emSide[1] << 
                ", non EM energy = " << nonEMSide[1] <<
                ", escaped energy = " << escSide[1]);

  ATH_MSG_DEBUG("ZDC C total energy = " << totalSide[0] << 
                ", invisible energy = " << invisSide[0] << 
                ", EM energy = " << emSide[0] << 
                ", non EM energy = " << nonEMSide[0] <<
                ", escaped energy = " << escSide[0]);


  /******************************************
   * Decorate the Sums container with the
   * per side sums
  ******************************************/
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumTruthTotalEnergy(m_zdcSumTruthTotalEnergy);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumTruthInvisEnergy(m_zdcSumTruthInvisEnergy);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumTruthEMEnergy   (m_zdcSumTruthEMEnergy);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumTruthNonEMEnergy(m_zdcSumTruthNonEMEnergy);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumTruthEscEnergy  (m_zdcSumTruthEscEnergy);
  for (const auto zdcSum : *moduleSumContainer){
    if (zdcSum->zdcSide() == 0) continue;
    int side = (zdcSum->zdcSide()==-1) ? 0 : 1;
    ATH_MSG_DEBUG("Filling zdcSum container side " << side);

    zdcSumTruthTotalEnergy(*zdcSum) = totalSide[side];
    zdcSumTruthInvisEnergy(*zdcSum) = invisSide[side];
    zdcSumTruthEMEnergy   (*zdcSum) = emSide   [side];
    zdcSumTruthNonEMEnergy(*zdcSum) = nonEMSide[side];
    zdcSumTruthEscEnergy  (*zdcSum) = escSide  [side];
  }

  return StatusCode::SUCCESS;
}


StatusCode ZdcMCTruthAlg::finalize()
{
  ATH_MSG_DEBUG( "--> ZDC: ZdcMCTruthAlg finalize complete" );

  return StatusCode::SUCCESS;
}
