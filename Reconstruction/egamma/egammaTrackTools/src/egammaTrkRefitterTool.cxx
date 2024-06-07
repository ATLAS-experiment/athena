/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaTrkRefitterTool.h"
#include "xAODEgamma/Electron.h"
#include "xAODTracking/TrackParticle.h"

#include "TrkCaloCluster_OnTrack/CaloCluster_OnTrack.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkMaterialOnTrack/MaterialEffectsBase.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkTrack/TrackStateOnSurface.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDictDetDescr/IdDictManager.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"

#include <algorithm>
#include <cmath>
#include <vector>


egammaTrkRefitterTool::egammaTrkRefitterTool(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent)
  : AthAlgTool(type, name, parent)
  , m_ParticleHypothesis(Trk::electron)
  , m_idHelper(nullptr)
{
  declareInterface<IegammaTrkRefitterTool>(this);
}

StatusCode
egammaTrkRefitterTool::initialize()
{
  // Retrieve fitter
  ATH_CHECK(m_ITrackFitter.retrieve());

  ATH_CHECK(detStore()->retrieve(m_idHelper, "AtlasID"));

  // configure calo cluster on track builder (only if used)
  if (m_useClusterPosition) {
    ATH_CHECK(m_CCOTBuilder.retrieve());
  } else {
    m_CCOTBuilder.disable();
  }
  // Set the particle hypothesis to match the material effects
  m_ParticleHypothesis = Trk::ParticleSwitcher::particle[m_matEffects];
  return StatusCode::SUCCESS;
}

StatusCode
egammaTrkRefitterTool::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode
egammaTrkRefitterTool::refitTrack(const EventContext& ctx,
                                  const Trk::Track* track,
                                  Cache& cache) const
{
  cache.refittedTrack = nullptr;
  cache.refittedTrackPerigee = nullptr;
  cache.originalTrack = nullptr;
  cache.originalTrackPerigee = nullptr;
  if (!track) {
    return StatusCode::FAILURE;
  }
  // Set the pointer to the track
  cache.originalTrack = track;

  // Set pointer to the original perigee
  cache.originalTrackPerigee = cache.originalTrack->perigeeParameters();

  if (cache.originalTrackPerigee != nullptr) {
    double od0 = cache.originalTrackPerigee->parameters()[Trk::d0];
    double oz0 = cache.originalTrackPerigee->parameters()[Trk::z0];
    double ophi0 = cache.originalTrackPerigee->parameters()[Trk::phi0];
    double otheta = cache.originalTrackPerigee->parameters()[Trk::theta];
    double oqOverP = cache.originalTrackPerigee->parameters()[Trk::qOverP];
    ATH_MSG_DEBUG("Original parameters " << od0 << " " << oz0 << " " << ophi0
                                         << " " << otheta << " " << oqOverP
                                         << " " << 1 / oqOverP);
  } else {
    ATH_MSG_WARNING("Could not get Trk::Perigee of original track");
  }

  // Refit the track with the beam spot if desired otherwise just refit the
  // original track
  if (m_useClusterPosition) {
    egammaTrkRefitterTool::MeasurementsAndTrash collect =
      addPointsToTrack(ctx, cache.originalTrack, cache.electron);
    if (collect.m_measurements.size() > 4) {
      cache.refittedTrack =
        m_ITrackFitter->fit(ctx,
                            collect.m_measurements,
                            *cache.originalTrack->perigeeParameters(),
                            false,
                            m_ParticleHypothesis);
    } else {
      cache.refittedTrack = nullptr;
    }
  } else {
    std::vector<const Trk::MeasurementBase*> measurements =
      getIDHits(cache.originalTrack);
    if (measurements.size() > 4) {
      cache.refittedTrack =
        m_ITrackFitter->fit(ctx,
                            measurements,
                            *cache.originalTrack->perigeeParameters(),
                            false,
                            m_ParticleHypothesis);
    } else {
      cache.refittedTrack = nullptr;
    }
  }

  // Store refitted perigee pointers
  if (cache.refittedTrack) {
    cache.refittedTrackPerigee = cache.refittedTrack->perigeeParameters();
    if (cache.refittedTrackPerigee == nullptr) {
      ATH_MSG_WARNING("Could not get refitted Trk::Perigee");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

egammaTrkRefitterTool::MeasurementsAndTrash
egammaTrkRefitterTool::addPointsToTrack(const EventContext& ctx,
                                        const Trk::Track* track,
                                        const xAOD::Electron* eg) const
{
  egammaTrkRefitterTool::MeasurementsAndTrash collect{};
  /* The issue here is that some of the returned measurements are owned by
   * storegate some not. For the ones that are not put them in a vector of
   * unique_ptr which we will also return to the caller*/
  if (m_useClusterPosition && eg->caloCluster()) {
    int charge(0);
    if (track->perigeeParameters()) {
      charge = (int)track->perigeeParameters()->charge();
    }
    std::unique_ptr<const Trk::CaloCluster_OnTrack> ccot(
      m_CCOTBuilder->buildClusterOnTrack(ctx, eg->caloCluster(), charge));
    if (ccot != nullptr) {
      collect.m_trash.push_back(std::move(ccot));
      collect.m_measurements.push_back(collect.m_trash.back().get());
    }
  }
  std::vector<const Trk::MeasurementBase*> vecIDHits = getIDHits(track);
  std::vector<const Trk::MeasurementBase*>::const_iterator it =
      vecIDHits.begin();
  std::vector<const Trk::MeasurementBase*>::const_iterator itend =
      vecIDHits.end();
  // Fill the track , these are not trash
  for (; it != itend; ++it) {
    collect.m_measurements.push_back(*it);
  }
  return collect;
}

std::vector<const Trk::MeasurementBase*>
egammaTrkRefitterTool::getIDHits(const Trk::Track* track) const
{

  //store measurement to fit in the measurementSet
  std::vector<const Trk::MeasurementBase*> measurementSet;
  measurementSet.reserve(track->trackStateOnSurfaces()->size());

  for (const auto* tsos : *(track->trackStateOnSurfaces())) {
    if (!tsos) {
      ATH_MSG_WARNING(
          "This track contains an empty TrackStateOnSurface "
          "that won't be included in the fit");
      continue;
    }
    if (tsos->type(Trk::TrackStateOnSurface::Measurement) ||
        tsos->type(Trk::TrackStateOnSurface::Outlier)) {

      const auto* meas = tsos->measurementOnTrack();
      if (meas) {
        const Trk::RIO_OnTrack* rio = nullptr;
        if (meas->type(Trk::MeasurementBaseType::RIO_OnTrack)) {
          rio = static_cast<const Trk::RIO_OnTrack*>(meas);
        }
        if (rio != nullptr) {
          const Identifier& surfaceID = (rio->identify());
          if (m_idHelper->is_sct(surfaceID) ||
              m_idHelper->is_pixel(surfaceID) ||
              (!m_RemoveTRT && m_idHelper->is_trt(surfaceID))) {
            measurementSet.push_back(meas);
          }
        }
      }
    }
  }
  return measurementSet;
}

