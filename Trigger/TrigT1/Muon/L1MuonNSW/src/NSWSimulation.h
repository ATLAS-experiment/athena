/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONNSW_NSWSIMULATION_H
#define L1MUONNSW_NSWSIMULATION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonDigitContainer/MmDigitContainer.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"

#include "xAODTrigL1Muon/L1NSWCandData.h"
#include "xAODTrigL1Muon/L1NSWCandDataContainer.h"
#include "xAODTrigL1Muon/L1NSWCandDataAuxContainer.h"

#include "IL1MMTriggerTool.h"

/**
 * @class NSWSimulation
 * @brief Algorithm to process New Small Wheel digits and produce Run 4 L1Muon trigger candidates.
 *
 * It runs the MM trigger processor emulation (MMTriggerTool) and, for the moment, still writes
 * dummy candidates. The encoding of the MM candidates into the output is the next step.
 */

namespace L1Muon {

  class NSWSimulation : public ::AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~NSWSimulation() = default;

    virtual StatusCode  initialize() override;
    virtual StatusCode  execute(const EventContext& ctx) const override;

  private:
    /// MM Digit container
    SG::ReadHandleKey<MmDigitContainer> m_keyMmDigit{this, "MMDigits", "MM_DIGITS", "Input MM digit container"};
    /// sTGC digits
    SG::ReadHandleKey<sTgcDigitContainer> m_keySTgcDigit{this, "sTGCDigits", "sTGC_DIGITS",
                                                          "Input sTGC digit container"};
    /// Output NSW trigger candidate container
    SG::WriteHandleKey<xAOD::L1NSWCandDataContainer> m_outputKey{this, "OutputNSWCandidates", "L1NSWCandidates",
                                                                  "Output NSW trigger candidates"};

    /// MM trigger processor emulation
    Gaudi::Property<bool> m_doMM{this, "DoMM", true, "Run the MM trigger simulation"};
    ToolHandle<L1MM::IL1MMTriggerTool> m_mmTrigger{this, "MMTriggerTool", "L1Muon::L1MM::MMTriggerTool",
                                                   "Tool simulating the MM trigger processor"};

    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};
  };

}   // end of namespace

#endif  // L1MUONNSW_NSWSIMULATION_H
