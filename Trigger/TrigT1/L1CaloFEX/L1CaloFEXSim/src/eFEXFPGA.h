/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXFPGA.h  -  
//                              -------------------
//     begin                : 15 10 2019
//     email                : jacob.julian.kempster@cern.ch
//  ***************************************************************************/


#ifndef eFEXFPGA_H
#define eFEXFPGA_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXSim/eFEXtauAlgoBase.h"
#include "L1CaloFEXSim/eFEXegAlgo.h"
#include "eFEXFormTOBs.h"
#include "L1CaloFEXSim/eFEXOutputCollection.h"
#include "TrigConfData/L1Menu.h"
#include "L1CaloFEXSim/eFEXegTOB.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"

#include <vector>

namespace LVL1 {
  
  //Doxygen class description below:
  /** The eFEXFPGA class defines the structure of a single eFEX FPGA
      Its purpose is:
      - to emulate the steps taken in processing data for a single eFEX FPGA in hardware and firmware
      - It will need to interact with eTowers and produce the eTOBs.  It will be created and handed data by eFEXSim
  */
  static const InterfaceID IID_IeFEXFPGA("LVL1::eFEXFPGA", 1, 0);
    class eFEXFPGA : public AthAlgTool {
    
  public:
        static const InterfaceID& interfaceID() { return IID_IeFEXFPGA; };
    /** Constructors */
    eFEXFPGA(const std::string& type,const std::string& name,const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize();
    /** Destructor */
    virtual ~eFEXFPGA();

    virtual StatusCode init(int id, int efexid);
    virtual StatusCode execute(eFEXOutputCollection* inputOutputCollection);
    virtual void reset();
    virtual int getID() const {return m_id;}

    virtual void SetTowersAndCells_SG( int [][6] );
    virtual void SetIsoWP(const std::vector<unsigned int>&, const std::vector<unsigned int>&, unsigned int &, unsigned int) const;

    virtual std::vector <std::unique_ptr<eFEXegTOB>> getEmTOBs();
    virtual std::vector <std::unique_ptr<eFEXtauTOB>> getTauHeuristicTOBs();
    virtual std::vector <std::unique_ptr<eFEXtauTOB>> getTauBDTTOBs();

  private:
    std::vector<std::unique_ptr<eFEXtauTOB>> getTauTOBs(std::vector< std::unique_ptr<eFEXtauTOB> >& tauTobObjects);

    /** Internal data */
  private:
    const unsigned int m_eFexStep = 25;

    int m_id = 0;
    int m_efexid = 0;
    std::vector< std::unique_ptr<eFEXegTOB> > m_emTobObjects;
    std::vector< std::unique_ptr<eFEXtauTOB> > m_tauHeuristicTobObjects;
    std::vector< std::unique_ptr<eFEXtauTOB> > m_tauBDTTobObjects;
    int m_eTowersIDs [10][6]{};

    SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey{
      this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu",
      "Name of the L1Menu object to read configuration from"};

    SG::ReadHandleKey<LVL1::eTowerContainer> m_eTowerContainerKey {
      this, "MyETowers", "eTowerContainer", 
	"Input container for eTowers"};

    ToolHandle<eFEXtauAlgoBase> m_eFEXtauAlgoTool {
      this, "eFEXtauAlgoTool", "LVL1::eFEXtauAlgo", 
	"Tool that runs the eFEX tau algorithm"};

    ToolHandle<eFEXtauAlgoBase> m_eFEXtauBDTAlgoTool {
      this, "eFEXtauBDTAlgoTool", "LVL1::eFEXtauBDTAlgo", 
	"Tool that runs the eFEX BDT tau algorithm"};

    ToolHandle<eFEXegAlgo> m_eFEXegAlgoTool {
      this, "eFEXegAlgoTool", "LVL1::eFEXegAlgo", 
	"Tool that runs the eFEX e/gamma algorithm"};
    
    ToolHandle<eFEXFormTOBs> m_eFEXFormTOBsTool {this, "eFEXFormTOBs", "LVL1::eFEXFormTOBs", "Tool that creates eFEX TOB words"};
  };
  
} // end of namespace

//CLASS_DEF( LVL1::eFEXFPGA , 32201201 , 1 )


#endif
