/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L0MUONNSW_NSWSIMULATION_H
#define L0MUONNSW_NSWSIMULATION_H


// Basic includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ConcurrencyFlags.h"

// L0MuonSimTools includes
#include "L0MuonNSWSimTools/IMMTriggerTool.h"
#include "L0MuonNSWSimTools/IPadEmulatorTool.h"
#include "L0MuonNSWSimTools/ITriggerProcessorTool.h"

// namespace for the NSW LVL0 related classes
namespace L0Muon {



  class NSWSimulation: public AthReentrantAlgorithm {

  public:
    NSWSimulation( const std::string& name, ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;
    const ServiceHandle<ITHistSvc>& histSvc() const { return m_histSvc; }

  protected:
    SG::WriteHandleKey<Muon::NSW_TrigRawDataContainer> m_trigRdoContainer{this, "NSWTrigRDOContainerName", "L0_NSWTrigContainer", "Name of the NSW trigger RDO container"};

  private:
    ToolHandle <L0Muon::IPadEmulatorTool>      m_pad_emulator{this, "PadEmulatorTool", "L0Muon::PadEmulatorTool", "Tool simulating the sTGC Pad Trigger"};
    ToolHandle <L0Muon::IMMTriggerTool>        m_mmtrigger{this, "MMTriggerTool", "L0Muon::MMTriggerTool", "Tool simulating the MM Trigger"};
    ToolHandle <L0Muon::ITriggerProcessorTool> m_trigProcessor{this, "TriggerProcessorTool", "L0Muon::TriggerProcessorTool", "Tool simulating the TP"};

    Gaudi::Property<bool> m_doNtuple{this, "DoNtuple", false,  "Create an ntuple for data analysis"};
    Gaudi::Property<bool> m_doMM{this, "DoMM", false, "Run data analysis for MM"};
    Gaudi::Property<bool> m_doMMDiamonds{this, "DoMMDiamonds", false, "Run data analysis for MM using Diamond Roads algorithm"};
    Gaudi::Property<bool> m_dosTGC{this, "DosTGC", false, "Run data analysis for sTGCs"};
    Gaudi::Property<bool> m_doStrip{this, "DoStrip", false, "Run data analysis for sTGC strip trigger"};
    Gaudi::Property<bool> m_doPad{this, "DoPad", false, "Run data analysis for sTGC pad trigger"};

    // External services
    ServiceHandle<ITHistSvc> m_histSvc;
    mutable MuonVal::MuonTesterTree m_altree ATLAS_THREAD_SAFE {"SimulationTree", "/NSWSimulation"};
  };  // end of NSWSimulation class
} // namespace L0Muon
#endif
