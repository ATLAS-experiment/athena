/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccTrackConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "Acts/EventData/TrackStateType.hpp"
#include "ActsPlugins/Detray/DetraySurfaceIdMap.hpp"
#include "ActsPlugins/Traccc/TrackConversion.hpp"
#include "traccc/geometry/host_detector.hpp"

#include <limits>
#include <stdexcept>

namespace ActsTrk {

StatusCode TracccTrackConverterAlg::initialize()
{
    ATH_MSG_DEBUG("Initializing.");

    ATH_CHECK(m_hostMR.retrieve());
    ATH_CHECK(m_copy.retrieve());

    ATH_CHECK(m_inputPixelClustersKey.initialize());
    ATH_CHECK(m_inputStripClustersKey.initialize());
    ATH_CHECK(m_inputMeasToPixelSPKey.initialize());
    ATH_CHECK(m_inputMeasToStripClKey.initialize());

    ATH_CHECK(m_inputTracksKey.initialize());
    ATH_CHECK(m_outputTracksKey.initialize());

    // The Detray geometry and the Detray -> ACTS surface map are built by the
    // ACTS Detray plugin in the TrackingGeometrySvc
    ATH_CHECK(m_trackingGeometrySvc.retrieve());
    m_trackingGeometry = m_trackingGeometrySvc->trackingGeometry();
    m_hostDetector = m_trackingGeometrySvc->detrayGeometry();
    if (!m_hostDetector || !m_trackingGeometrySvc->detraySurfaceIdMap()) {
        ATH_MSG_FATAL("No Detray geometry available from the TrackingGeometrySvc, "
                      "cannot translate Traccc tracks to ACTS tracks. "
                      "Set its BuildDetrayGeometry property to True.");
        return StatusCode::FAILURE;
    }

    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(
        ActsTrk::prefixFromTrackContainerName(
            m_outputTracksKey.key())));

    ATH_MSG_DEBUG("Successfully initialized");
    return StatusCode::SUCCESS;
}


const xAOD::UncalibratedMeasurement& TracccTrackConverterAlg::makeSourceLink(
    const unsigned int& meas_index,
    std::span<const unsigned int> pixelMap,
    std::span<const unsigned int> stripMap,
    const xAOD::PixelClusterContainer& pixel_clusters,
    const xAOD::StripClusterContainer& strip_clusters) const
{
    const unsigned int pixel_index = (pixelMap)[meas_index];
    if(pixel_index != std::numeric_limits<unsigned int>::max()){
    
        return *pixel_clusters.at(pixel_index);
    }

    const unsigned int strip_index = (stripMap)[meas_index];
    if(strip_index != std::numeric_limits<unsigned int>::max()){
        return *strip_clusters.at(strip_index);
    }

    ATH_MSG_FATAL("Measurement index could not be found!!!");
    throw std::domain_error("No measurement index match found in xAOD map");
    
}

StatusCode TracccTrackConverterAlg::execute(const EventContext& ctx) const
{
    // ---- Retrieve HOST RESIDENT clusters (always needed) ----
    // These are made during the TracccMeasurementConverterAlg 
    auto pixel_clusters = SG::makeHandle(m_inputPixelClustersKey, ctx);
    ATH_CHECK(pixel_clusters.isValid());

    auto strip_clusters = SG::makeHandle(m_inputStripClustersKey, ctx);
    ATH_CHECK(strip_clusters.isValid());

    // ---- Retrieve mapping from traccc measurement index to pixel cluster/spacepoint index (always needed) ----
    // Because the SPs were created from xAOD pixel clusters on the host, the same traccc measurement index 
    // points to the xAOD pixel cluster and xAOD spacepoint in their respective containers
    auto pixelMeasMap = SG::makeHandle(m_inputMeasToPixelSPKey, ctx);
    ATH_CHECK(pixelMeasMap.isValid());

    auto stripMeasMap = SG::makeHandle(m_inputMeasToStripClKey, ctx);
    ATH_CHECK(stripMeasMap.isValid());


    // ---- Retrieve DEVICE resident traccc tracks and track states ----
    auto tracks = SG::makeHandle(m_inputTracksKey, ctx);
    ATH_CHECK(tracks.isValid());

    auto copy = m_copy->copy(ctx);

    traccc_track_container::buffer traccc_tracks_buffer;
    traccc_tracks_buffer.tracks =
        copy->to(tracks->tracks, m_hostMR->mr(),
            nullptr, vecmem::copy::type::device_to_host);
    traccc_tracks_buffer.states =
        copy->to(tracks->states, m_hostMR->mr(),
            nullptr, vecmem::copy::type::device_to_host);
        
    // -- Write HOST resident ACTS track container ----
    Acts::VectorTrackContainer trackBackend;
    Acts::VectorMultiTrajectory trackStateBackend;
    ActsTrk::MutableTrackContainer trackContainer(std::move(trackBackend),
                                                  std::move(trackStateBackend));

    // The traccc measurement index points to the xAOD pixel or strip cluster
    auto makeActsSourceLink = [&](unsigned int meas_index,
                                  const Acts::Surface& /*surface*/) {
        const xAOD::UncalibratedMeasurement* measurement =
            &makeSourceLink(meas_index, *pixelMeasMap, *stripMeasMap,
                            *pixel_clusters, *strip_clusters);
        return ActsTrk::detail::xAODUncalibMeasCalibrator::pack(measurement);
    };

    // traccc does not yet do backpropagation, so there is no true reference
    // surface (perigee) for the track. The track parameters are on the surface
    // of the first measurement and back-propagation during the later
    // Acts -> xAOD conversion step finds the real perigee.
    const ActsPlugins::TracccSourceLinkCreator sourceLinkCreator{
        makeActsSourceLink};
    const ActsPlugins::TracccTrackConversionStatistics stats =
        ActsPlugins::convertTracksFromTraccc(
            traccc_track_container::const_view(traccc_tracks_buffer),
            m_hostDetector->as<traccc::itk_detector>(), *m_trackingGeometry,
            sourceLinkCreator, trackContainer);

    ATH_MSG_DEBUG("Traccc track conversion to " << m_outputTracksKey.key()
                  << ": " << stats);

    m_nTracksIn += stats.nInput;
    m_nExcludedFitOutcome += stats.nFailedFit;
    m_nExcludedMissingSurface += stats.nMissingSurface;
    m_nExcludedDegenerate += stats.nDegenerate;
    m_nTracksOut += trackContainer.size();

    Acts::ConstVectorTrackContainer constTrackBackend(
        std::move(trackContainer.container()));
    Acts::ConstVectorMultiTrajectory constTrackStateBackend(
        std::move(trackContainer.trackStateContainer()));
    auto constTrackContainer = std::make_unique<ActsTrk::TrackContainer>(
        std::move(constTrackBackend), std::move(constTrackStateBackend));
 
    SG::WriteHandle<ActsTrk::TrackContainer> outputHandle(
        m_outputTracksKey, ctx);
    ATH_CHECK(outputHandle.record(std::move(constTrackContainer)));                        

    return StatusCode::SUCCESS;
}

StatusCode TracccTrackConverterAlg::finalize()
{
    ATH_MSG_DEBUG("Finalizing.");

    ATH_MSG_INFO("Received " << m_nTracksIn << " tracks, wrote " << m_nTracksOut);
    ATH_MSG_INFO("Excluded: "
                            << m_nExcludedFitOutcome << " (fit outcome), "
                            << m_nExcludedMissingSurface << " (missing surface), "
                            << m_nExcludedDegenerate << " (degenerate parameters)");

    ATH_MSG_DEBUG("Successfully finalized");
    return StatusCode::SUCCESS;
}

} // namespace ActsTrk
