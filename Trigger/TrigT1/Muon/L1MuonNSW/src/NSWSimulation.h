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
 * Run 4 counterpart of the Run 3 NSWL1::NSWL1Simulation: it calls the MM trigger tool
 * (and later the sTGC one) and encodes every candidate into one L1NSWCandData segment word.
 *
 * Like Run 3, all candidates are written: no duplicate removal and no cap at 8 per board and BC.
 * The frame header fields are:
 *   boardID     = 16*side + (sector-1)  (provisional, see NSWEncoding.h)
 *   fiberID     = 0                     (the 2-fibre split is not emulated)
 *   l1NSegments = number of segments written for this (boardID, BC), may exceed 4
 *   l1Overflow  = that number > 8, i.e. the hardware would have overflowed
 *   l1Bcid      = BC of the candidate as in Run 3 (strip-time BC, not an LHC BCID)
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

    /// Segment encoding (Run 4 interface document, EDMS 2377909, Section 4.2)
    Gaudi::Property<double> m_dThetaStep{this, "DThetaStep", 0.001, "Delta theta LSB [rad]"};
    Gaudi::Property<double> m_dThetaRange{this, "DThetaRange", 0.015,
                                          "|Delta theta| covered by the 4 magnitude bits [rad] (Run 3: +-15 mrad)"};
    Gaudi::Property<unsigned int> m_maxSegmentsPerBoard{this, "MaxSegmentsPerBoard", 8,
                                                        "Hardware capacity per board and BC, used for l1Overflow only"};

    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};
  };

}   // end of namespace

#endif  // L1MUONNSW_NSWSIMULATION_H
