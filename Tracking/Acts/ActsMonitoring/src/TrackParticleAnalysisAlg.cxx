/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackParticleAnalysisAlg.h"
#include "ActsEvent/TrackContainer.h"
#include "SGTools/CurrentEventStore.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace {
   int getSummaryValue(const xAOD::TrackParticle &track_particle, xAOD::SummaryType summary_type) {
      uint8_t tmp;
      return track_particle.summaryValue(tmp, summary_type) ? static_cast<int>(tmp) : -1;
   }
}


namespace ActsTrk {

  TrackParticleAnalysisAlg::TrackParticleAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator)
  {}

  StatusCode TrackParticleAnalysisAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << " ...");

    ATH_MSG_VERBOSE("Properties:");
    ATH_MSG_VERBOSE(m_tracksKey);

    ATH_CHECK(m_tracksKey.initialize());

    ATH_MSG_VERBOSE("Monitoring settings ...");
    ATH_MSG_VERBOSE(m_monGroupName);

    return AthMonitorAlgorithm::initialize();
  }

  StatusCode TrackParticleAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Filling Histograms for " << name() << " ... " );

    // Retrieve the tracks
    SG::ReadHandle<xAOD::TrackParticleContainer> trackParticlesHandle = SG::makeHandle(m_tracksKey, ctx);
    ATH_CHECK(trackParticlesHandle.isValid());
    const xAOD::TrackParticleContainer *track_particles = trackParticlesHandle.cptr();

    if (m_monitorTrackStateCounts) {
       for (const xAOD::TrackParticle *track_particle : *track_particles) {
	 ATH_CHECK( monitorTrackStateCounts(*track_particle) );
       }
    }
    auto monitor_pt = Monitored::Collection("pt", *track_particles,
                                            [](const xAOD::TrackParticle *tp){return static_cast<double>(tp->pt()); } );
    auto monitor_eta = Monitored::Collection("eta", *track_particles,
                                             [](const xAOD::TrackParticle *tp){return static_cast<double>(tp->eta()); } );

    auto monitor_pixelHits = Monitored::Collection("pixelHits", *track_particles,
                                                   [](const xAOD::TrackParticle *tp){
                                                      return getSummaryValue(*tp,xAOD::numberOfPixelHits); });

    auto monitor_innermostHits = Monitored::Collection("innermostHits", *track_particles,
                                                       [](const xAOD::TrackParticle *tp){
                                                          return getSummaryValue(*tp,xAOD::numberOfInnermostPixelLayerHits); });

    auto monitor_nextToInnermostHits = Monitored::Collection("nextToInnermostHits", *track_particles,
                                                       [](const xAOD::TrackParticle *tp){
                                                          return getSummaryValue(*tp,xAOD::numberOfNextToInnermostPixelLayerHits);});

    auto monitor_expectInnermost = Monitored::Collection("expectInnermostHit", *track_particles,
                                             [](const xAOD::TrackParticle *tp){
                                                return getSummaryValue(*tp,xAOD::expectInnermostPixelLayerHit); });

    auto monitor_expectNextToInnermost = Monitored::Collection("expectNextToInnermostHit", *track_particles,
                                             [](const xAOD::TrackParticle *tp){
                                                return getSummaryValue(*tp,xAOD::expectNextToInnermostPixelLayerHit); });

    fill(m_monGroupName.value(),monitor_pt,monitor_eta, monitor_pixelHits,  monitor_innermostHits, monitor_nextToInnermostHits,
         monitor_expectInnermost, monitor_expectNextToInnermost);
    return StatusCode::SUCCESS;
  }

  StatusCode TrackParticleAnalysisAlg::monitorTrackStateCounts(const xAOD::TrackParticle &track_particle) const {
     static const SG::AuxElement::ConstAccessor<ElementLink<ActsTrk::TrackContainer> > actsTrackLink("actsTrack");

     ElementLink<ActsTrk::TrackContainer> link_to_track = actsTrackLink(track_particle);
     ATH_CHECK(link_to_track.isValid());

     // to ensure that the code does not suggest something stupid (i.e. creating an unnecessary copy)
     static_assert( std::is_same<ElementLink<ActsTrk::TrackContainer>::ElementConstReference,
                    std::optional<ActsTrk::TrackContainer::ConstTrackProxy> >::value);
     std::optional<ActsTrk::TrackContainer::ConstTrackProxy> optional_track = *link_to_track;

     if ( not optional_track.has_value() ) {
       ATH_MSG_WARNING("Invalid track link for particle  " << track_particle.index());
       return StatusCode::SUCCESS;
     }
     
     ActsTrk::TrackContainer::ConstTrackProxy track = optional_track.value();
     std::array<uint8_t, Acts::toUnderlying(Acts::TrackStateFlag::NumFlags)+1> counts{};
     
     const ActsTrk::TrackContainer::ConstTrackProxy::IndexType
       lastMeasurementIndex = track.tipIndex();
     
     track.container().trackStateContainer().visitBackwards(lastMeasurementIndex,
							    [&counts] (const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) -> void
							    {
							      auto flags = state.typeFlags();
							      ++counts[Acts::toUnderlying(Acts::TrackStateFlag::NumFlags)];
							      
							      for (unsigned int flag_i=0; flag_i<Acts::toUnderlying(Acts::TrackStateFlag::NumFlags); ++flag_i) {
								if (flags.test(flag_i)) {
                  if (flag_i == Acts::toUnderlying(Acts::TrackStateFlag::IsHole)) {
								    if (!state.hasReferenceSurface() || !state.referenceSurface().isSensitive()) continue;
								  }
								  ++counts[flag_i];
								}
							      }
							    });

     auto monitor_states = Monitored::Scalar<int>("States",counts[Acts::toUnderlying(Acts::TrackStateFlag::NumFlags)]);
     auto monitor_measurement = Monitored::Scalar<int>("Measurements",counts[Acts::toUnderlying(Acts::TrackStateFlag::HasMeasurement)]);
     auto monitor_parameter = Monitored::Scalar<int>("Parameters",counts[Acts::toUnderlying(Acts::TrackStateFlag::HasParameters)]);
     auto monitor_outlier = Monitored::Scalar<int>("Outliers",counts[Acts::toUnderlying(Acts::TrackStateFlag::IsOutlier)]);
     auto monitor_hole = Monitored::Scalar<int>("Holes",counts[Acts::toUnderlying(Acts::TrackStateFlag::IsHole)]);
     auto monitor_material = Monitored::Scalar<int>("MaterialStates",counts[Acts::toUnderlying(Acts::TrackStateFlag::HasMaterial)]);
     auto monitor_sharedHit = Monitored::Scalar<int>("SharedHits",counts[Acts::toUnderlying(Acts::TrackStateFlag::IsSharedHit)]);

     fill(m_monGroupName.value(),
	  monitor_states, monitor_measurement, monitor_parameter, monitor_outlier, monitor_hole,
	  monitor_material, monitor_sharedHit);
     return StatusCode::SUCCESS;
  }

}
