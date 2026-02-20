/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1NSW_NSWL1SIMULATION_H
#define TRIGT1NSW_NSWL1SIMULATION_H

// Basic includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ConcurrencyFlags.h"

// NSWL1SimTools includes
#include "TrigT1NSWSimTools/IMMTriggerTool.h"
#include "TrigT1NSWSimTools/IPadEmulatorTool.h"
#include "TrigT1NSWSimTools/ITriggerProcessorTool.h"

// namespace for the NSW LVL1 related classes
namespace NSWL1 {


  /**
   *
   *   @short NSW L1 simulation algorithm
   *
   * This is the algorithm that masters the simulation of the Level-1 into the NSW.
   * This algorithm will manage the configuration of the NSW LV1 and handle the tools
   * used for simulating the hardware components, for monitoring the basic functionalities
   * and for collecting the data into an ntuple (parameter definition and performance 
   * studies). Depending of the job configuration it executes the pure offline simulation
   * or the detailed hardware simulation.
   *
   *  @authors Alessandro Di Mattia <dimattia@cern.ch>, Geraldine Conti <geraldine.conti@cern.ch>
   *  Major updates for Release 22 processing: Francesco Giuseppe Gravili <francesco.giuseppe.gravili@cern.ch>
   *
   */

  class NSWL1Simulation: public AthReentrantAlgorithm {

  public:
    NSWL1Simulation( const std::string& name, ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;
    const ServiceHandle<ITHistSvc>& histSvc() const { return m_histSvc; }

  protected:
    SG::WriteHandleKey<Muon::NSW_TrigRawDataContainer> m_trigRdoContainer{this, "NSWTrigRDOContainerName", "L1_NSWTrigContainer", "Name of the NSW trigger RDO container"};

  private:
    ToolHandle <IPadEmulatorTool>      m_pad_emulator{this, "PadEmulatorTool", "NSWL1::PadEmulatorTool", "Tool simulating the sTGC Pad Trigger"};
    ToolHandle <IMMTriggerTool>        m_mmtrigger{this, "MMTriggerTool", "NSWL1::MMTriggerTool", "Tool simulating the MM Trigger"};
    ToolHandle <ITriggerProcessorTool> m_trigProcessor{this, "TriggerProcessorTool", "NSWL1::TriggerProcessorTool", "Tool simulating the TP"};

    Gaudi::Property<bool> m_doNtuple{this, "DoNtuple", false,  "Create an ntuple for data analysis"};
    Gaudi::Property<bool> m_doMM{this, "DoMM", false, "Run data analysis for MM"};
    Gaudi::Property<bool> m_doMMDiamonds{this, "DoMMDiamonds", false, "Run data analysis for MM using Diamond Roads algorithm"};
    Gaudi::Property<bool> m_dosTGC{this, "DosTGC", false, "Run data analysis for sTGCs"};
    Gaudi::Property<bool> m_doStrip{this, "DoStrip", false, "Run data analysis for sTGC strip trigger"};
    Gaudi::Property<bool> m_doPad{this, "DoPad", false, "Run data analysis for sTGC pad trigger"};

    // External services
    ServiceHandle<ITHistSvc> m_histSvc;
    mutable MuonVal::MuonTesterTree m_altree ATLAS_THREAD_SAFE {"SimulationTree", "/NSWL1Simulation"};
  };  // end of NSWL1Simulation class
} // namespace NSWL1
#endif
