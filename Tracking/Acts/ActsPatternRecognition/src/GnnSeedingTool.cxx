/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GnnSeedingTool.h"

// Other
#include <sstream>

namespace ActsTrk {

GnnSeedingTool::GnnSeedingTool(const std::string& type, const std::string& name,
                               const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode GnnSeedingTool::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << " ...");
  ATH_CHECK(m_gnnPipelineTool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode GnnSeedingTool::createSeeds(
    const EventContext& /*ctx*/,
    const std::vector<const xAOD::SpacePointContainer*>& spacePointCollections,
    const Eigen::Vector3f& /*beamSpotPos*/, float /*bFieldInZ*/,
    ActsTrk::SeedContainer& seedContainer) const {
  ATH_CHECK(m_gnnPipelineTool->buildSeed(spacePointCollections, seedContainer));
  ATH_MSG_DEBUG("GNN pipeline produced " << seedContainer.size() << " seeds");
  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
