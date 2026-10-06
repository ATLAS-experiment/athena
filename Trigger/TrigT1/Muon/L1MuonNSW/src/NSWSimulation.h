/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONNSW_NSWSIMULATION_H
#define L1MUONNSW_NSWSIMULATION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "MuonDigitContainer/MmDigitContainer.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"

#include "xAODL0MuonCand/NSWCandData.h"
#include "xAODL0MuonCand/NSWCandDataContainer.h"
#include "xAODL0MuonCand/NSWCandDataAuxContainer.h"

/**
 * @class NSWSimulation
 * @brief Algorithm to process New Small Wheel digits and produce Run 4 L1Muon trigger candidates.
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
    SG::ReadHandleKey<sTgcDigitContainer> m_keySTgcDigit{this, "sTGCDigits", "sTGC_DIGITS", "Input sTGC digit container"};
    /// Output NSW trigger candidate container
    SG::WriteHandleKey<xAOD::NSWCandDataContainer> m_outputKey{ this, "OutputNSWCandidates", "L0MuonNSWCandidates", "Output NSW trigger candidates"};

    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};
  
  };

}   // end of namespace

#endif  // L1MUONNSW_NSWSIMULATION_H

