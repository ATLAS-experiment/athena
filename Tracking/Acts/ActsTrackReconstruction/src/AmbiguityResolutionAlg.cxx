/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AmbiguityResolutionAlg.h"

// Athena
#include "AthenaMonitoringKernel/Monitored.h"

// ACTS
#include "Acts/Definitions/Units.hpp"

#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#include "Acts/EventData/MultiTrajectoryHelpers.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Utilities/Logger.hpp"

#include "ActsInterop/Logger.h"
#include "ActsInterop/TableUtils.h"
#include "ActsGeometry/ATLASSourceLink.h"

#include "src/detail/MeasurementIndex.h"
#include "src/detail/SharedHitCounter.h"

namespace {
   std::size_t sourceLinkHash(const Acts::SourceLink& slink) {
      const ActsTrk::ATLASUncalibSourceLink &atlasSourceLink = slink.get<ActsTrk::ATLASUncalibSourceLink>();
      const xAOD::UncalibratedMeasurement &uncalibMeas = ActsTrk::getUncalibratedMeasurement(atlasSourceLink);
      return uncalibMeas.identifier();
   }

   bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b) {
      const xAOD::UncalibratedMeasurement &uncalibMeas_a = ActsTrk::getUncalibratedMeasurement(a.get<ActsTrk::ATLASUncalibSourceLink>());
      const xAOD::UncalibratedMeasurement &uncalibMeas_b = ActsTrk::getUncalibratedMeasurement(b.get<ActsTrk::ATLASUncalibSourceLink>());

      return uncalibMeas_a.identifier() == uncalibMeas_b.identifier();
   }
}

namespace ActsTrk
{

  AmbiguityResolutionAlg::AmbiguityResolutionAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode AmbiguityResolutionAlg::initialize()
  {
     {
        Acts::GreedyAmbiguityResolution::Config cfg;
        cfg.maximumSharedHits = m_maximumSharedHits;
        cfg.maximumIterations = m_maximumIterations;
        cfg.nMeasurementsMin = m_nMeasurementsMin;
        m_ambi = std::make_unique<Acts::GreedyAmbiguityResolution>(std::move(cfg), makeActsAthenaLogger(this, "Acts")  ) ;
        assert( m_ambi );
     }

     ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
     ATH_CHECK(m_trackingGeometryTool.retrieve());
     ATH_CHECK(m_tracksKey.initialize());
     ATH_CHECK(m_resolvedTracksKey.initialize());
     ATH_CHECK(m_resolvedTracksBackendHandles.initialize(ActsTrk::prefixFromTrackContainerName(m_resolvedTracksKey.key()))); //TODO choose prefix related to the output tracks name
     return StatusCode::SUCCESS;
  }

  StatusCode AmbiguityResolutionAlg::finalize() {
    ATH_MSG_INFO("Ambiguity Resolution statistics" << std::endl
                 << makeTable(m_stat,
                              std::array<std::string, kNStat>{
                                  "Input tracks",
                                  "Resolved tracks",
                                  "Total shared hits"}).columnWidth(10));
    return StatusCode::SUCCESS;
 }

  StatusCode AmbiguityResolutionAlg::execute(const EventContext &ctx) const
  {
    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto mon = Monitored::Group( m_monTool, timer );

    SG::ReadHandle<ActsTrk::TrackContainer> trackHandle = SG::makeHandle(m_tracksKey, ctx);
    ATH_CHECK(trackHandle.isValid());
    m_stat[kNInputTracks] += trackHandle->size();

    Acts::GreedyAmbiguityResolution::State state;
    m_ambi->computeInitialState(*trackHandle, state, &sourceLinkHash,
                                &sourceLinkEquality);

    m_ambi->resolve(state);

    ATH_MSG_DEBUG("Resolved to " << state.selectedTracks.size() << " tracks from "
                  << trackHandle->size());
    m_stat[kNResolvedTracks] += state.selectedTracks.size();

    ActsTrk::MutableTrackContainer solvedTracks;
    solvedTracks.ensureDynamicColumns(*trackHandle);

    detail::MeasurementIndex measurementIndex;
    detail::SharedHitCounter sharedHits;

    std::size_t totalShared = 0;
    for (auto iTrack : state.selectedTracks) {
       auto destProxy = solvedTracks.getTrack(solvedTracks.addTrack());
       destProxy.copyFrom(trackHandle->getTrack(state.trackTips.at(iTrack)));
       if (m_countSharedHits) {
        auto [nShared, nBadTrackMeasurements] = sharedHits.computeSharedHitsDynamic(destProxy, solvedTracks, measurementIndex);
        if (nBadTrackMeasurements > 0)
          ATH_MSG_ERROR("computeSharedHits: " << nBadTrackMeasurements << " track measurements not found in input track");
        totalShared += nShared;
      }
    }
    if (m_countSharedHits) {
      ATH_MSG_DEBUG("total number of shared hits = " << totalShared);
      m_stat[kNSharedHits] += totalShared;
    }

    std::unique_ptr<ActsTrk::TrackContainer> outputTracks = m_resolvedTracksBackendHandles.moveToConst(std::move(solvedTracks), 
       m_trackingGeometryTool->getGeometryContext(ctx).context(), ctx);
    SG::WriteHandle<ActsTrk::TrackContainer> resolvedTrackHandle(m_resolvedTracksKey, ctx);

    if (resolvedTrackHandle.record( std::move(outputTracks)).isFailure()) {
       ATH_MSG_ERROR("Failed to record resolved ACTS tracks with key " << m_resolvedTracksKey.key() );
       return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

} // namespace
