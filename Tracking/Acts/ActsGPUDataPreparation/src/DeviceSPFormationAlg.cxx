/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceSPFormationAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// traccc EDM
#include "traccc/edm/silicon_cell_collection.hpp"
#include "traccc/edm/measurement_collection.hpp"

// vecmem
#include "vecmem/memory/memory_resource.hpp"

namespace ActsTrk {

// -----------------------------------------------------------------------
StatusCode DeviceSPFormationAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name());

  ATH_CHECK(m_spAlgProviderTool.retrieve());
  ATH_CHECK(m_deviceMR.retrieve());
  ATH_CHECK(m_inputMeasKey.initialize());
  ATH_CHECK(m_outputPixelSPKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_deviceDetector, m_deviceDetectorName.value()));

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode DeviceSPFormationAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing device SP formation.");

  // ---- 1. Read input traccc measurements from StoreGate --------------------------------
  auto inputTracccMeas = SG::makeHandle(m_inputMeasKey, ctx);
  ATH_CHECK(inputTracccMeas.isValid());
  ATH_MSG_DEBUG("Read traccc measurements from '"
                         << m_inputMeasKey.key() << "'");

  // ---- 2. Get traccc spacepoint formation alg ---------------------------------------------
  auto sp_alg = m_spAlgProviderTool->getAlgorithm(ctx);

  // ---- 3. Run traccc pixel spacepoint formation ---------------------------------------------

  traccc::edm::spacepoint_collection::buffer pixel_spacepoints_gpu_buffer = (*sp_alg)(*m_deviceDetector, *inputTracccMeas);

  ATH_MSG_DEBUG("Reconstructed " << sp_alg.copy().get_size(pixel_spacepoints_gpu_buffer) << " pixel spacepoints.");

  // ---- 4. Write output traccc spacepoints to StoreGate -------------------------
  auto outputTracccPixelSP = SG::makeHandle(m_outputPixelSPKey, ctx);
  ATH_CHECK(outputTracccPixelSP.record(
    std::make_unique<traccc::edm::spacepoint_collection::buffer>(
        std::move(pixel_spacepoints_gpu_buffer))));
  ATH_MSG_DEBUG("Wrote spacepoint buffer to '" << m_outputPixelSPKey.key() << "'");

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk
