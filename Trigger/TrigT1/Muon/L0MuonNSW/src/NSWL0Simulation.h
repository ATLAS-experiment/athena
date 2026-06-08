/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1NSW_NSWL0SIMULATION_H
#define TRIGT1NSW_NSWL0SIMULATION_H

// Basic includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ConcurrencyFlags.h"

// NSWL0SimTools includes
#include "TrigT1NSWSimTools/IMMTriggerTool.h"
#include "TrigT1NSWSimTools/IPadEmulatorTool.h"
#include "TrigT1NSWSimTools/ITriggerProcessorTool.h"

// namespace for the NSW LVL1 related classes
namespace NSWL0 {


  /**
   *

   *
   */

  class NSWL0Simulation: public AthReentrantAlgorithm {

  public:
    NSWL0Simulation( const std::string& name, ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;
    const ServiceHandle<ITHistSvc>& histSvc() const { return m_histSvc; }

  protected:
    SG::WriteHandleKey<Muon::NSW_TrigRawDataContainer> m_trigRdoContainer{this, "NSWTrigRDOContainerName", "L1_NSWTrigContainer", "Name of the NSW trigger RDO container"};

  private:
    ToolHandle <NSWL0::IPadEmulatorTool>      m_pad_emulator{this, "PadEmulatorTool", "NSWL0::PadEmulatorTool", "Tool simulating the sTGC Pad Trigger"};
    ToolHandle <NSWL0::IMMTriggerTool>        m_mmtrigger{this, "MMTriggerTool", "NSWL0::MMTriggerTool", "Tool simulating the MM Trigger"};
    ToolHandle <NSWL0::ITriggerProcessorTool> m_trigProcessor{this, "TriggerProcessorTool", "NSWL0::TriggerProcessorTool", "Tool simulating the TP"};

    Gaudi::Property<bool> m_doNtuple{this, "DoNtuple", false,  "Create an ntuple for data analysis"};
    Gaudi::Property<bool> m_doMM{this, "DoMM", false, "Run data analysis for MM"};
    Gaudi::Property<bool> m_doMMDiamonds{this, "DoMMDiamonds", false, "Run data analysis for MM using Diamond Roads algorithm"};
    Gaudi::Property<bool> m_dosTGC{this, "DosTGC", false, "Run data analysis for sTGCs"};
    Gaudi::Property<bool> m_doStrip{this, "DoStrip", false, "Run data analysis for sTGC strip trigger"};
    Gaudi::Property<bool> m_doPad{this, "DoPad", false, "Run data analysis for sTGC pad trigger"};

    // External services
    ServiceHandle<ITHistSvc> m_histSvc;
    mutable MuonVal::MuonTesterTree m_altree ATLAS_THREAD_SAFE {"SimulationTree", "/NSWL0Simulation"};
  };  // end of NSWL0Simulation class
} // namespace NSWL0
#endif
