/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MlMsTrackSeeder.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"

#include <map>
#include <utility>
#include <vector>

namespace MuonR4 {

namespace {

struct MlComponent {
  std::vector<const xAOD::MuonSegment*> segments;
  std::vector<const xAOD::MuonSegment*> anchors;
};

}  // namespace

StatusCode MlMsTrackSeeder::initialize() {
  ATH_CHECK(m_seedParameterEstimator.retrieve());
  ATH_CHECK(m_segmentKey.initialize());
  ATH_CHECK(m_candidateDecorKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode MlMsTrackSeeder::findTrackSeeds(
    const EventContext& ctx,
    std::vector<MsTrackSeed>& outSeeds) const {
  const xAOD::MuonSegmentContainer* segments{nullptr};
  ATH_CHECK(SG::get(segments, m_segmentKey, ctx));

  // The decoration payload is [componentId, isSeedAnchor].
  SG::ReadDecorHandle<xAOD::MuonSegmentContainer, std::vector<unsigned int>>
      componentAcc{m_candidateDecorKey, ctx};
  if (!componentAcc.isAvailable()) {
    ATH_MSG_DEBUG("No ML candidate decoration found");
    return StatusCode::SUCCESS;
  }

  std::map<unsigned int, MlComponent> components;

  for (const xAOD::MuonSegment* segment : *segments) {
    if (!segment) continue;
    const std::vector<unsigned int>& payload = componentAcc(*segment);
    if (payload.size() != 2 || payload[0] == 0) continue;
    MlComponent& component = components[payload[0]];
    component.segments.push_back(segment);
    if (payload[1] != 0) component.anchors.push_back(segment);
  }

  std::size_t acceptedComponents = 0;
  std::size_t constructedSeeds = 0;
  for (const auto& [componentId, component] : components) {
    if (component.segments.size() < m_minSegmentsPerCandidate.value() ||
        component.anchors.empty()) {
      continue;
    }
    ++acceptedComponents;

    for (const xAOD::MuonSegment* anchor : component.anchors) {
      const MsTrackSeed::Location location =
          Muon::MuonStationIndex::isBarrel(anchor->chamberIndex())
              ? MsTrackSeed::Location::Barrel
              : MsTrackSeed::Location::Endcap;
      MsTrackSeed seed{location, ExpandedSector{anchor->position().phi()}};
      for (const xAOD::MuonSegment* segment : component.segments) {
        seed.addSegment(segment);
      }
      seed.setPosition(anchor->position());
      outSeeds.push_back(std::move(seed));
      ++constructedSeeds;
    }
    ATH_MSG_VERBOSE("Constructed " << component.anchors.size()
                    << " seed(s) from ML component " << componentId
                    << " with " << component.segments.size()
                    << " segments");
  }

  ATH_MSG_DEBUG("Constructed " << constructedSeeds << " seed(s) from "
                << acceptedComponents << "/" << components.size()
                << " decorated ML component(s)");
  return StatusCode::SUCCESS;
}

Acts::Result<Acts::BoundTrackParameters>
MlMsTrackSeeder::estimateStartParameters(
    const EventContext& ctx,
    const MsTrackSeed& seed) const {
  return m_seedParameterEstimator->estimateStartParameters(ctx, seed);
}

}  // namespace MuonR4
