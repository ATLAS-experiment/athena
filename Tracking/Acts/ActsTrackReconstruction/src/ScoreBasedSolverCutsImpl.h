/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_SCOREBASEDSOLVERCUTSIMPL_H
#define ACTSTRKFINDING_SCOREBASEDSOLVERCUTSIMPL_H 1

#include "Acts/AmbiguityResolution/ScoreBasedAmbiguityResolution.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "ActsEvent/TrackContainer.h"

// Athena
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "InDetRecToolInterfaces/IInDetEtaDependentCutsSvc.h"


// Gaudi includes
#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

namespace ActsTrk {
namespace ScoreBasedSolverCutsImpl {

using trackProxy_t = ActsTrk::MutableTrackContainer::ConstTrackProxy;
/** Adds summary information to the track container */
ActsTrk::MutableTrackContainer addSummaryInformation(
    ActsTrk::TrackContainer trackContainer);

/** Filter for tracks based on double holes */    
bool doubleHolesFilter(const trackProxy_t &track);

/** Score modifier for tracks based on innermost pixel layer hits */
void innermostPixelLayerHitsScore(const trackProxy_t &track, double &score);

/** Score modifier for tracks based on contributing pixel layers */
void ContribPixelLayersScore(const trackProxy_t &track, double &score);

/** Filter for tracks based on eta dependent cuts */
bool etaDependentCuts(const trackProxy_t &track, ServiceHandle<InDet::IInDetEtaDependentCutsSvc> etaDependentCutsSvc);

/** Hit selection for tracks based on pattern track hits */
void patternTrackHitSelection(
    const trackProxy_t &track, const trackProxy_t::ConstTrackStateProxy &ts,
    Acts::ScoreBasedAmbiguityResolution::TrackStateTypes &trackStateType);

}  // namespace ScoreBasedSolverCutsImpl
}  // namespace ActsTrk
#endif  // ACTSTRKFINDING_SCOREBASEDSOLVERCUTSIMPL_H