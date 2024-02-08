/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFEXSysSim.h  -  
//                              -------------------
//     begin                : 12 07 2019
//     email                : alison.elliot@cern.ch, jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#ifndef jFEXSysSim_H
#define jFEXSysSim_H
#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjFEXSysSim.h"
#include "AthenaKernel/CLASS_DEF.h"

#include "L1CaloFEXCond/jFEXDBCondData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

#include "L1CaloFEXSim/jFEXSim.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexSRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTrigger/jFexTauRoIAuxContainer.h"
#include "xAODTrigger/jFexFwdElRoIContainer.h"
#include "xAODTrigger/jFexFwdElRoIAuxContainer.h"
#include "xAODTrigger/jFexMETRoIContainer.h"
#include "xAODTrigger/jFexMETRoIAuxContainer.h"
#include "xAODTrigger/jFexSumETRoIContainer.h"
#include "xAODTrigger/jFexSumETRoIAuxContainer.h"
#include "TrigConfData/L1Menu.h"

#include "L1CaloFEXSim/jFEXTOB.h"

namespace LVL1 {
  
  //Doxygen class description below:
  /** The jFEXSysSim class defines the structure of the jFEX system
      Its purpose is:
      - to follow the structure of the 24 jFEXes and their FPGAs in as much
      detail as necessary to simulate the output of the system
      It will need to interact with jTowers and produce the eTOBs
  */

  class jFEXSysSim : public AthAlgTool, virtual public IjFEXSysSim {
    
  public:
    
    /** Constructors */

    jFEXSysSim(const std::string& type,const std::string& name,const IInterface* parent);
    /** Destructor */
    jFEXSysSim&& operator= (const jFEXSysSim& ) = delete;

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode finalize  () override;

    virtual StatusCode execute(jFEXOutputCollection* inputOutputCollection) override ;

    virtual void init() const override;

    virtual void cleanup() override;

    virtual int calcTowerID(int eta, int phi, int mod) const override;


      
  /** Internal data */
  private:
    std::vector<jFEXSim*>  m_jFEXCollection;
    
    SG::ReadCondHandleKey<jFEXDBCondData> m_DBToolKey {this, "DBToolKey", "jFEXDBParams", "DB tool key"};
    
    ToolHandle<IjFEXSim> m_jFEXSimTool       {this, "jFEXSimTool",    "LVL1::jFEXSim",    "Tool that creates the jFEX Simulation"};

    SG::ReadHandleKey<LVL1::jTowerContainer> m_jTowerContainerSGKey {this, "MyETowers", "jTowerContainer", "Input container for jTowers"};
    
    SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey{this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","Name of the L1Menu object to read configuration from"};

    // HLT TOBs
    SG::WriteHandleKey< xAOD::jFexSRJetRoIContainer> m_TobOutKey_jJ   {this,"Key_jFexSRJetOutputContainer","L1_jFexSRJetRoI","Output jFexEDM jets container"};
    SG::WriteHandleKey< xAOD::jFexLRJetRoIContainer> m_TobOutKey_jLJ  {this,"Key_jFexLRJetOutputContainer","L1_jFexLRJetRoI","Output jFexEDM Ljets container"};
    SG::WriteHandleKey< xAOD::jFexTauRoIContainer>   m_TobOutKey_jTau {this,"Key_jFexTauOutputContainer"  ,"L1_jFexTauRoI"  ,"Output jFexEDM tau container"};
    SG::WriteHandleKey< xAOD::jFexFwdElRoIContainer> m_TobOutKey_jEM  {this,"Key_jFexFwdElOutputContainer","L1_jFexFwdElRoI","Output jFexEDM fwdEl container"};
    SG::WriteHandleKey< xAOD::jFexSumETRoIContainer> m_TobOutKey_jTE  {this,"Key_jFexSumETOutputContainer","L1_jFexSumETRoI","Output jFexEDM SumET container"};
    SG::WriteHandleKey< xAOD::jFexMETRoIContainer>   m_TobOutKey_jXE  {this,"Key_jFexMETOutputContainer"  ,"L1_jFexMETRoI"  ,"Output jFexEDM Met container"};
    
    // xTOBS
    SG::WriteHandleKey< xAOD::jFexSRJetRoIContainer> m_xTobOutKey_jJ   {this,"Key_xTobOutKey_jJ"   ,"L1_jFexSRJetxRoI","Output jFexEDM xTOBs jets container"};
    SG::WriteHandleKey< xAOD::jFexLRJetRoIContainer> m_xTobOutKey_jLJ  {this,"Key_xTobOutKey_jLJ"  ,"L1_jFexLRJetxRoI","Output jFexEDM xTOBs Ljets container"};
    SG::WriteHandleKey< xAOD::jFexTauRoIContainer>   m_xTobOutKey_jTau {this,"Key_xTobOutKey_jTau" ,"L1_jFexTauxRoI"  ,"Output jFexEDM xTOBs tau container"};
    SG::WriteHandleKey< xAOD::jFexFwdElRoIContainer> m_xTobOutKey_jEM  {this,"Key_xTobOutKey_jEM"  ,"L1_jFexFwdElxRoI","Output jFexEDM xTOBs fwdEl container"};    

    std::unordered_map<int,jTower> m_jTowersColl;

    std::unordered_map<uint8_t, std::vector<std::vector<std::vector<uint32_t>>> > m_allfwdElTobs;

    std::unordered_map<uint8_t, std::vector<std::vector<std::unique_ptr<jFEXTOB>>> > m_alltauTobs;
    std::unordered_map<uint8_t, std::vector<std::vector<std::unique_ptr<jFEXTOB>>> > m_allSmallRJetTobs; 
    std::unordered_map<uint8_t, std::vector<std::vector<std::unique_ptr<jFEXTOB>>> > m_allLargeRJetTobs;
    std::unordered_map<uint8_t, std::vector<std::unique_ptr<jFEXTOB>> > m_allsumEtTobs;
    std::unordered_map<uint8_t, std::vector<std::unique_ptr<jFEXTOB>> > m_allMetTobs; 
    
    
    // Create and fill a new EDMs object
    StatusCode fillSRJetEDM(uint8_t jFexNum, uint8_t fpgaNumber, uint32_t tobWord, char istob, int resolution, float_t eta, float_t phi, std::unique_ptr< xAOD::jFexSRJetRoIContainer > &jContainer) const;
    StatusCode fillLRJetEDM(uint8_t jFexNum, uint8_t fpgaNumber, uint32_t tobWord, char istob, int resolution, float_t eta, float_t phi, std::unique_ptr< xAOD::jFexLRJetRoIContainer > &jContainer) const;
    StatusCode fillTauEDM  (uint8_t jFexNum, uint8_t fpgaNumber, uint32_t tobWord, char istob, int resolution, float_t eta, float_t phi, std::unique_ptr< xAOD::jFexTauRoIContainer   > &jContainer) const;
    StatusCode fillFwdElEDM(uint8_t jFexNum, uint8_t fpgaNumber, uint32_t tobWord, char istob, int resolution, float_t eta, float_t phi, std::unique_ptr< xAOD::jFexFwdElRoIContainer > &jContainer) const;
    StatusCode fillSumEtEDM(uint8_t jFexNum, uint8_t fpgaNumber, uint32_t tobWord, int resolution, std::unique_ptr< xAOD::jFexSumETRoIContainer > &jContainer) const;
    StatusCode fillMetEDM  (uint8_t jFexNum, uint8_t fpgaNumber, uint32_t tobWord, int resolution, std::unique_ptr< xAOD::jFexMETRoIContainer   > &jContainer) const;
       
  };
  
} // end of namespace

//CLASS_DEF( LVL1::jFEXSysSim , 141823245 , 1 )


#endif
