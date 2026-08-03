/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXSysSim - Overall gFEX simulation
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#ifndef gFEXSysSim_H
#define gFEXSysSim_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IgFEXSysSim.h"
#include "gFEXSim.h"
#include "L1CaloFEXSim/gTower.h"

#include "xAODTrigger/gFexJetRoI.h"
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexJetRoIAuxContainer.h"
#include "xAODTrigger/gFexGlobalRoI.h"
#include "xAODTrigger/gFexGlobalRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoIAuxContainer.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "TrigConfData/L1Menu.h"

namespace LVL1 {

  //Doxygen class description below:
  /** The gFEXSysSim class defines the structure of the gFEX system
      Its purpose is:
      - to follow the structure of the gFEX and its FPGAs in as much
      detail as necessary to simulate the output of the system
      It will need to interact with gTowers and produce the gTOBs
  */

  class gFEXSysSim : public extends<AthAlgTool, IgFEXSysSim> {

  public:
    /** Constructors */
    using base_class::base_class;

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;

    virtual StatusCode execute(const EventContext& ctx, gFEXOutputCollection* gFEXOutputs) const override ;

    virtual int calcTowerID(int eta, int phi, int nphi, int mod) const override ;

    /**Create and fill a new gFexJetRoI object, and return a pointer to it*/
    virtual StatusCode fillgRhoEDM(xAOD::gFexJetRoIContainer* gRhoContainer, uint32_t tobWord, int scale) const override ;

    virtual StatusCode fillgBlockEDM(xAOD::gFexJetRoIContainer* gBlockContainer, uint32_t tobWord, int scale) const override ;

    virtual StatusCode fillgJetEDM(xAOD::gFexJetRoIContainer* gJetContainer, uint32_t tobWord, int scale) const override ;

    virtual StatusCode fillgScalarEJwojEDM(xAOD::gFexGlobalRoIContainer* gScalarEJwojContainer, uint32_t tobWord, int scale1, int scale2) const override ;

    virtual StatusCode fillgMETComponentsJwojEDM(xAOD::gFexGlobalRoIContainer* gMETComponentsJwojContainer, uint32_t tobWord, int scale1, int scale2) const override ;

    virtual StatusCode fillgMHTComponentsJwojEDM(xAOD::gFexGlobalRoIContainer* gMHTComponentsJwojContainer, uint32_t tobWord, int scale1, int scale2) const override ;

    virtual StatusCode fillgMSTComponentsJwojEDM(xAOD::gFexGlobalRoIContainer* gMSTComponentsJwojContainer, uint32_t tobWord, int scale1, int scale2) const override ;

    virtual StatusCode fillgMETComponentsNoiseCutEDM(xAOD::gFexGlobalRoIContainer* gMETComponentsNoiseCutContainer, uint32_t tobWord, int scale1, int scale2) const override ;
  
    virtual StatusCode fillgMETComponentsRmsEDM(xAOD::gFexGlobalRoIContainer* gMETComponentsRmsContainer, uint32_t tobWord, int scale1, int scale2) const override ;

    virtual StatusCode fillgScalarENoiseCutEDM(xAOD::gFexGlobalRoIContainer* gScalarENoiseCutContainer, uint32_t tobWord, int scale1, int scale2) const override ;
 
    virtual StatusCode fillgScalarERmsEDM(xAOD::gFexGlobalRoIContainer* gScalarERmsContainer, uint32_t tobWord, int scale1, int scale2) const override ;


    /** Internal data */
  private:


    ToolHandle<IgFEXSim> m_gFEXSimTool       {this, "gFEXSimTool",    "LVL1::gFEXSim",    "Tool that creates the gFEX Simulation"};

    SG::ReadHandleKey<LVL1::gTowerContainer> m_gTowerContainerSGKey {this, "MyGTowers", "gTowerContainer", "Input container for gTowers"};
    SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey{this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","Name of the L1Menu object to read configuration from"};
    
    SG::WriteHandleKey< xAOD::gFexJetRoIContainer > m_gFexRhoOutKey {this,"Key_gFexRhoOutputContainer","L1_gFexRhoRoI","Output gFexRho (energy density) container"};
    SG::WriteHandleKey< xAOD::gFexJetRoIContainer > m_gFexBlockOutKey {this,"Key_gFexSRJetOutputContainer","L1_gFexSRJetRoI","Output gFexBlock (small-R jet) container"};
    SG::WriteHandleKey< xAOD::gFexJetRoIContainer > m_gFexJetOutKey {this,"Key_gFexLRJetOutputContainer","L1_gFexLRJetRoI","Output gFexJet (large-R jet) container"};
    
    SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarEJwojOutKey {this,"Key_gScalarEJwojOutputContainer","L1_gScalarEJwoj","Output Scalar MET and SumET (from Jets without Jets algo) container"};
    SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsJwojOutKey {this,"Key_gMETComponentsJwojOutputContainer","L1_gMETComponentsJwoj","Output total MET components (from Jets without Jets algo) container"};
    SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMHTComponentsJwojOutKey {this,"Key_gMHTComponentsJwojOutputContainer","L1_gMHTComponentsJwoj","Output hard MET components (from Jets without Jets algo) container"};
    SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMSTComponentsJwojOutKey {this,"Key_gMSTComponentsJwojOutputContainer","L1_gMSTComponentsJwoj","Output soft MET components (from Jets without Jets algo) container"};

    SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsNoiseCutOutKey {this,"Key_gMETComponentsNoiseCutOutputContainer","L1_gMETComponentsNoiseCut","Output total MET components (from Noise Cut algo) container"};
    SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsRmsOutKey {this,"Key_gMETComponentsRmsOutputContainer","L1_gMETComponentsRms","Output total MET components (from RMS algo) container"};
    SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarENoiseCutOutKey {this,"Key_gScalarENoiseCutOutputContainer","L1_gScalarENoiseCut","Output Scalar MET and SumET (from Noise Cut algo) container"};
    SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarERmsOutKey {this,"Key_gScalarERmsOutputContainer","L1_gScalarERms","Output Scalar MET and SumET (from RMS algo) container"};

  };

} // end of namespace


#endif
