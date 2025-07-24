/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFEXFPGA.h  -  
//                              -------------------
//     begin                : 15 10 2019
//     email                : jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#ifndef jFEXFPGA_H
#define jFEXFPGA_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXToolInterfaces/IjFEXFPGA.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXToolInterfaces/IjFEXSmallRJetAlgo.h"
#include "L1CaloFEXToolInterfaces/IjFEXLargeRJetAlgo.h"
#include "L1CaloFEXToolInterfaces/IjFEXtauAlgo.h"
#include "L1CaloFEXToolInterfaces/IjFEXsumETAlgo.h"
#include "L1CaloFEXToolInterfaces/IjFEXmetAlgo.h"
#include "L1CaloFEXToolInterfaces/IjFEXForwardJetsAlgo.h"
#include "L1CaloFEXToolInterfaces/IjFEXForwardElecAlgo.h"
#include "L1CaloFEXToolInterfaces/IjFEXPileupAndNoise.h"
#include "L1CaloFEXToolInterfaces/IjFEXFormTOBs.h"
#include "L1CaloFEXSim/jFEXOutputCollection.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "L1CaloFEXSim/jFEXForwardJetsInfo.h"
#include "L1CaloFEXSim/jFEXForwardElecInfo.h"
#include "TrigConfData/L1Menu.h"
#include <vector>
#include "SGTools/StlMapClids.h"
#include "StoreGate/ReadHandle.h"

#include "L1CaloFEXSim/jFEXTOB.h"

namespace LVL1 {
  
  //Doxygen class description below:
  /** The jFEXFPGA class defines the structure of a single jFEX FPGA
      Its purpose is:
      - to emulate the steps taken in processing data for a single jFEX FPGA in hardware and firmware
      - It will need to interact with jTowers and produce the jTOBs.  It will be created and handed data by jFEXSim
  */
  
  class jFEXFPGA : public AthAlgTool, virtual public IjFEXFPGA {
    
  public:
    /** Constructors */
    jFEXFPGA(const std::string& type,const std::string& name,const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;
    /** Destructor */
    virtual ~jFEXFPGA();

    virtual StatusCode init(int id, int efexid) override ;

    virtual StatusCode execute(jFEXOutputCollection* inputOutputCollection, const std::pair<unsigned int, const std::vector<int>&> & jetCalibrationParameters) override ;

    virtual void reset() override ;

    virtual int ID() override {return m_id;}

    virtual void SetTowersAndCells_SG( int [][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width] ) override ;
    virtual void SetTowersAndCells_SG( int [][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width] ) override ;

    /**Form a tob word out of the potential candidate SmallRJet tob */
    virtual std::vector <std::unique_ptr<jFEXTOB>> getSmallRJetTOBs() override;
    virtual std::vector <std::unique_ptr<jFEXTOB>> getLargeRJetTOBs() override;

    /** sorted Electron tobs */
    virtual std::vector <std::vector <uint32_t>> getFwdElTOBs() override;

    /* Retreive Tau TOB words  */
    virtual std::vector<std::unique_ptr<jFEXTOB>> getTauTOBs() override;    
       
    /**Form a tob word out of the potential candidate SumET tob */
    virtual std::vector<std::unique_ptr<jFEXTOB>> getSumEtTOBs() override;    
       
    /**Form a tob word out of the potential candidate MET tob */
    virtual std::vector<std::unique_ptr<jFEXTOB>> getMetTOBs() override;    
    
    int getTTowerET_EM     (unsigned int TTID ) override; 
    int getTTowerET_HAD    (unsigned int TTID ) override; 
    int getTTowerET        (unsigned int TTID ) override; 
    int getTTowerET_forMET (unsigned int TTID ) override; 
    
   /** Internal data */
  private:
  
    template <class TOBObjectClass> static bool TOBetSort(const TOBObjectClass& i, const TOBObjectClass& j , uint bits, uint mask) {
        return (((i->getWord() >> bits ) & mask)>((j->getWord() >> bits ) & mask)); 
    }
    
    static bool etFwdElSort  (const std::vector<uint32_t>& i, const std::vector<uint32_t>& j){ return (((i.at(0) >> FEXAlgoSpaceDefs::jEM_etBit ) & 0x7ff  )> ((j.at(0) >> FEXAlgoSpaceDefs::jEM_etBit) & 0x7ff ));}
    
    std::vector<std::unique_ptr<jFEXTOB>> m_tau_tobwords;
    std::vector<std::unique_ptr<jFEXTOB>> m_SRJet_tobwords;
    std::vector<std::unique_ptr<jFEXTOB>> m_LRJet_tobwords;  
    std::vector<std::unique_ptr<jFEXTOB>> m_sumET_tobwords;
    std::vector<std::unique_ptr<jFEXTOB>> m_Met_tobwords;    
      
    int m_id;
    int m_jfexid;

    std::vector<std::vector<uint32_t>> m_FwdEl_tobwords;

    int m_jTowersIDs_Wide [FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width] = {{0}};
    int m_jTowersIDs_Thin [FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width] = {{0}};
    
    std::unordered_map<int,jTower> m_jTowersColl;
    std::unordered_map<int,std::vector<int> > m_map_Etvalues_FPGA;
    std::unordered_map<int,std::vector<int> > m_map_HAD_Etvalues_FPGA;
    std::unordered_map<int,std::vector<int> > m_map_EM_Etvalues_FPGA;
    

    std::unordered_map<int, jFEXForwardJetsInfo> m_FCALJets; 
    std::unordered_map<uint, jFEXForwardElecInfo> m_ForwardElecs;

    int m_SRJetET;
    int m_LRJetET;

    // FIXME: unused?
    //CaloCellContainer m_sCellsCollection;

    SG::ReadHandleKey<LVL1::jTowerContainer> m_jTowerContainerKey {this, "MyETowers", "jTowerContainer", "Input container for jTowers"};
    SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey{this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","Name of the L1Menu object to read configuration from"}; 
    ToolHandle<IjFEXSmallRJetAlgo> m_jFEXSmallRJetAlgoTool {this, "jFEXSmallRJetAlgoTool", "LVL1::jFEXSmallRJetAlgo", "Tool that runs the jFEX Small R Jet algorithm"};
    ToolHandle<IjFEXLargeRJetAlgo> m_jFEXLargeRJetAlgoTool {this, "jFEXLargeRJetAlgoTool", "LVL1::jFEXLargeRJetAlgo", "Tool that runs the jFEX Large R Jet algorithm"};
    ToolHandle<IjFEXtauAlgo> m_jFEXtauAlgoTool             {this, "jFEXtauAlgoTool"      , "LVL1::jFEXtauAlgo"      , "Tool that runs the jFEX tau algorithm"};
    ToolHandle<IjFEXsumETAlgo> m_jFEXsumETAlgoTool         {this, "jFEXsumETAlgoTool"    , "LVL1::jFEXsumETAlgo"    , "Tool that runs the jFEX sumET algorithm"};
    ToolHandle<IjFEXmetAlgo> m_jFEXmetAlgoTool             {this, "jFEXmetAlgoTool"      , "LVL1::jFEXmetAlgo"      , "Tool that runs the jFEX met algorithm"};
    ToolHandle<IjFEXForwardJetsAlgo> m_jFEXForwardJetsAlgoTool {this, "jFEXForwardJetsAlgoTool"      , "LVL1::jFEXForwardJetsAlgo"      , "Tool that runs the jFEX FCAL Jets algorithm"};
    ToolHandle<IjFEXForwardElecAlgo> m_jFEXForwardElecAlgoTool {this, "jFEXForwardElecAlgoTool"      , "LVL1::jFEXForwardElecAlgo"      , "Tool that runs the jFEX FCAL Electrons algorithm"};
    ToolHandle<IjFEXPileupAndNoise> m_jFEXPileupAndNoiseTool {this, "jFEXPileupAndNoiseTool", "LVL1::jFEXPileupAndNoise", "Tool that applies Pileup and Noise"};
    ToolHandle<IjFEXFormTOBs> m_IjFEXFormTOBsTool {this, "IjFEXFormTOBsTool", "LVL1::jFEXFormTOBs", "Tool that forms TOB words"};
    
    int getTTowerET_SG(unsigned int TTID);
    std::string m_jfex_string[6] = {"1C","2C","3C","3A","2A","1A"};
  };
  
} // end of namespace


CLASS_DEF( LVL1::jFEXFPGA , 76081081 , 1 )
#endif
