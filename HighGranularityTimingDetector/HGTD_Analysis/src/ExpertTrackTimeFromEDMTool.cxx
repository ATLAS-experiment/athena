/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/ExpertTrackTimeFromEDMTool.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date Feb, 2023
 * @brief
 */

#include "ExpertTrackTimeFromEDMTool.h"

using namespace HGTD;

ExpertTrackTimeFromEDMTool::ExpertTrackTimeFromEDMTool(const std::string& t,
                                                       const std::string& n,
                                                       const IInterface* p)
    : base_class(t, n, p) {}

StatusCode ExpertTrackTimeFromEDMTool::initialize() {
  ATH_CHECK(AthAlgTool::initialize());
  return StatusCode::SUCCESS;
}

bool ExpertTrackTimeFromEDMTool::expertHasTime(
    const xAOD::TrackParticle& /*track_particle*/) const {
  return true;
}

float ExpertTrackTimeFromEDMTool::expertTime(
    const xAOD::TrackParticle& track_particle) const {
  return track_particle.time();
}

float ExpertTrackTimeFromEDMTool::expertTimeRes(
    const xAOD::TrackParticle& /*track_particle*/) const {
  return -1;
}

float ExpertTrackTimeFromEDMTool::fracPrimaryHits(
    const xAOD::TrackParticle& /*track_particle*/) const {
  return -1;
}

int ExpertTrackTimeFromEDMTool::numberPotentialPrimaryHits(
    const xAOD::TrackParticle& /*track_particle*/) const {
  return -1;
}
