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

#include "src/detail/MeasurementIndex.h"
#include "src/detail/SharedHitCounter.h"
#include "src/detail/Definitions.h"

#include "ActsCalibrators/SourceLinkHash.h"

namespace ActsTrk {

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
     ATH_CHECK(m_tracksKey.initialize());
     ATH_CHECK(m_resolvedTracksKey.initialize());


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

    const ActsTrk::TrackContainer* trackContainer{};
    ATH_CHECK(SG::get(trackContainer, m_tracksKey, ctx));
    m_stat[kNInputTracks] += trackContainer->size();

    Acts::GreedyAmbiguityResolution::State state;

   
    m_ambi->computeInitialState(*trackContainer, state, &detail::sourceLinkHash,
                                &detail::sourceLinkEquality);
    m_ambi->resolve(state);

    ATH_MSG_DEBUG("Resolved to " << state.selectedTracks.size() << " tracks from "
                  << trackContainer->size());
    m_stat[kNResolvedTracks] += state.selectedTracks.size();


    // we start shortlisting the container
    Acts::VectorTrackContainer resolvedTrackBackend;
    Acts::VectorMultiTrajectory resolvedTrackStateBackend;
    detail::RecoTrackContainer resolvedTracksContainer(resolvedTrackBackend, resolvedTrackStateBackend);

    // need centralized function here
    resolvedTracksContainer.ensureDynamicColumns(*trackContainer);
        
    detail::MeasurementIndex measurementIndex;
    detail::SharedHitCounter sharedHits;

    std::size_t totalShared = 0;
    /** Validation of the sharedhit counts for the algorithm is only available if the
     *  source link type is xAOD::Uncalibrated measurement */

    for (auto iTrack : state.selectedTracks) {
      auto destProxy = resolvedTracksContainer.getTrack(resolvedTracksContainer.addTrack());
      destProxy.copyFrom(trackContainer->getTrack(state.trackTips.at(iTrack)));
      
      if (m_countSharedHits) {
        auto [nShared, nBadTrackMeasurements] = sharedHits.computeSharedHitsDynamic(destProxy, resolvedTracksContainer, measurementIndex);
        if (nBadTrackMeasurements > 0)
          ATH_MSG_ERROR("computeSharedHits: " << nBadTrackMeasurements << " track measurements not found in input track");
        totalShared += nShared;
      }
    }
    
    if (m_countSharedHits) {
      ATH_MSG_DEBUG("total number of shared hits = " << totalShared);
      m_stat[kNSharedHits] += totalShared;
    }

    // make const collection
    Acts::ConstVectorTrackContainer storableTrackBackend( std::move(resolvedTrackBackend) );
    Acts::ConstVectorMultiTrajectory storableTrackStateBackend( std::move(resolvedTrackStateBackend) );
    std::unique_ptr< ActsTrk::TrackContainer > storableTracksContainer = std::make_unique< ActsTrk::TrackContainer >( std::move(storableTrackBackend),
                                                                                                                      std::move(storableTrackStateBackend) );
    SG::WriteHandle resolvedTrackHandle{m_resolvedTracksKey, ctx };
    ATH_CHECK(resolvedTrackHandle.record( std::move(storableTracksContainer)));
    
    return StatusCode::SUCCESS;
  }

} // namespace
