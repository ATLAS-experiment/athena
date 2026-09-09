/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
*/

#include "HGTD_Analysis/HGTD_AllTracksSelectionTool.h"

HGTD_AllTracksSelectionTool::HGTD_AllTracksSelectionTool(const std::string& t,
                                                         const std::string& n,
                                                         const IInterface* p)
    : base_class(t, n, p) {}

StatusCode HGTD_AllTracksSelectionTool::initialize() {
  return StatusCode::SUCCESS;
}

bool HGTD_AllTracksSelectionTool::trackPassesSelection(
    const xAOD::TrackParticle* track_particle) const {
  bool passes_cuts = track_particle->pt() / 1.e3 > 1.;
  passes_cuts &= std::abs(track_particle->eta()) > 2.4;
  return passes_cuts;
}
