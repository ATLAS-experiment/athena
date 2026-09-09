/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/TrackTimeAccTool2.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date Feb, 2023
 * @brief
 */

#include "TrackTimeAccTool2.h"

#include <numeric>

using namespace HGTD;

TrackTimeAccTool2::TrackTimeAccTool2(const std::string& t, const std::string& n,
                                     const IInterface* p)
    : AthAlgTool(t, n, p) {}

StatusCode TrackTimeAccTool2::initialize() {
  StatusCode sc = AthAlgTool::initialize();
  return sc;
}

bool TrackTimeAccTool2::hasTime(const xAOD::TrackParticle& /*track_particle*/) {
  return true;
}

float TrackTimeAccTool2::time(const xAOD::TrackParticle& track_particle) {
  return track_particle.time();
}

float TrackTimeAccTool2::timeRes(
    const xAOD::TrackParticle& /*track_particle*/) {
  return -1;
}

int TrackTimeAccTool2::nHits(const xAOD::TrackParticle& /*track_particle*/) {
  return -1;
}

int TrackTimeAccTool2::nPrimaryHits(
    const xAOD::TrackParticle& /*track_particle*/) {
  return -1;
}

float TrackTimeAccTool2::fracPrimaryHits(
    const xAOD::TrackParticle& /*track_particle*/) {
  return -1;
}

int TrackTimeAccTool2::numberPotentialPrimaryHits(
    const xAOD::TrackParticle& /*track_particle*/) {
  return -1;
}
