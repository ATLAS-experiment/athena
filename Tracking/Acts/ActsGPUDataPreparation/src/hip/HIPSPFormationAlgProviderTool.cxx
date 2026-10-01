/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "HIPSPFormationAlgProviderTool.h"

// Athena Acts include(s).
#include "ActsInterop/Logger.h"

// Traccc include(s).
#include "traccc/hip/seeding/silicon_pixel_spacepoint_formation_algorithm.hpp"

namespace ActsTrk {

StatusCode HIPSPFormationAlgProviderTool::initialize() {

  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

DeviceAlgorithmT<traccc::device::silicon_pixel_spacepoint_formation_algorithm>
HIPSPFormationAlgProviderTool::getAlgorithm(const EventContext& ctx) const {

  ATH_MSG_VERBOSE(
      "Constructing HIP traccc pixel spacepoint formation algorithm");

  auto copy = m_copy->copy(ctx);

  return {copy,
          std::make_shared<
              traccc::hip::silicon_pixel_spacepoint_formation_algorithm>(
              traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()}, *copy,
              traccc::hip::stream_wrapper{m_streamTool->stream(ctx)},
              makeActsAthenaLogger(this, "TracccSPFormationHIP"))};
}

}  // namespace ActsTrk
