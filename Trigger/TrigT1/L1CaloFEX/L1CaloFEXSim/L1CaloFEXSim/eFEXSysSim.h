/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXSysSim.h  -  
//                              -------------------
//     begin                : 12 07 2019
//     email                : alison.elliot@cern.ch, jacob.julian.kempster@cern.ch
//  ***************************************************************************/


#ifndef eFEXSysSim_H
#define eFEXSysSim_H
#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IeFEXSysSim.h"
#include "L1CaloFEXToolInterfaces/IeFEXFillEDM.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/eFEXSim.h"
#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXSim/eFEXFPGATowerIdProvider.h"
#include "L1CaloFEXSim/eFEXegTOB.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"

#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexEMRoIAuxContainer.h"

#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/eFexTauRoIAuxContainer.h"

namespace LVL1 {
  
  //Doxygen class description below:
  /** The eFEXSysSim class defines the structure of the eFEX system
      Its purpose is:
      - to follow the structure of the 24 eFEXes and their FPGAs in as much
      detail as necessary to simulate the output of the system
      It will need to interact with eTowers and produce the eTOBs
  */

  class eFEXSysSim : public AthAlgTool, virtual public IeFEXSysSim {
    
  public:
    
    /** Constructors */

    eFEXSysSim(const std::string& type,const std::string& name,const IInterface* parent);
    /** Destructor */
    eFEXSysSim&& operator= (const eFEXSysSim& ) = delete;

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;

    virtual StatusCode execute(eFEXOutputCollection* inputOutputCollection) override ;

    virtual int calcTowerID(int eta, int phi, int mod) const override;

    /** Internal data */
  private:
    // TOB sorting function
    template <class TOBObjectClass> static bool TOBetSort(const TOBObjectClass& i, const TOBObjectClass& j, bool isTau ) {
      auto et_i = (i->getTobword() >> 0 ) & 0xfff;
      auto et_j = (j->getTobword() >> 0 ) & 0xfff;
      if(et_i > et_j) return true;
      if(et_i==et_j) {
        // resolve ties with procNumber (2,1,3,0 for em, 2,1,0,3 for tau), then phi, then eta
        auto procOrder = (isTau) ? std::map<unsigned int,unsigned int>{{2,3},{1,2},{0,1},{3,0}} :
                                   std::map<unsigned int,unsigned int>{{2,3},{1,2},{3,1},{0,0}};
        auto proc_i = procOrder.at((i->getTobword()) >> 30);
        auto proc_j = procOrder.at((j->getTobword()) >> 30);
        if(proc_i > proc_j) return true;
        if (proc_i == proc_j) {
          auto phi_i = (i->getTobword()) & 0x07000000;
          auto phi_j = (j->getTobword()) & 0x07000000;
          if(phi_i > phi_j) return true;
          if(phi_i == phi_j) {
            auto eta_i = (i->getTobword()) & 0x38000000;
            auto eta_j = (j->getTobword()) & 0x38000000;
            if(eta_i > eta_j) return true;
          }

        }
      }
      return false;
    }

    // Auxiliary for storing EDMs of both tau algos
    StatusCode StoreTauTOBs(std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >& allTauTobObjects,
		 SG::WriteHandleKey< xAOD::eFexTauRoIContainer >& eFexTauxTOBOutKey,
		 SG::WriteHandleKey< xAOD::eFexTauRoIContainer >& eFexTauOutKey);

    // EM TOBs and xTOBS
    ToolHandle<IeFEXSim> m_eFEXSimTool {this, "eFEXSimTool",    "LVL1::eFEXSim",    "Tool that creates the eFEX Simulation"};

    ToolHandle<IeFEXFillEDM> m_eFEXFillEDMTool {this, "eFEXFillEDMTool", "LVL1::eFEXFillEDM", "Tool to fille eFEX EDMs"};

    SG::ReadHandleKey<LVL1::eTowerContainer> m_eTowerContainerSGKey {this, "MyETowers", "eTowerContainer", "Input container for eTowers"};
    SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey{this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","Name of the L1Menu object to read configuration from"}; 

    SG::WriteHandleKey< xAOD::eFexEMRoIContainer > m_eFexOutKey {this,"Key_eFexEMOutputContainer","L1_eEMRoI","Output eFexEM TOB container"};
    SG::WriteHandleKey< xAOD::eFexEMRoIContainer > m_eFexEMxTOBOutKey {this,"Key_eFexEMxTOBOutputContainer","L1_eEMxRoI","Output eFexEM xTOB container"};
    SG::WriteHandleKey< xAOD::eFexTauRoIContainer > m_eFexTauActiveOutKey {this,"Key_eFexTauOutputContainer","L1_eTauRoI","Output eFexTau active (BDT/heuristic) algorithm TOB container"};
    SG::WriteHandleKey< xAOD::eFexTauRoIContainer > m_eFexTauActivexTOBOutKey {this,"Key_eFexTauxTOBOutputContainer","L1_eTauxRoI","Output eFexTau active (BDT/heuristic) algorithm xTOB container"};
    SG::WriteHandleKey< xAOD::eFexTauRoIContainer > m_eFexTauAltOutKey {this,"Key_eFexAltTauOutputContainer","","Output eFexTau alternative (BDT/heuristic) algorithm TOB container"};
    SG::WriteHandleKey< xAOD::eFexTauRoIContainer > m_eFexTauAltxTOBOutKey {this,"Key_eFexAltTauxTOBOutputContainer","","Output eFexTau alternative (BDT/heuristic) algorithm xTOB container"};
    ToolHandle<IeFEXFPGATowerIdProvider> m_eFEXFPGATowerIdProviderTool {this, "eFEXFPGATowerIdProviderTool", "LVL1::eFEXFPGATowerIdProvider", "Tool that provides tower-FPGA mapping"};

    //std::map<int,eTower> m_eTowersColl;

  };
  
} // end of namespace

//CLASS_DEF( LVL1::eFEXSysSim , 32201258 , 1 )


#endif
