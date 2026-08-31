/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PU1SuppAlgTool.h"

#include "GaudiKernel/EventContext.h"
#include "PU1Suppression.h"  // Includes runPU1suppression()
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

namespace GlobalSim {

/// Constructor
PU1SuppAlgTool::PU1SuppAlgTool(const std::string& type, const std::string& name,
                               const IInterface* parent)
    : extends(type, name, parent) {}

StatusCode PU1SuppAlgTool::initialize() {
  ATH_MSG_INFO("Initializing PU1SuppAlgTool");

  // Initialize read/write handles
  ATH_CHECK(m_HypoFIFOReadKey.initialize());
  ATH_CHECK(m_portsOutWriteKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PU1SuppAlgTool::run(const std::unique_ptr<IDataCollector>& dc,
			       const EventContext& ctx) const {
  ATH_MSG_DEBUG("Running PU1SuppAlgTool");
  if (dc){dc->collect(*this, "start");}

  // Read input FIFO of TOBs
  SG::ReadHandle<GepAlgoPU1SuppFIFO> fifoHandle(m_HypoFIFOReadKey, ctx);
  if (!fifoHandle.isValid()) {
    ATH_MSG_ERROR(
        "Failed to retrieve PU1 TOBs with key: " << m_HypoFIFOReadKey.key());
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Read " << fifoHandle->size() << " TOBs from event store");

  // Create container for output TOBs
  auto ports_out = std::make_unique<GepAlgoPU1SuppPortsOutFIFO>();

  // Apply suppression to each input TOB
  for (const auto& tob_in : *fifoHandle) {
    PU1SuppPortsOut tob_out;
    const StatusCode ok = runPU1Suppression(tob_in, tob_out, msg());
    if (ok.isFailure()) {
      ATH_MSG_WARNING("Suppression failed for one TOB - skipping");
      continue;
    }
    ports_out->push_back(std::move(tob_out));
  }

  // Optionally print output TOB hex strings
  for (const auto& tob_out : *ports_out) {
    for (const auto& hexstr : tob_out.m_outputTobs) {
      ATH_MSG_DEBUG("PU1SuppPortsOut TOB hex string: " << hexstr);
    }
  }

  // Write output FIFO to event store
  SG::WriteHandle<GepAlgoPU1SuppPortsOutFIFO> outputHandle(m_portsOutWriteKey,
                                                           ctx);
  ATH_CHECK(outputHandle.record(std::move(ports_out)));

  ATH_MSG_DEBUG("PU1 suppression outputs recorded to event store under key: "
                << m_portsOutWriteKey.key());


  if (dc){dc->collect(*this, "end");}
  return StatusCode::SUCCESS;
}

std::string PU1SuppAlgTool::toString() const {
  return "PU1SuppAlgTool: Tool for PU1 suppression";
}

}  // namespace GlobalSim
