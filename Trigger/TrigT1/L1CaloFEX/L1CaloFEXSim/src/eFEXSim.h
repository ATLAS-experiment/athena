/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXSim.h  -  
//                              -------------------
//     begin                : 22 08 2019
//     email                : jacob.julian.kempster@cern.ch
//  ***************************************************************************/


#ifndef eFEXSim_H
#define eFEXSim_H
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/eTower.h"
#include "eFEXFPGA.h"
#include "L1CaloFEXSim/eFEXOutputCollection.h"
#include "L1CaloFEXSim/eFEXegTOB.h"
#include "CaloEvent/CaloCellContainer.h"

namespace LVL1 {
  
  //Doxygen class description below:
  /** The eFEXSim class defines the structure of a single eFEX
      Its purpose is:
      - to emulate the steps taken in processing data for a single eFEX in hardware and firmware
      - It will need to interact with eTowers and produce the eTOBs.  It will be created and handed data by eFEXSysSim
  */
  static const InterfaceID IID_IeFEXSim("LVL1::eFEXSim", 1, 0);

    class eFEXSim : public AthAlgTool {
    
  public:
        static const InterfaceID& interfaceID() { return IID_IeFEXSim; };
    /** Constructors */
    eFEXSim(const std::string& type,const std::string& name,const IInterface* parent);

    /** Destructor */
    virtual ~eFEXSim();

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize();
    /** standard Athena-Algorithm method */
    virtual StatusCode finalize  ();

    virtual void init (int id);

    virtual void reset ();

    virtual void execute();

    virtual int ID() const {return m_id;}
    
    virtual void SetTowersAndCells_SG(int tmp[10][18]);

    virtual StatusCode NewExecute(int tmp[10][18], eFEXOutputCollection* inputOutputCollection);

    virtual std::vector<std::unique_ptr<eFEXegTOB>> getEmTOBs();
    virtual std::vector<std::unique_ptr<eFEXtauTOB>> getTauHeuristicTOBs();
    virtual std::vector<std::unique_ptr<eFEXtauTOB>> getTauBDTTOBs();

  private:

    std::vector<std::unique_ptr<eFEXtauTOB>> getTauTOBs(std::vector<std::vector<std::unique_ptr<eFEXtauTOB>> >& tauTobObjects);
    /** Internal data */
  private:
    int m_id{};
    int m_eTowersIDs [10][18]{};
    CaloCellContainer m_sCellsCollection;
    std::vector<eFEXFPGA*> m_eFEXFPGACollection;

    std::vector<std::vector<std::unique_ptr<eFEXegTOB>> > m_emTobObjects;
    std::vector<std::vector<std::unique_ptr<eFEXtauTOB>> > m_tauHeuristicTobObjects;
    std::vector<std::vector<std::unique_ptr<eFEXtauTOB>> > m_tauBDTTobObjects;

    ToolHandle<eFEXFPGA> m_eFEXFPGATool {this, "eFEXFPGATool", "LVL1::eFEXFPGA", "Tool that simulates the FPGA hardware"};

    
  };
  
} // end of namespace

//CLASS_DEF( LVL1::eFEXSim , 32201200 , 1 )


#endif
