/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsToTrkFitterWrapTool.h"

#include "ActsCalibBase/CalibrationContext.h"
#include "ActsCalibBase/MeasurementCalibratorBase.h"
#include "ActsEvent/ParticleHypothesisEncoding.h"
#include "ActsEvent/TrackContainerUtils.h"
#include "ActsEvent/EnumConversion.h"


namespace ActsTrk {

StatusCode ActsToTrkFitterWrapTool::initialize() {
    ATH_CHECK(m_actsFitterTool.retrieve());
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_extrapolationTool.retrieve());
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_geometryConvTool.retrieve());

    return StatusCode::SUCCESS;
}

std::unique_ptr<Trk::Track> 
ActsToTrkFitterWrapTool::fit(const EventContext& ctx,
                             const Trk::Track& track,
                             const Trk::RunOutlierRemoval /*runOutlier*/,
                             const Trk::ParticleHypothesis hypothesis) const {

    bool useScaledCov = std::abs(m_option_seedCovarianceScale - 1.0) > Acts::s_epsilon;

    return reFitImpl(ctx, track, {}, hypothesis, useScaledCov);
}
std::unique_ptr<Trk::Track> 
ActsToTrkFitterWrapTool::fit(const EventContext& ctx,
                             const Trk::Track& track,
                             const Trk::MeasurementSet& measSet,
                             const Trk::RunOutlierRemoval /*runOutlier*/,
                             const Trk::ParticleHypothesis hypothesis) const {
    
    std::vector<Acts::SourceLink> sourceLinks;
    detail::MeasurementCalibratorBase::pack(measSet, sourceLinks);

    bool useScaledCov = false;

    return reFitImpl(ctx, track, sourceLinks, hypothesis, useScaledCov);
}

std::unique_ptr<Trk::Track> 
ActsToTrkFitterWrapTool::fit(const EventContext& ctx,
                             const Trk::Track& track,
                             const Trk::PrepRawDataSet& prepRawSet,
                             const Trk::RunOutlierRemoval /*runOutlier*/,
                             const Trk::ParticleHypothesis hypothesis) const {

    std::vector<Acts::SourceLink> sourceLinks;
    detail::MeasurementCalibratorBase::pack(prepRawSet, sourceLinks);

    bool useScaledCov = false;

    return reFitImpl(ctx, track, sourceLinks, hypothesis, useScaledCov);
}

std::unique_ptr<Trk::Track> 
ActsToTrkFitterWrapTool::fit(const EventContext& ctx,
                             const Trk::PrepRawDataSet& prepRawSet,
                             const Trk::TrackParameters& params,
                             const Trk::RunOutlierRemoval /*runOutlier*/,
                             const Trk::ParticleHypothesis /*hypothesis*/) const {
    
    std::vector<Acts::SourceLink> sourceLinks;
    detail::MeasurementCalibratorBase::pack(prepRawSet, sourceLinks);

    const auto initialParams = m_geometryConvTool->convertTrackParametersToActs(ctx, params); 
    
    return fitImpl(ctx, sourceLinks, initialParams);
}

std::unique_ptr<Trk::Track> 
ActsToTrkFitterWrapTool::fit(const EventContext& ctx,
                             const Trk::MeasurementSet& prepRawSet,
                             const Trk::TrackParameters& params,
                             const Trk::RunOutlierRemoval /*runOutlier*/,
                             const Trk::ParticleHypothesis /*hypothesis*/) const {

    std::vector<Acts::SourceLink> sourceLinks;
    detail::MeasurementCalibratorBase::pack(prepRawSet, sourceLinks);

    const auto initialParams = m_geometryConvTool->convertTrackParametersToActs(ctx, params); 

    return fitImpl(ctx, sourceLinks, initialParams);
}

std::unique_ptr<Trk::Track> 
ActsToTrkFitterWrapTool::fit(const EventContext& ctx,
                             const Trk::Track& track1,
                             const Trk::Track& track2,
                             const Trk::RunOutlierRemoval /*runOutlier*/,
                             const Trk::ParticleHypothesis hypothesis) const {
    ATH_MSG_VERBOSE ("--> enter ActsToTrkFitterWrapTool::fit(Track,Track,)"
        << "with Tracks from #1 = " << track1.info().dumpInfo()
                    << " and #2 = " << track2.info().dumpInfo());

    // protection, if empty track2
    if (!track2.measurementsOnTrack()) {
        ATH_MSG_DEBUG( "input #2 is empty try to fit track 1 alone" );
        return fit(ctx,track1);
    }

    // protection, if empty track1
    if (!track1.measurementsOnTrack()) {
        ATH_MSG_DEBUG( "input #1 is empty try to fit track 2 alone" );
        return fit(ctx,track2);
    }

    std::vector<Acts::SourceLink> trackSourceLinks2 = m_ATLASConverterTool->trkTrackToSourceLinks(track2);

    bool useScaledCov = std::abs(m_option_seedCovarianceScale - 1.0) > Acts::s_epsilon;

    return reFitImpl(ctx, track1, trackSourceLinks2, hypothesis, useScaledCov);
}

std::unique_ptr<Trk::Track> 
ActsToTrkFitterWrapTool::reFitImpl(const EventContext& ctx,
                                   const Trk::Track& track,
                                   const std::vector<Acts::SourceLink>& measColl,
                                   const Trk::ParticleHypothesis hypothesis,
                                   const bool useScaledCov) const {
    ATH_MSG_VERBOSE ("--> enter ActsToTrkFitterWrapTool::reFitImpl(Track,measColl,)"
       << " with Track from author = " << track.info().dumpInfo());

    // protection against not having measurements on the input track
    if (!track.measurementsOnTrack() || (track.measurementsOnTrack()->size() < 2 && measColl.empty())) {
        ATH_MSG_DEBUG("called to refit empty track or track with too little information, reject fit");
        return nullptr;
    }

    // protection against not having track parameters on the input track
    if (!track.trackParameters() || track.trackParameters()->empty()) {
        ATH_MSG_DEBUG("input fails to provide track parameters for seeding the fitter, reject fit");
        return nullptr;
    }

    std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(track);
    trackSourceLinks.insert(trackSourceLinks.end(), std::make_move_iterator(measColl.begin()), 
                            std::make_move_iterator(measColl.end()));

    auto initialParams = m_geometryConvTool->convertTrackParametersToActs(ctx, *track.perigeeParameters());
    if (useScaledCov) {
        // The covariance from already fitted track are too small and would result an incorect smoothing.
        // We scale up the input covaraiance to avoid this.
        Acts::BoundMatrix scaledCov = m_option_seedCovarianceScale * (*initialParams.covariance());
        initialParams = Acts::BoundTrackParameters(initialParams.referenceSurface().getSharedPtr(),
                                                   initialParams.parameters(),
                                                   scaledCov, ParticleHypothesis::convert(hypothesis));
    }

    return fitImpl(ctx, trackSourceLinks, initialParams);
}

std::unique_ptr<Trk::Track> 
ActsToTrkFitterWrapTool::fitImpl(const EventContext& ctx,
                               const std::vector<Acts::SourceLink>& measColl,
                               const Acts::BoundTrackParameters& params) const {
    ATH_MSG_VERBOSE ("--> enter ActsToTrkFitterWrapTool::fitImpl(measColl,)"
       << " with nMeas = " << measColl.size());

    // protection against not having measurements on the input track
    if (measColl.size() < 2) {
        ATH_MSG_DEBUG("called to refit empty measurement set or a measurement set with too little information, reject fit");
        return nullptr;
    }

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
  
    const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
    const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};

    std::unique_ptr<MutableTrackContainer> outTracks = 
        m_actsFitterTool->fit(measColl, params, tgContext, mfContext, calContext, pSurface.get());
    
    if (!outTracks || outTracks->size() == 0 ) {
        ATH_MSG_DEBUG("Acts fitter failed to refit the track, reject fit");
        return nullptr;
    }

    // We expect only one track to be returned
    for (auto trkProxy : *outTracks) {

        xAOD::TrackFitter fitterType = TrackContainerUtils::hasFitterType(trkProxy) 
            ? TrackContainerUtils::fitterType(trkProxy) 
            : xAOD::TrackFitter::Unknown;

        std::unique_ptr<Trk::Track> convertedTrack = 
            m_ATLASConverterTool->convertActsToTrk(ctx, trkProxy, toTrkFitterType(fitterType));

        if (convertedTrack) {
            return convertedTrack;
        }     
    }
    return nullptr;                            
}                     
}