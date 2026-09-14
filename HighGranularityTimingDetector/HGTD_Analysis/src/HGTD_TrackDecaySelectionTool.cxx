/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
*/

#include "HGTD_Analysis/HGTD_TrackDecaySelectionTool.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/xAODTruthHelpers.h"

HGTD_TrackDecaySelectionTool::HGTD_TrackDecaySelectionTool(const std::string& t,
                                                           const std::string& n,
                                                           const IInterface* p)
    : base_class(t, n, p) {}

StatusCode HGTD_TrackDecaySelectionTool::initialize() {
  return StatusCode::SUCCESS;
}

/** @brief Selects tracks that have a valid link to a truth particle and for
 * which the decay vertex is inside of a box in the z/r plane defined by
 * the selected min/max values.
 */
bool HGTD_TrackDecaySelectionTool::trackPassesSelection(
    const xAOD::TrackParticle* track_particle) const {

  const xAOD::TruthParticle* truth_particle =
      xAOD::TruthHelpers::getTruthParticle(*track_particle);

  if (truth_particle and truth_particle->hasProdVtx()) {

    const xAOD::TruthVertex* truth_vertex_prod = truth_particle->prodVtx();

    float prodVTX_r_abs = std::hypot(std::abs(truth_vertex_prod->x()),
                                     std::abs(truth_vertex_prod->y()));
    float prodVTX_z_abs = std::abs(truth_vertex_prod->z());

    if (prodVTX_r_abs < m_min_radius_prod or
        prodVTX_r_abs > m_max_radius_prod or prodVTX_z_abs < m_min_z_prod or
        prodVTX_z_abs > m_max_z_prod) {
      return false;
    }

    const xAOD::TruthVertex* truth_vertex_decay = truth_particle->decayVtx();
    if (not truth_vertex_decay) {
      return true;
    }

    float decayVTX_r_abs = std::hypot(std::abs(truth_vertex_decay->x()),
                                      std::abs(truth_vertex_decay->y()));
    float decayVTX_z_abs = std::abs(truth_vertex_decay->z());

    ATH_MSG_DEBUG("Track has a decay vertex at z = "
                  << decayVTX_z_abs << " and r = " << decayVTX_r_abs);

    return decayVTX_r_abs > m_min_radius_dec and
           decayVTX_r_abs < m_max_radius_dec and
           decayVTX_z_abs > m_min_z_dec and decayVTX_z_abs < m_max_z_dec;
  }

  return false;
}
