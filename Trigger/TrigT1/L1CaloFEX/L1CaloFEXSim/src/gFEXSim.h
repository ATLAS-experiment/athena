/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXSim - Simulation of the gFEX module
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#ifndef gFEXSim_H
#define gFEXSim_H
#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IgFEXSim.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/gTower.h"
#include "gFEXFPGA.h"
#include "gFEXJetAlgo.h"
#include "L1CaloFEXSim/gFEXJetTOB.h"
#include "gFEXJwoJAlgo.h"
#include "L1CaloFEXSim/gFEXJwoJTOB.h"
#include "gFEXaltMetAlgo.h"
#include "L1CaloFEXSim/gFEXOutputCollection.h"
#include "TrigConfData/L1Menu.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"



namespace LVL1 {

  //Doxygen class description below:
  /** The gFEXSim class defines the structure of the gFEX
      Its purpose is:
      - to emulate the steps taken in processing data for gFEX in hardware and firmware
      - It will need to interact with gTowers and produce the gTOBs. It will be created and handed data by gFEXSysSim
  */

  class gFEXSim : public AthAlgTool, virtual public IgFEXSim {

  public:

    /** Constructors */
    gFEXSim(const std::string& type,const std::string& name,const IInterface* parent);

    /** Destructor */
    virtual ~gFEXSim();

    virtual StatusCode initialize() override ;

    virtual StatusCode execute(const EventContext& ctx,
			       const gTowersIDs& tmp_gTowersIDs_subset,
			       gFEXOutputCollection* gFEXOutputs,
			       std::vector<uint32_t>& gRhoTobWords,
			       std::vector<uint32_t>& gBlockTobWords,
			       std::vector<uint32_t>& gJetTobWords,
			       std::vector<int32_t>&  gScalarEJwojTobWords,
			       std::vector<uint32_t>& gMETComponentsJwojTobWords,
			       std::vector<uint32_t>& gMHTComponentsJwojTobWords,
			       std::vector<uint32_t>& gMSTComponentsJwojTobWords,
			       std::vector<uint32_t>& gMETComponentsNoiseCutTobWords,
			       std::vector<uint32_t>& gMETComponentsRmsTobWords,
			       std::vector<uint32_t>& gScalarENoiseCutTobWords,
			       std::vector<uint32_t>& gScalarERmsTobWords) const override;


    /** Internal data */
  private:

    ToolHandle<IgFEXFPGA> m_gFEXFPGA_Tool {this, "gFEXFPGATool", "LVL1::gFEXFPGA", "Tool that simulates the FPGA hardware"};

    ToolHandle<IgFEXJetAlgo> m_gFEXJetAlgoTool {this, "gFEXJetAlgoTool", "LVL1::gFEXJetAlgo", "Tool that runs the gFEX jet algorithm"};

    ToolHandle<IgFEXJwoJAlgo> m_gFEXJwoJAlgoTool {this, "gFEXJwoJAlgoTool", "LVL1::gFEXJwoJAlgo", "Tool that runs the gFEX Jets without Jets algorithm"};

    ToolHandle<IgFEXaltMetAlgo> m_gFEXaltMetAlgoTool {this, "gFEXaltMetAlgoTool", "LVL1::gFEXaltMetAlgo", "Tool that runs the gFEX noise cut and rho+RMS algorithms for MET"};

    SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey{this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","Name of the L1Menu object to read configuration from"}; 
  
    SG::WriteHandleKey < xAOD::gFexTowerContainer > m_gTowersWriteKey    {this,"gTowersWriteKey"   ,"L1_gFexTriggerTowers", "Write gFexEDM Trigger Tower container"};
  };

} // end of namespace


#endif
