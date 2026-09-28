/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccTrackConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "ActsGeometry/ActsDetectorElement.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include <detray/geometry/tracking_surface.hpp>
#include "Acts/EventData/TrackStateType.hpp"

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

    ATH_CHECK(detStore()->retrieve(m_hostDetector, m_hostDetectorObjectName.value()));

    // Build the ACTS-surface <-> ACTS-id lookup map once,
    // up front, rather than re-deriving them per event.
    // Possibly this will be built during ACTS->Detray conversion
    // Possibly this is an issue when the tracking geometry changes between events
    if (!m_trackingGeometrySvc.empty()) {
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        m_trackingGeometry = m_trackingGeometrySvc->trackingGeometry();
        m_trackingGeometry->visitSurfaces([&](const Acts::Surface* surface) {
            if (!surface) return;
            // m_allsurfaces.push_back(surface->getSharedPtr());
            const auto *actsElement = getActsDetectorElement(surface);
            if (!actsElement){
                ATH_MSG_DEBUG("Could not find matching Acts detector element for surface with geometryId " << surface->geometryId());
                return;
            } 
            if(dynamic_cast<const InDetDD::HGTD_DetectorElement*>(actsElement->upstreamDetectorElement()) != nullptr) { 
                // skip HGTD surfaces for now
                // currently no time info is being used in traccc
                ATH_MSG_VERBOSE("Found HGTD surface with geometryId " << surface->geometryId());
                return;
            }
            const auto *geoElement = actsElement->upstreamDetectorElement();
            const auto *detElem = dynamic_cast<const InDetDD::SiDetectorElement*>(geoElement);
            if (!geoElement || !detElem) {
                ATH_MSG_DEBUG("Could not find matching Athena silicon detector element for surface with geometryId " << surface->geometryId());
                return;
            }
    
            const Acts::GeometryIdentifier actsID = surface->geometryId();
            const auto [it, inserted] = m_actsSurfaceMap.insert({actsID, surface});
            if (!inserted) {
                ATH_MSG_WARNING("ACTS id " << actsID
                                        << " maps to two Acts surfaces: "
                                        << it->second->geometryId() << " and "
                                        << surface->geometryId());
            }
    
        });

    }else {
        ATH_MSG_FATAL("TrackingGeometrySvc is not configured, cannot translate Traccc tracks to ACTS tracks");
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

const Acts::Surface& TracccTrackConverterAlg::findActsSurface(
    const Acts::GeometryIdentifier& actsID) const
{
    const auto surfaceIt = m_actsSurfaceMap.find(actsID);
    if (surfaceIt == m_actsSurfaceMap.end()) {
        ATH_MSG_ERROR("No Acts surface known for Acts geometry id "
                      << actsID);
        throw std::domain_error("No Acts surface for this geometry id");
    }
    return *surfaceIt->second;
}

std::optional<Acts::BoundTrackParameters>
TracccTrackConverterAlg::convertGlobalToActsParameters(
    traccc::bound_track_parameters<traccc::default_algebra> const& trkParams)
    const
{
    if (trkParams.bound_local()[0] == 0 && trkParams.bound_local()[1] == 0 &&
        trkParams.phi() == 0 && trkParams.theta() == 0 &&
        trkParams.qop() == 0 && trkParams.time() == 0) {
        // traccc reports degenerate states like this; treat as "no
        // parameters" rather than as a real (but zeroed-out) track.
        return std::nullopt;
    }
 
    const auto& detrayDetector = m_hostDetector->as<traccc::itk_detector>();
    const detray::tracking_surface detray_surface{detrayDetector, trkParams.surface_link()};
    const auto geo_id = detray_surface.source();
    const Acts::GeometryIdentifier acts_geom_id{geo_id};

    const Acts::Surface& surface = findActsSurface(acts_geom_id);
 
    Acts::BoundVector params;
    params << trkParams.bound_local()[0], trkParams.bound_local()[1],
        trkParams.phi(), trkParams.theta(), trkParams.qop(), trkParams.time();

    // ---- sanity-check units ----
    const double qop = trkParams.qop();
    const double p   = (qop != 0) ? 1.0 / std::abs(qop) : 0.0;
    const double pT  = p * std::sin(trkParams.theta());
    ATH_MSG_DEBUG("Global params: d0=" << trkParams.bound_local()[0]
                  << " z0=" << trkParams.bound_local()[1]
                  << " phi=" << trkParams.phi()
                  << " theta=" << trkParams.theta()
                  << " qop=" << qop
                  << " -> |p|=" << p << " pT=" << pT
                  << " charge=" << (qop > 0 ? +1 : -1)
                  << " time=" << trkParams.time());    
 
    // Traccc covariance has no time uncertainty; Acts::BoundMatrix does
    // (it's eBoundSize x eBoundSize == 6x6). For now leave the time row/column at the
    // Identity() values set above and only fill the spatial+qop values.
    static constexpr unsigned int kAtlasCovSize = 5; // d0, z0, phi, theta, qop (no time)

    Acts::BoundMatrix cov = Acts::BoundMatrix::Identity();
    const auto& atlasCov = trkParams.covariance();
    for (unsigned i = 0; i < kAtlasCovSize; ++i) {
        for (unsigned j = 0; j < kAtlasCovSize; ++j) {
            cov(i, j) = atlasCov[i][j];
        }
    }
 
    return Acts::BoundTrackParameters(surface.getSharedPtr(), params, cov,
                                      Acts::ParticleHypothesis::pion());
}
 
template <typename state_t>
std::optional<Acts::BoundTrackParameters>
TracccTrackConverterAlg::convertSmoothedToActsParameters(
    traccc::edm::track_state<state_t> const& state) const
{
    const auto& atlasParam = state.smoothed_params();
 
    if (atlasParam.bound_local()[0] == 0 && atlasParam.bound_local()[1] == 0 &&
        atlasParam.phi() == 0 && atlasParam.theta() == 0 &&
        atlasParam.qop() == 0 && atlasParam.time() == 0) {
        // traccc reports degenerate states like this; treat as "no
        // parameters" rather than as a real (but zeroed-out) state.
        return std::nullopt;
    }
 
    const auto& detrayDetector = m_hostDetector->as<traccc::itk_detector>();
    const detray::tracking_surface detray_surface{detrayDetector, atlasParam.surface_link()};
    const auto geo_id = detray_surface.source();
    const Acts::GeometryIdentifier acts_geom_id{geo_id};

    const Acts::Surface& surface = findActsSurface(acts_geom_id);
 
    // d0/z0 come from the measurement's local position; the remaining bound
    // parameters come from the smoothed track state.
    Acts::BoundVector params;
    params << atlasParam.bound_local()[0], atlasParam.bound_local()[1],
        atlasParam.phi(), atlasParam.theta(), atlasParam.qop(),
        atlasParam.time();

    // ---- per-state momentum check for sanity ----
    const double qop = atlasParam.qop();
    const double p   = (qop != 0) ? 1.0 / std::abs(qop) : 0.0;
    const double pT  = p * std::sin(atlasParam.theta());
    ATH_MSG_DEBUG("Smoothed state: d0=" << atlasParam.bound_local()[0]
                  << " z0=" << atlasParam.bound_local()[1]
                  << " phi=" << atlasParam.phi()
                  << " theta=" << atlasParam.theta()
                  << " qop=" << qop
                  << " -> |p|=" << p << " pT=" << pT);    
 
    // Traccc covariance has no time uncertainty; Acts::BoundMatrix does
    // (it's eBoundSize x eBoundSize == 6x6). For now leave the time row/column at the
    // Identity() values set above and only fill the spatial+qop values.
    static constexpr unsigned int kAtlasCovSize = 5; // d0, z0, phi, theta, qop (no time)

    Acts::BoundMatrix cov = Acts::BoundMatrix::Identity();
    const auto& atlasCov = atlasParam.covariance();
    for (unsigned i = 0; i < kAtlasCovSize; ++i) {
        for (unsigned j = 0; j < kAtlasCovSize; ++j) {
            cov(i, j) = atlasCov[i][j];
        }
    }

    return Acts::BoundTrackParameters(surface.getSharedPtr(), params, cov,
                                      Acts::ParticleHypothesis::pion());
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

    if(stripMeasMap->size() == 0 || pixelMeasMap->size() == 0){
        ATH_MSG_FATAL("Maps relating traccc measurements to xAOD cluster containers are empty!!");
        return StatusCode::FAILURE;
    }

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
        
    traccc_track_container::const_device traccc_tracks(
        traccc_tracks_buffer);  

    ATH_MSG_DEBUG("Read " << traccc_tracks.tracks.size() << " tracks from device.");
    m_nTracksIn += traccc_tracks.tracks.size();

    // -- Write HOST resident ACTS track container ----
    Acts::VectorTrackContainer trackBackend;
    Acts::VectorMultiTrajectory trackStateBackend;
    ActsTrk::MutableTrackContainer trackContainer(std::move(trackBackend),
                                                  std::move(trackStateBackend));
    
    // Bookkeeping for debug summary
    unsigned nExcludedFitOutcome = 0;
    unsigned nExcludedNoState = 0;
    unsigned nExcludedBadNdf = 0;
    unsigned nExcludedWeirdState = 0;
    unsigned nExcludedWeirdGlobal = 0;

    for (std::size_t i = 0; i < traccc_tracks.tracks.size(); ++i) {

        const traccc::edm::track track = traccc_tracks.tracks.at(i);

        const auto fitOutcome = track.fit_outcome();
        if (fitOutcome == traccc::track_fit_outcome::FAILURE_NON_POSITIVE_NDF ||
            fitOutcome == traccc::track_fit_outcome::FAILURE_NOT_ALL_SMOOTHED ||
            fitOutcome == traccc::track_fit_outcome::UNKNOWN) {
            ATH_MSG_DEBUG("Skipping track " << i << ": fit outcome "
                                            << static_cast<int>(fitOutcome));
            ++nExcludedFitOutcome;
            continue;
        }
 
        if (track.constituent_links().empty()) {
            ++nExcludedNoState;
            continue;
        }
 
        if (track.ndf() >
                static_cast<float>(std::numeric_limits<unsigned int>::max()) ||
            track.ndf() <
                static_cast<float>(std::numeric_limits<unsigned int>::min())) {
            ++nExcludedBadNdf;
            continue;
        }

        TrackValidity track_validity = kValid;

        auto actsTrack = trackContainer.makeTrack();
        actsTrack.chi2() = track.chi2();
        actsTrack.nDoF() = track.ndf(); 

        Acts::TrackStatePropMask const state_mask =
            Acts::TrackStatePropMask::Smoothed;

        bool firstState = true;
 
        for (const auto [linkType, stateIdx] : track.constituent_links()) {
            
            assert(linkType == traccc::edm::track_constituent_link::track_state);
 
            const auto& state = traccc_tracks.states.at(stateIdx);
            const auto& meas_index = state.measurement_index();
 
            auto trackState = actsTrack.appendTrackState(state_mask);
            trackState.typeFlags().setIsMeasurement();

            const std::optional<Acts::BoundTrackParameters> smoothed =
                convertSmoothedToActsParameters(state);

            if (!smoothed) {
                ATH_MSG_DEBUG("Track " << i << ": degenerate smoothed state, "
                                             "dropping track");
                track_validity = TrackValidity::kInvalidState;
                break;
            }

            const xAOD::UncalibratedMeasurement* sourceLink =
                &makeSourceLink(meas_index, *pixelMeasMap, *stripMeasMap, *pixel_clusters, *strip_clusters);
            
            trackState.setUncalibratedSourceLink(ActsTrk::detail::xAODUncalibMeasCalibrator::pack(sourceLink));

            // traccc does not yet do backpropagation, so there is no true
            // reference surface (perigee) for the track. We use the surface
            // of the first measurement instead and rely on back-propagation
            // during the later Acts -> xAOD conversion step to find the real
            // perigee.
            if (firstState) {
                const std::optional<Acts::BoundTrackParameters> global =
                    convertGlobalToActsParameters(track.params());
                if (!global) {
                    ATH_MSG_DEBUG("Track "
                                  << i
                                  << ": degenerate global parameters, "
                                     "dropping track");
                    track_validity = TrackValidity::kInvalidGlobalParams;
                    break;
                }
                actsTrack.parameters() = global->parameters();
                actsTrack.covariance() = *global->covariance();
                actsTrack.setReferenceSurface(
                    global->referenceSurface().getSharedPtr());
                firstState = false;
            }

            try {
                trackState.setReferenceSurface(
                    smoothed->referenceSurface().getSharedPtr());
                trackState.smoothed() = smoothed->parameters();
                trackState.smoothedCovariance() = *smoothed->covariance();
            } catch (const std::exception& e) {
                ATH_MSG_ERROR("Track " << i << ": failed to set track state ("
                                       << e.what() << ")");
            }

        }

        if (track_validity == TrackValidity::kInvalidState) {
            ++nExcludedWeirdState;
            trackContainer.removeTrack(actsTrack.index());
        } else if (track_validity == TrackValidity::kInvalidGlobalParams) {
            ++nExcludedWeirdGlobal;
            trackContainer.removeTrack(actsTrack.index());
        } else {
            // sanity checks
            const auto& pars = actsTrack.parameters();
            const double qop = pars[Acts::eBoundQOverP];
            const double p   = (qop != 0) ? 1.0 / std::abs(qop) : 0.0;
            const double pT  = p * std::sin(pars[Acts::eBoundTheta]);
            ATH_MSG_DEBUG("Track " << i << " ACCEPTED: nHits="
                          << track.constituent_links().size()
                          << " chi2=" << track.chi2()
                          << " ndf=" << track.ndf()
                          << " |p|=" << p << " GeV pT=" << pT << " GeV");
        }

    }

    ATH_MSG_DEBUG("Converted " << trackContainer.size() << " tracks to " << m_outputTracksKey.key() );
    ATH_MSG_DEBUG("excluded: "
                            << nExcludedFitOutcome << " (fit outcome), "
                            << nExcludedNoState << " (no state), "
                            << nExcludedBadNdf << " (bad ndf), "
                            << nExcludedWeirdState << " (weird state), "
                            << nExcludedWeirdGlobal << " (weird global params)");
    
    m_nExcludedFitOutcome += nExcludedFitOutcome;
    m_nExcludedNoState += nExcludedNoState;
    m_nExcludedBadNdf += nExcludedBadNdf;
    m_nExcludedWeirdState += nExcludedWeirdState;
    m_nExcludedWeirdGlobal += nExcludedWeirdGlobal;
    
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

    ATH_MSG_DEBUG("Received " << m_nTracksIn << " tracks, wrote " << m_nTracksOut);
    ATH_MSG_DEBUG("Excluded: "
                            << m_nExcludedFitOutcome << " (fit outcome), "
                            << m_nExcludedNoState << " (no state), "
                            << m_nExcludedBadNdf << " (bad ndf), "
                            << m_nExcludedWeirdState << " (weird state), "
                            << m_nExcludedWeirdGlobal << " (weird global params)");

    ATH_MSG_DEBUG("Successfully finalized");
    return StatusCode::SUCCESS;
}

} // namespace ActsTrk
