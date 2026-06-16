#ifndef EFEXDRIVER_H
#define EFEXDRIVER_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "eFEXSysSim.h"
#include "L1CaloFEXSim/eFEXOutputCollection.h"

namespace LVL1 {

class eFEXDriver : public AthAlgorithm
{
 public:
  //using AthReentrantAlgorithm::AthReentrantAlgorithm;

  eFEXDriver(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~eFEXDriver();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx);
  virtual StatusCode finalize() override;

 private:

  SG::WriteHandleKey<eFEXOutputCollection> m_eFEXOutputCollectionSGKey {this, "MyOutputs", "eFEXOutputCollection", "MyOutputs"};

  ToolHandle<eFEXSysSim> m_eFEXSysSimTool {this, "eFEXSysSimTool", "LVL1::eFEXSysSim", "Tool that creates the eFEX System Simulation"};

};

} // end of LVL1 namespace
#endif
