/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GFEXDRIVER_H
#define GFEXDRIVER_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "L1CaloFEXToolInterfaces/IgFEXSysSim.h"
#include "L1CaloFEXSim/gFEXOutputCollection.h"


class CaloIdManager;

namespace LVL1 {

class gFEXDriver : public AthAlgorithm
{
 public:
  //using AthReentrantAlgorithm::AthReentrantAlgorithm;

  gFEXDriver(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~gFEXDriver();

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

 private:

  //Declare that gFEXDriver class will write an object of type LVL1::gTowerContainer, one of type gFEXOutputCollection
  SG::WriteHandleKey<gFEXOutputCollection> m_gFEXOutputCollectionSGKey {this, "MyOutputs", "gFEXOutputCollection", "MyOutputs"};

  ToolHandle<IgFEXSysSim> m_gFEXSysSimTool {this, "gFEXSysSimTool", "LVL1::gFEXSysSim", "Tool that creates the gFEX System Simulation"};

};

} // end of LVL1 namespace
#endif
