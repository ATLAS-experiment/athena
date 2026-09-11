/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/IHGTD_ExpertTrackTimeAccessor.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date April, 2022
 * @brief Expert-only interface to retrieve an HGTD track time under a
 *  configurable working point.
 *
 *  WARNING: this interface exists for HGTD expert studies and internal
 *  validation only, and must not be scheduled in standard reconstruction
 *  jobs. Reconstruction and analysis code has to take the track time from the
 *  xAOD::TrackParticle EDM (hasValidTime(), time(), timeResolution())
 *  instead. The expert* methods below re-derive the time under tool-specific
 *  cuts and are named so that they cannot be mistaken for the EDM getters.
 */

#ifndef IHGTD_EXPERTTRACKTIMEACCESSOR_H
#define IHGTD_EXPERTTRACKTIMEACCESSOR_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/xAODTruthHelpers.h"

class MsgStream;

class IHGTD_ExpertTrackTimeAccessor : virtual public IAlgTool {

public:
  DeclareInterfaceID(IHGTD_ExpertTrackTimeAccessor, 1, 0);

  /**
   * @brief Not each track has a time defined. Before accessing the time, check
   *  with this function, if the track has a time assigned.
   *
   *  Expert use only, see the warning in the file header.
   */
  virtual bool
  expertHasTime(const xAOD::TrackParticle& track_particle) const = 0;

  /**
   * @brief Returns the reconstructed time assigned to the track. The time has
   *  already been corrected for time of flight relative to the reconstructed
   *  z0 position of the track.
   *
   *  Expert use only, see the warning in the file header.
   */
  virtual float
  expertTime(const xAOD::TrackParticle& track_particle) const = 0;

  /**
   * @brief Assuming gaussian behaviour for the track time resolution, this
   * function retuns the uncertainty on the track time that is defined by the
   * per hit resolutin and the number of hits associated to the given track
   * by the relation sigma_track = sigma_hit / sqrt(n_hits).
   *
   *  Expert use only, see the warning in the file header.
   */
  virtual float
  expertTimeRes(const xAOD::TrackParticle& track_particle) const = 0;

  /**
   * @brief Returns the fraction of the hits used for the track time that were
   * left by a primary particle.
   */
  virtual float
  fracPrimaryHits(const xAOD::TrackParticle& track_particle) const = 0;

  /**
   * @brief Returns the number of HGTD surfaces on which a hit from a primary
   * particle was expected for this track.
   */
  virtual int numberPotentialPrimaryHits(
      const xAOD::TrackParticle& track_particle) const = 0;

  /**
   * @brief Returns the time of the associated truth vertex, if one can be
   * found. A default value of -999 is returned if not.
   */
  virtual float getTruthTime(const xAOD::TruthParticle& truth_particle) const {
    float truth_time = -999.;
    auto production_vx = truth_particle.prodVtx();
    if (production_vx) {
      truth_time = production_vx->t() / Gaudi::Units::c_light;
    }
    return truth_time;
  };

  virtual float getTruthTime(const xAOD::TrackParticle& track_particle) const {
    float truth_time = -999.;
    auto truth_ptcl = xAOD::TruthHelpers::getTruthParticle(track_particle);
    if (truth_ptcl) {
      auto production_vx = truth_ptcl->prodVtx();
      if (production_vx) {
        truth_time = production_vx->t() / Gaudi::Units::c_light;
      }
    }
    return truth_time;
  }

  virtual float getTruthTime(const xAOD::TruthVertex& truth_vertex) const {
    return truth_vertex.t() / Gaudi::Units::c_light;
  }
};

#endif // IHGTD_EXPERTTRACKTIMEACCESSOR_H
