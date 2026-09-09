/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/IHGTD_TrackTimeAccessor.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date April, 2022
 * @brief
 */

#ifndef IHGTD_TRACKTIMEACCESSOR_H
#define IHGTD_TRACKTIMEACCESSOR_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/xAODTruthHelpers.h"

#include <memory>

class MsgStream;

class IHGTD_TrackTimeAccessor : virtual public IAlgTool {

public:
  DeclareInterfaceID(IHGTD_TrackTimeAccessor, 1, 0);

  /**
   * @brief Not each track has a time defined. Before accessing the time, check
   *  with this function, if the track has a time assigned.
   */
  virtual bool hasTime(const xAOD::TrackParticle& track_particle) = 0;

  /**
   * @brief Returns the reconstructed time assigned to the track. The time has
   *  already been corrected for time of flight relative to the reconstructed
   *  z0 position of the track.
   */
  virtual float time(const xAOD::TrackParticle& track_particle) = 0;

  /**
   * @brief Assuming gaussian behaviour for the track time resolution, this
   * function retuns the uncertainty on the track time that is defined by the
   * per hit resolutin and the number of hits associated to the given track
   * by the relation sigma_track = sigma_hit / sqrt(n_hits).
   */
  virtual float timeRes(const xAOD::TrackParticle& track_particle) = 0;

  /**
   * @brief Returns the number of hits in the HGTD surfaces that were assigned
   * to this track.
   */
  virtual int nHits(const xAOD::TrackParticle& track_particle) = 0;

  virtual float fracPrimaryHits(const xAOD::TrackParticle& track_particle) = 0;

  virtual int
  numberPotentialPrimaryHits(const xAOD::TrackParticle& track_particle) = 0;

  // virtual float getTruthTime(const xAOD::TruthVertex& truth_vertex) = 0;

  virtual std::string toolName() = 0;

  /**
   * @brief Returns the time of the associated truth vertex, if one can be
   * found. A default value of -999 is returned if not.
   */
  virtual float getTruthTime(const xAOD::TruthParticle& truth_particle) {
    float truth_time = -999.;
    auto production_vx = truth_particle.prodVtx();
    if (production_vx) {
      truth_time = production_vx->t() / Gaudi::Units::c_light;
    }
    return truth_time;
  };

  virtual float getTruthTime(const xAOD::TrackParticle& track_particle) {
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

  virtual float getTruthTime(const xAOD::TruthVertex& truth_vertex) {
    return truth_vertex.t() / Gaudi::Units::c_light;
  }
};

#endif // IHGTD_TRACKTIMEACCESSOR_H
