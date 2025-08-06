/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_SCOREBASEDSOLVERCUTSIMPL_H
#define ACTSTRKFINDING_SCOREBASEDSOLVERCUTSIMPL_H 1

#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "ActsEvent/TrackContainer.h"

namespace ActsTrk {
namespace ScoreBasedSolverCutsImpl {

using trackContainer_t = Acts::TrackContainer<Acts::VectorTrackContainer,
  Acts::VectorMultiTrajectory,
  Acts::detail::ValueHolder>;
using trackProxy_t = typename trackContainer_t::ConstTrackProxy;

/** Adds summary information to the track container */
trackContainer_t
  addSummaryInformation(const ActsTrk::TrackContainer& trackContainer);
  
/** Filter for tracks based on double holes */
void doubleHolesScore(const trackProxy_t &track, double &score);

/** Score modifier for tracks based on innermost pixel layer hits */
void innermostPixelLayerHitsScore(const trackProxy_t &track, double &score);

/** Score modifier for tracks based on number of contributing pixel layers */
void ContribPixelLayersScore(const trackProxy_t &track, double &score);

/** Score modifier for tracks based on number of SCT and pixel hits */
void nSCTPixelHitsScore(const trackProxy_t &track, double &score);

/** Filter for tracks based on eta dependent cuts */
bool etaDependentCuts(const trackProxy_t &track);

}  // namespace ScoreBasedSolverCutsImpl
}  // namespace ActsTrk
#endif  // ACTSTRKFINDING_SCOREBASEDSOLVERCUTSIMPL_H
