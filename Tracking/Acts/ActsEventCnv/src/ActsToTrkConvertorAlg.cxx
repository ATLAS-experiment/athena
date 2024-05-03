/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/ActsToTrkConvertorAlg.h"

#include "ActsEvent/TrackParameters.h"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"

// get Athena SiDetectorElement from Acts surface
static const InDetDD::SiDetectorElement *actsToDetElem(const Acts::Surface &surface)
{
  const auto *actsElement = dynamic_cast<const ActsDetectorElement *>(surface.associatedDetectorElement());
  if (!actsElement)
  {
    return nullptr;
  }
  return dynamic_cast<const InDetDD::SiDetectorElement *>(actsElement->upstreamDetectorElement());
}

namespace ActsTrk
{

  ActsToTrkConvertorAlg::ActsToTrkConvertorAlg(const std::string &name,
					       ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode ActsToTrkConvertorAlg::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << " ...");

    ATH_CHECK(m_tracksContainerKey.initialize());
    ATH_CHECK(m_tracksKey.initialize());

    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_trkSummaryTool.retrieve());

    if (not m_RotCreatorTool.empty())
    {
      ATH_MSG_DEBUG("RotCreatorTool will be used");
      ATH_CHECK(m_RotCreatorTool.retrieve());
    }

    return StatusCode::SUCCESS;
  }

  StatusCode ActsToTrkConvertorAlg::execute(const EventContext &ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ...");

    // I/O
    ATH_MSG_DEBUG("Retrieving input track collection '" << m_tracksContainerKey.key() << "' ...");
    SG::ReadHandle<ActsTrk::TrackContainer> inputTracksHandle = SG::makeHandle(m_tracksContainerKey, ctx);
    ATH_CHECK(inputTracksHandle.isValid());
    const ActsTrk::TrackContainer *inputTracks = inputTracksHandle.cptr();

    std::unique_ptr<::TrackCollection> trackCollection = std::make_unique<::TrackCollection>();

    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    ATH_CHECK(makeTracks(ctx, tgContext, *inputTracks, *trackCollection));

    SG::WriteHandle<::TrackCollection> outputTrackHandle = SG::makeHandle(m_tracksKey, ctx);
    ATH_MSG_DEBUG("Output Tracks Collection `" << m_tracksKey.key() << "` created ...");
    ATH_CHECK(outputTrackHandle.record(std::move(trackCollection)));

    return StatusCode::SUCCESS;
  }

  StatusCode ActsToTrkConvertorAlg::makeTracks(const EventContext &ctx,
					       const Acts::GeometryContext &tgContext,
                                           const ActsTrk::TrackContainer &tracks,
                                           ::TrackCollection &tracksContainer) const
  {

    for (const typename ActsTrk::TrackContainer::ConstTrackProxy track : tracks)
    {
      const auto lastMeasurementIndex = track.tipIndex();
      auto finalTrajectory = std::make_unique<Trk::TrackStates>();
      // initialise the number of dead Pixel and Acts strip
      int numberOfDeadPixel = 0;
      int numberOfDeadSCT = 0;

      Acts::ParticleHypothesis hypothesis = track.particleHypothesis();

      std::vector<std::unique_ptr<const Acts::BoundTrackParameters>> actsSmoothedParam;
      tracks.trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [this, &tgContext, &track, &finalTrajectory, &actsSmoothedParam, &numberOfDeadPixel, &numberOfDeadSCT](const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) -> void
          {
            // First only consider states with an associated detector element
            if (!state.hasReferenceSurface() || !state.referenceSurface().associatedDetectorElement())
            {
              return;
            }

            auto flag = state.typeFlags();
            std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
            std::unique_ptr<Trk::TrackParameters> parm;

            // State is a hole (no associated measurement), use predicted parameters
            if (flag.test(Acts::TrackStateFlag::HoleFlag))
            {
              const Acts::BoundTrackParameters actsParam = track.createParametersFromState(state);
              parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
              auto boundaryCheck = m_boundaryCheckTool->boundaryCheck(*parm);

              // Check if this is a hole, a dead sensors or a state outside the sensor boundary
              if (boundaryCheck == Trk::BoundaryCheckResult::DeadElement)
              {
                auto *detElem = actsToDetElem(state.referenceSurface());
                if (!detElem)
                {
                  return;
                }
                if (detElem->isPixel())
                {
                  ++numberOfDeadPixel;
                }
                else if (detElem->isSCT())
                {
                  ++numberOfDeadSCT;
                }
                // Dead sensors states are not stored
                return;
              }
              else if (boundaryCheck != Trk::BoundaryCheckResult::Candidate)
              {
                return;
              }
              typePattern.set(Trk::TrackStateOnSurface::Hole);
            }
            // The state was tagged as an outlier or (TODO!) was missed in the reverse filtering, use filtered parameters
            else if (flag.test(Acts::TrackStateFlag::OutlierFlag))
            {
              const Acts::BoundTrackParameters actsParam = track.createParametersFromState(state);
              parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
              typePattern.set(Trk::TrackStateOnSurface::Outlier);
            }
            // The state is a measurement state, use smoothed parameters
            else
            {
              const Acts::BoundTrackParameters actsParam = track.createParametersFromState(state);

              // is it really necessary to keep our own copy of all the smoothed parameters?
              actsSmoothedParam.push_back(std::make_unique<const Acts::BoundTrackParameters>(Acts::BoundTrackParameters(actsParam)));
              parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsParam, tgContext);
              typePattern.set(Trk::TrackStateOnSurface::Measurement);
            }

            std::unique_ptr<Trk::MeasurementBase> measState;
            if (state.hasUncalibratedSourceLink())
            {
              try {
                auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
                assert( sl.isValid() && *sl != nullptr);
                const xAOD::UncalibratedMeasurement &uncalibMeas = **sl;
                measState = makeRIO_OnTrack(uncalibMeas, *parm);
                ATH_MSG_DEBUG("Successfully used ATLASUncalibratedSourceLink");
              } catch ( const std::bad_any_cast& ){
                ATH_MSG_DEBUG("Not an ATLASUncalibSourceLink, trying ATLASSourceLink");
                auto sl = state.getUncalibratedSourceLink().template get<ATLASSourceLink>();
                assert( sl != nullptr );
                measState.reset(sl->clone());
                ATH_MSG_DEBUG("Successfully used ATLASSourceLink");

              }
            }

            double nDoF = state.calibratedSize();
            auto quality = Trk::FitQualityOnSurface(state.chi2(), nDoF);
            auto perState = new Trk::TrackStateOnSurface(quality,
                                                         std::move(measState),
                                                         std::move(parm),
                                                         nullptr,
                                                         typePattern);

            // If a state was succesfully created add it to the trajectory
            finalTrajectory->insert(finalTrajectory->begin(), perState);
          });

      // Convert the perigee state and add it to the trajectory
      const Acts::BoundTrackParameters actsPer(track.referenceSurface().getSharedPtr(),
                                               track.parameters(),
                                               track.covariance(),
                                               hypothesis);

      std::unique_ptr<Trk::TrackParameters> per = m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsPer, tgContext);
      std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
      typePattern.set(Trk::TrackStateOnSurface::Perigee);
      auto perState = new Trk::TrackStateOnSurface(nullptr,
                                                   std::move(per),
                                                   nullptr,
                                                   typePattern);
      finalTrajectory->insert(finalTrajectory->begin(), perState);

      // Create the track using Athena TrackFitter::KalmanFitter and TrackPatternRecoInfo::SiSPSeededFinder algorithm enums
      Trk::TrackInfo newInfo(Trk::TrackInfo::TrackFitter::KalmanFitter, Trk::noHypothesis);
      newInfo.setPatternRecognitionInfo(Trk::TrackInfo::TrackPatternRecoInfo::SiSPSeededFinder);

      auto newtrack = std::make_unique<Trk::Track>(newInfo, std::move(finalTrajectory), nullptr);
      // TODO: use TrackSummaryTool to create trackSummary?
      if (!newtrack->trackSummary())
      {
        newtrack->setTrackSummary(std::make_unique<Trk::TrackSummary>());
        newtrack->trackSummary()->update(Trk::numberOfPixelHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfSCTHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfTRTHoles, 0);
        newtrack->trackSummary()->update(Trk::numberOfPixelDeadSensors, numberOfDeadPixel);
        newtrack->trackSummary()->update(Trk::numberOfSCTDeadSensors, numberOfDeadSCT);
      }

      m_trkSummaryTool->updateTrackSummary(ctx, *newtrack, true);
      tracksContainer.push_back(std::move(newtrack));
    }

    return StatusCode::SUCCESS;
  }

  std::unique_ptr<Trk::MeasurementBase> ActsToTrkConvertorAlg::makeRIO_OnTrack(const xAOD::UncalibratedMeasurement &uncalibMeas,
										     const Trk::TrackParameters &parm) const
  {
    const Trk::PrepRawData *rio = nullptr;
    const xAOD::UncalibMeasType measurementType = uncalibMeas.type();

    if (measurementType == xAOD::UncalibMeasType::PixelClusterType)
    {
      static const SG::AuxElement::ConstAccessor<ElementLink<InDet::PixelClusterCollection>> pixelLinkAcc("pixelClusterLink");
      auto pixcl = dynamic_cast<const xAOD::PixelCluster *>(&uncalibMeas);
      if (not pixcl or not pixelLinkAcc.isAvailable(*pixcl))
      {
        ATH_MSG_DEBUG("no pixelClusterLink for cluster associated to measurement");
        return nullptr;
      }

      auto pix = *pixelLinkAcc(*pixcl);
      if (m_RotCreatorTool.empty())
      {
        const InDetDD::SiDetectorElement *element = pix->detectorElement();
        if (!element)
        {
          ATH_MSG_WARNING("Cannot access pixel detector element");
          return nullptr;
        }
        ATH_MSG_DEBUG("create InDet::PixelClusterOnTrack without correction");
        return std::make_unique<InDet::PixelClusterOnTrack>(pix,
                                                            Trk::LocalParameters(pix->localPosition()),
                                                            Amg::MatrixX(pix->localCovariance()),
                                                            element->identifyHash(),
                                                            pix->globalPosition(),
                                                            pix->gangedPixel(),
                                                            false);
      }
      rio = pix;
    }
    else if (measurementType == xAOD::UncalibMeasType::StripClusterType)
    {
      static const SG::AuxElement::ConstAccessor<ElementLink<InDet::SCT_ClusterCollection>> stripLinkAcc("sctClusterLink");
      auto stripcl = dynamic_cast<const xAOD::StripCluster *>(&uncalibMeas);
      if (not stripcl or not stripLinkAcc.isAvailable(*stripcl))
      {
        ATH_MSG_WARNING("no sctClusterLink for clusters associated to measurement");
        return nullptr;
      }

      auto sct = *stripLinkAcc(*stripcl);
      if (m_RotCreatorTool.empty())
      {
        const InDetDD::SiDetectorElement *element = sct->detectorElement();
        if (!element)
        {
          ATH_MSG_WARNING("Cannot access strip detector element");
          return nullptr;
        }
        ATH_MSG_DEBUG("create InDet::SCT_ClusterOnTrack without correction");
        return std::make_unique<InDet::SCT_ClusterOnTrack>(sct,
                                                           Trk::LocalParameters(sct->localPosition()),
                                                           Amg::MatrixX(sct->localCovariance()),
                                                           element->identifyHash(),
                                                           sct->globalPosition(),
                                                           false);
      }
      rio = sct;
    }
    else
    {
      ATH_MSG_WARNING("xAOD::UncalibratedMeasurement is neither xAOD::PixelCluster nor xAOD::StripCluster");
      return nullptr;
    }

    ATH_MSG_DEBUG("use Trk::RIO_OnTrackCreator::correct to create corrected Trk::RIO_OnTrack");
    assert(!m_RotCreatorTool.empty());
    assert(rio != nullptr);
    return std::unique_ptr<Trk::MeasurementBase>(m_RotCreatorTool->correct(*rio, parm));
  }

}
