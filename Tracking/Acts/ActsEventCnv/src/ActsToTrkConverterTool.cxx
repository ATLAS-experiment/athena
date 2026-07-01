/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsToTrkConverterTool.h"

// Trk
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "TrkSurfaces/AnnulusBounds.h"
#include "TrkSurfaces/Surface.h"
#include "TrkTrack/Track.h"

// ATHENA
#include "GaudiKernel/IInterface.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "TrkExUtils/RungeKuttaUtils.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/Surface.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"

#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "xAODMuonPrepData/MuonMeasurement.h"
#include "xAODMuonPrepData/CombinedMuonStrip.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
// PACKAGE
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeoUtils/SurfaceCache.h"
#include "ActsInterop/IdentityHelper.h"
#include "ActsEvent/ParticleHypothesisEncoding.h"

// ACTS
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Surfaces/CylinderBounds.hpp"
#include "Acts/Surfaces/DiscBounds.hpp"
#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Surfaces/RadialBounds.hpp"
#include "Acts/Surfaces/DiamondBounds.hpp"

#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/TransformationHelpers.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Propagator/detail/JacobianEngine.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp"

#include "ActsEvent/MultiTrajectory.h"
#include "Acts/EventData/TrackStatePropMask.hpp"
#include "Acts/EventData/SourceLink.hpp"

#include "TrkSurfaces/DiscBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"
#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiamondBounds.h"

// STL
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <format>

namespace ActsTrk {

using namespace Acts::UnitLiterals;

std::unique_ptr<Trk::TrackParameters> rotateParams(const Trk::TrackParameters& inPars,
                                                   const Trk::Surface&  target) {
    const Amg::Vector3D& pos = inPars.position();
    const Amg::Vector3D& mom = inPars.momentum();
    using namespace Acts::PlanarHelper;
    
    const auto isect = intersectPlane(pos, mom.normalized(), target.normal(), target.center());
    std::optional<AmgSymMatrix(5)> cov{};
    if (inPars.covariance()) {
        AmgSymMatrix(5) rot {AmgSymMatrix(5)::Identity()};
        rot.block<2,2>(0,0) = AmgSymMatrix(2){Eigen::Rotation2D{90._degree}};
        cov = rot.transpose() * (*inPars.covariance()) * rot;
    }
    return target.createUniqueTrackParameters(isect.position(), mom,
                                              std::copysign(1., inPars.parameters()[Trk::qOverP]),
                                              std::move(cov));                     
}


StatusCode ActsToTrkConverterTool::initialize() {
  ATH_MSG_DEBUG("Initializing ACTS to ATLAS converter tool");
  
  ATH_CHECK(m_trkSummaryTool.retrieve());
  ATH_CHECK(m_ROTcreator.retrieve());
  ATH_CHECK(m_geometryConvTool.retrieve());
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  m_prdCalib = detail::TrkPrepRawDataCalibrator{m_geometryConvTool.get(), m_ROTcreator.get()};
  ATH_CHECK(m_keyMdt.initialize(SG::AllowEmpty));
  ATH_CHECK(m_keyRpc.initialize(SG::AllowEmpty));
  ATH_CHECK(m_keyTgc.initialize(SG::AllowEmpty));
  ATH_CHECK(m_keyMm.initialize(SG::AllowEmpty));
  ATH_CHECK(m_keyStgc.initialize(SG::AllowEmpty));
  if (!m_keyMdt.empty() || !m_keyRpc.empty() || !m_keyTgc.empty() ||
      !m_keyMm.empty() || !m_keyStgc.empty()) {
    ATH_CHECK(m_idHelperSvc.retrieve());
  }
  ATH_CHECK(m_compRotCreator.retrieve(EnableTool{!m_keyRpc.empty() || !m_keyTgc.empty()}));
  return StatusCode::SUCCESS;
}

std::vector<Acts::SourceLink>
ActsToTrkConverterTool::trkTrackToSourceLinks(const Trk::Track &track) const {
  std::vector<Acts::SourceLink> sourceLinks{};
  sourceLinks.reserve(track.measurementsOnTrack()->size() +
                      track.outliersOnTrack()->size());
  detail::MeasurementCalibratorBase::pack(track.measurementsOnTrack()->stdcont(), sourceLinks);
  detail::MeasurementCalibratorBase::pack(track.outliersOnTrack()->stdcont(), sourceLinks);
  return sourceLinks;
}
void ActsToTrkConverterTool::convertTrkToActsContainer(const EventContext& ctx,
                                                       const TrackCollection& trackColl,
                                                       ActsTrk::MutableTrackContainer& outTrackcoll) const {
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  ATH_MSG_VERBOSE("Calling trkTrackCollectionToActsTrackContainer with "
                  << trackColl.size() << " tracks.");
  unsigned int trkCount = 0;
  std::vector<Identifier> failedIds; // Keep track of Identifiers of failed conversions
  for (const Trk::Track* trk : trackColl) {
    // Do conversions!
    const Trk::TrackStates *trackStates = trk->trackStateOnSurfaces();
   
    auto actsTrack = outTrackcoll.getTrack(outTrackcoll.addTrack());
    auto& trackStateContainer = outTrackcoll.trackStateContainer();

    ATH_MSG_VERBOSE("Track "<<trkCount++<<" has " << trackStates->size()
                                 << " track states on surfaces.");
    // basic quantities copy
    actsTrack.chi2() = trk->fitQuality()->chiSquared();
    actsTrack.nDoF() = trk->fitQuality()->numberDoF();

    // loop over track states on surfaces, convert and add them to the ACTS
    // container
    bool first_tsos = true;  // We need to handle the first one differently
    int measurementsCount = 0;
    for (const Trk::TrackStateOnSurface* tsos : *trackStates) {

      // Setup the mask
      Acts::TrackStatePropMask mask = Acts::TrackStatePropMask::None;
      if (tsos->measurementOnTrack()) {
        mask |= Acts::TrackStatePropMask::Calibrated;
      }
      if (tsos->trackParameters()) {
        mask |= Acts::TrackStatePropMask::Smoothed;
      }

      // Setup the index of the trackstate
      auto index = Acts::kTrackIndexInvalid;
      if (!first_tsos) {
        index = actsTrack.tipIndex();
      }
      auto actsTSOS = trackStateContainer.getTrackState(trackStateContainer.addTrackState(mask, index));
      ATH_MSG_VERBOSE("TipIndex: " << actsTrack.tipIndex() << " TSOS index within trajectory: "<< actsTSOS.index());
      actsTrack.tipIndex() = actsTSOS.index();

      if (tsos->trackParameters()) {
        // TODO This try/catch is temporary and should be removed once the sTGC problem is fixed.
        try {
          ATH_MSG_VERBOSE("Converting track parameters.");
          // TODO - work out whether we should set predicted, filtered, smoothed
          const Acts::BoundTrackParameters parameters = m_geometryConvTool->convertTrackParametersToActs(ctx, *tsos->trackParameters());
          ATH_MSG_VERBOSE("Track parameters: " << parameters.parameters());
          // Sanity check on positions
          if (!actsTrackParameterPositionCheck(parameters, *(tsos->trackParameters()), tgContext)) {
            failedIds.push_back(tsos->trackParameters()->associatedSurface().associatedDetectorElementIdentifier());
          }

          if (first_tsos) {
            // This is the first track state, so we need to set the track
            // parameters
            actsTrack.parameters() = parameters.parameters();
            actsTrack.covariance() = *parameters.covariance();
            actsTrack.setReferenceSurface(parameters.referenceSurface().getSharedPtr());
            first_tsos = false;
          } else {
            actsTSOS.setReferenceSurface(parameters.referenceSurface().getSharedPtr());
            // Since we're converting final Trk::Tracks, let's assume they're smoothed
            actsTSOS.smoothed() = parameters.parameters();
            actsTSOS.smoothedCovariance() = *parameters.covariance();
            // Not yet implemented in MultiTrajectory.icc
            // actsTSOS.typeFlags().setHasParameters();
            if (!(actsTSOS.hasSmoothed() && actsTSOS.hasReferenceSurface())) {
              ATH_MSG_WARNING("TrackState does not have smoothed state ["
                              << actsTSOS.hasSmoothed()
                              << "] or reference surface ["
                              << actsTSOS.hasReferenceSurface() << "].");
            } else {
              ATH_MSG_VERBOSE("TrackState has smoothed state and reference surface.");
            }
          }
        } catch (const std::exception& e){
          ATH_MSG_ERROR("Unable to convert TrackParameter with exception ["<<e.what()<<"]. Will be missing from ACTS track."
                        <<(*tsos->trackParameters()));
        }
      }
      if (tsos->measurementOnTrack()) {
        auto &measurement = *(tsos->measurementOnTrack());
        actsTSOS.typeFlags().setIsMeasurement();

        measurementsCount++;
        // const Acts::Surface &surface =
        //     convertSurfaceToActs(measurement.associatedSurface());
        //  Commented for the moment because Surfaces not yet implemented in
        //  MultiTrajectory.icc

        int dim = measurement.localParameters().dimension();
        actsTSOS.allocateCalibrated(dim); 
        if (dim == 1) {
          actsTSOS.calibrated<1>() = measurement.localParameters();
          actsTSOS.calibratedCovariance<1>() = measurement.localCovariance();
        } else if (dim == 2) {
          actsTSOS.calibrated<2>() = measurement.localParameters();
          actsTSOS.calibratedCovariance<2>() = measurement.localCovariance();
        } else {
          throw std::domain_error("Cannot handle measurement dim>2");
        }
        actsTSOS.setUncalibratedSourceLink(detail::TrkMeasurementCalibrator::pack(tsos->measurementOnTrack()));

      }  // end if measurement
    }    // end loop over track states
    actsTrack.nMeasurements() = measurementsCount;
    ATH_MSG_VERBOSE("TrackProxy has " << actsTrack.nTrackStates()
                                 << " track states on surfaces.");
  }
  ATH_MSG_VERBOSE("Finished converting " << trackColl.size() << " tracks.");

  if (!failedIds.empty()){
    ATH_MSG_WARNING("Failed to convert "<<failedIds.size()<<" track parameters.");
    for (auto id : failedIds){
      ATH_MSG_WARNING("-> Failed for Identifier "<<m_idHelperSvc->toString(id));
    }
  }
  ATH_MSG_VERBOSE("ACTS Track container has " << outTrackcoll.size() << " tracks.");
}
bool ActsToTrkConverterTool::actsTrackParameterPositionCheck(
    const Acts::BoundTrackParameters &parameters,
    const Trk::TrackParameters &trkparameters,
    const Acts::GeometryContext &gctx) const {
  auto actsPos = parameters.position(gctx);

  if ( (actsPos - trkparameters.position()).mag() > 0.1) {
    ATH_MSG_WARNING("Parameter position mismatch. Acts \n"
                    << actsPos << " vs Trk \n"
                    << trkparameters.position());
    ATH_MSG_WARNING("Acts surface:");
    ATH_MSG_WARNING(parameters.referenceSurface().toString(gctx));
    ATH_MSG_WARNING("Trk surface:");
    ATH_MSG_WARNING(trkparameters.associatedSurface());
    return false;
  }
  return true;
}

std::unique_ptr<Trk::Track> ActsToTrkConverterTool::convertFitResult(const EventContext& ctx,
                                                                     TrackFitResult_t& fitResult,
                                                                     const Trk::TrackInfo::TrackFitter fitAuthor) const {

    if (not fitResult.ok()) {
      ATH_MSG_VERBOSE("Fit did not converge");  
      return nullptr;    
    }
    return convertActsTrack(ctx, fitResult.value(), fitAuthor);
}

template <typename Proxy_t>
  std::unique_ptr<Trk::Track> 
    ActsToTrkConverterTool::convertActsTrack(const EventContext& ctx,
                                             const Proxy_t& acts_track,
                                             const Trk::TrackInfo::TrackFitter fitAuthor) const{


    const Acts::CalibrationContext cctx{getCalibrationContext(ctx)};
    const Acts::GeometryContext tgContext{m_trackingGeometryTool->getGeometryContext(ctx).context()};
   
    auto finalTrajectory = std::make_unique<Trk::TrackStates>();
    int nDoF{0};

    double chi2{0};

    // Loop over all the output state to create track state
    acts_track.container().trackStateContainer().visitBackwards(acts_track.tipIndex(), 
      [&] (const auto &state) -> void {
        if (!state.hasReferenceSurface()) {
          return;
        }
        // First only consider state with an associated detector element
        if (!m_convertMaterial && !state.referenceSurface().isSensitive()) {
          return;
        }

        if (const auto* associatedDetEl = dynamic_cast<const IDetectorElementBase*>(
                                        state.referenceSurface().surfacePlacement());
            associatedDetEl != nullptr) {
            ATH_MSG_VERBOSE("Associated det: "<<associatedDetEl->detectorType());
        }

        auto flag = state.typeFlags();
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - "<<", hole: "<<flag.isHole()
                      <<", outlier: "<<flag.isOutlier()<<", measurement: "<<flag.isMeasurement()<<"/"
                      <<flag.hasMeasurement()<<", "<<m_convertOutliers<<", "<<m_convertHoles
                      <<", has SL: "<<state.hasUncalibratedSourceLink());
        // We need to determine the type of state 
        TrkTSOSMask typePattern;
        std::unique_ptr<Trk::TrackParameters> trkPars = m_geometryConvTool->convertTrackParametersToTrk(ctx, 
                                                                      acts_track.createParametersFromState(state));
        std::unique_ptr<Trk::MeasurementBase> trkMeasurement{};


        // State is a hole (no associated measurement), use predicted parameters   
        if (flag.isHole()) {
          if (!m_convertHoles) { return; }
          typePattern.set(Trk::TrackStateOnSurface::Hole);
        } if (flag.isOutlier()) {
          if (!m_convertOutliers) { return; }
          typePattern.set(Trk::TrackStateOnSurface::Outlier);
        } 
        if (flag.hasMeasurement()) {
          typePattern.set(Trk::TrackStateOnSurface::Measurement);
          nDoF = state.calibratedSize();
          chi2 = state.chi2();
          const auto slType = detail::MeasurementCalibratorBase::getType(state.getUncalibratedSourceLink());
          switch (slType) {
              using enum detail::SourceLinkType;
              case TrkMeasurement: 
                trkMeasurement = m_measCalib.unpack(state.getUncalibratedSourceLink())->uniqueClone();
                break;
              case TrkPrepRawData:
                trkMeasurement = m_prdCalib.createROT(tgContext, cctx, state.getUncalibratedSourceLink(), state);
                break;
              case xAODUnCalibMeas:
                  appendMeasTSOS(ctx, detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink()),
                                 typePattern, Trk::FitQualityOnSurface{chi2, nDoF}, 
                                 std::move(trkPars), *finalTrajectory);
                  return;
              default:
                THROW_EXCEPTION("Invalid "<<slType<<" type parsed.");
          }
        }
        auto perState = std::make_unique<Trk::TrackStateOnSurface>(Trk::FitQualityOnSurface{chi2, nDoF},
                                                                   std::move(trkMeasurement), 
                                                                   std::move(trkPars), nullptr, typePattern);
        // If a state was succesfully created add it to the trajectory 
        ATH_MSG_VERBOSE("State succesfully created, adding it to the trajectory");
        finalTrajectory->insert(finalTrajectory->begin(), std::move(perState));
      });
      // Convert the perigee state and add it to the trajectory
      std::unique_ptr<Trk::TrackParameters> per = m_geometryConvTool->convertTrackParametersToTrk(ctx, acts_track.createParametersAtReference());
      TrkTSOSMask typePattern;
      typePattern.set(Trk::TrackStateOnSurface::Perigee);
      finalTrajectory->insert(finalTrajectory->begin(), 
                              std::make_unique<Trk::TrackStateOnSurface>(nullptr, std::move(per), nullptr, typePattern));
      // Create the track using the states
      Trk::TrackInfo newInfo{fitAuthor, ParticleHypothesis::convertTrk(acts_track.particleHypothesis())};
      auto newtrack = std::make_unique<Trk::Track>(newInfo, std::move(finalTrajectory), nullptr);
      constexpr bool suppressHoleSearch = false;
      m_trkSummaryTool->updateTrackSummary(ctx, *newtrack, suppressHoleSearch);
      ATH_MSG_VERBOSE("Created new track "<<(*newtrack->trackSummary()));
      return newtrack;
  }

  std::unique_ptr<TrackCollection> 
    ActsToTrkConverterTool::convertActsToTrkContainer(const EventContext& ctx,
                                                      const ActsTrk::TrackContainer& trackCont) const {
      auto outColl = std::make_unique<TrackCollection>();
      for (const ActsTrk::TrackContainer::ConstTrackProxy& trk : trackCont) {
          outColl->push_back(convertActsTrack(ctx, trk, m_fitAuthor));
      }
      return outColl;
  }
  void ActsToTrkConverterTool::appendMeasTSOS(const EventContext& ctx,
                                              const xAOD::UncalibratedMeasurement* meas,
                                              const TrkTSOSMask typePattern,
                                              Trk::FitQualityOnSurface&& quality,
                                              std::unique_ptr<Trk::TrackParameters> trkPars,
                                              Trk::TrackStates& states) const {
    std::unique_ptr<Trk::MeasurementBase> rot{};
    switch (meas->type()) {
        using enum xAOD::UncalibMeasType;
        case PixelClusterType: {
          static const SG::AuxElement::ConstAccessor<ElementLink<InDet::PixelClusterCollection>> acc_prdLink("pixelClusterLink");
          if (acc_prdLink.isAvailable(*meas) && acc_prdLink(*meas).isValid()) {
              rot.reset(m_ROTcreator->correct(**acc_prdLink(*meas), *trkPars, ctx));
          } else {
              ATH_MSG_WARNING(__func__<<" () "<<__LINE__<<" - The pixel xAOD -> prd accessor is invalid");
          }
          break;
        } case StripClusterType: {
          static const SG::AuxElement::ConstAccessor<ElementLink<InDet::SCT_ClusterCollection>> acc_prdLink("sctClusterLink");
          if (acc_prdLink.isAvailable(*meas) && acc_prdLink(*meas).isValid()) {
              rot.reset(m_ROTcreator->correct(**acc_prdLink(*meas), *trkPars, ctx));
          } else {
              ATH_MSG_WARNING(__func__<<" () "<<__LINE__<<" - The strip xAOD -> prd accessor is invalid");
          }
          break;
        } case MdtDriftCircleType:
          case MMClusterType: {
            const Identifier& id = static_cast<const xAOD::MuonMeasurement*>(meas)->identify();
            const IdentifierHash modHash = m_idHelperSvc->moduleHash(id);
            const auto* prd =  meas->type() == MdtDriftCircleType ?  fetchPrd(ctx, m_keyMdt, id, modHash) 
                                                                  :  fetchPrd(ctx, m_keyMm, id, modHash);
            assert(prd != nullptr);
            rot.reset(m_ROTcreator->correct(*prd, *trkPars, ctx));
            break;
        } case TgcStripType:
          case RpcStripType:
          case sTgcStripType: {
            const Identifier& id = static_cast<const xAOD::MuonMeasurement*>(meas)->identify();
            const IdentifierHash modHash = m_idHelperSvc->moduleHash(id);
            /// Dimension 0 measurements are combined measurements!
            const Trk::PrepRawData* prd{nullptr}, *prd1{nullptr};

            if (meas->numDimensions() == 0) {
              const auto* muonMeas = static_cast<const xAOD::CombinedMuonStrip*>(meas);
              if (meas->type() == RpcStripType) {
                prd  = fetchPrd(ctx, m_keyRpc, id, modHash);
                prd1 = fetchPrd(ctx, m_keyRpc, muonMeas->secondaryStrip()->identify(), modHash);
              } else if (meas->type() == TgcStripType) {
                prd = fetchPrd(ctx, m_keyTgc, id, modHash);
                prd1 = fetchPrd(ctx, m_keyTgc, muonMeas->secondaryStrip()->identify(), modHash);
              } else {
                prd  = fetchPrd(ctx, m_keyStgc, id, modHash);
                prd1 = fetchPrd(ctx, m_keyStgc, muonMeas->secondaryStrip()->identify(), modHash);
              }
              assert(prd != nullptr);
              assert(prd1 != nullptr);

              /// @todo Check whether the phi surface is in front of the eta surface
              const Trk::Surface& phiSurface = prd1->detectorElement()->surface(prd1->identify());
              auto phiPars = rotateParams(*trkPars, phiSurface);
              assert(phiPars != nullptr);
              std::unique_ptr<Trk::MeasurementBase> phiRot{};
              if (meas->type() != sTgcStripType) {
                phiRot = m_compRotCreator->createBroadCluster(std::list{prd1}, 1.);
                rot = m_compRotCreator->createBroadCluster(std::list{prd}, 1.);
              } else {
                phiRot.reset(m_ROTcreator->correct(*prd1, *phiPars, ctx));
                rot.reset(m_ROTcreator->correct(*prd, *trkPars, ctx));
              }
              assert(phiRot != nullptr);
              states.insert(states.begin(),
                            std::make_unique<Trk::TrackStateOnSurface>(quality, 
                                                                       std::move(phiRot), 
                                                                       std::move(phiPars), nullptr, typePattern));
            } else {
              const auto* prd  = meas->type() == RpcStripType ? fetchPrd(ctx, m_keyRpc, id, modHash)
                                                              :
                                meas->type() == TgcStripType ? fetchPrd(ctx, m_keyTgc, id, modHash)
                                                             : fetchPrd(ctx, m_keyStgc, id, modHash);
              assert(prd != nullptr);
              // Track parameter representation needs to change towards a phi surface
              if (m_idHelperSvc->measuresPhi(id)) {
                  ATH_MSG_VERBOSE("Convert the track parameters "<<m_idHelperSvc->toString(id)
                                <<", "<<m_idHelperSvc->toStringDetEl(prd->detectorElement()->identify()));
                  const Trk::Surface& target = prd->detectorElement()->surface(id);
                  trkPars = rotateParams(*trkPars, target);
              }
              /// Correct the ROT according to the surface
              rot.reset(m_ROTcreator->correct(*prd, *trkPars, ctx));
            }
            break;
        } default:
          ATH_MSG_WARNING("Measurement type "<<meas->type()<<" is not implemented");
          return;
    }
    assert(rot != nullptr);
    assert(trkPars != nullptr);
    states.insert(states.begin(),
                  std::make_unique<Trk::TrackStateOnSurface>(std::move(quality), std::move(rot), 
                                                             std::move(trkPars), nullptr, typePattern));

  }
  template <typename PrdType_t>
  const Trk::PrepRawData* ActsToTrkConverterTool::fetchPrd(const EventContext& ctx,
                                                           const SG::ReadHandleKey<PrdType_t>& key,
                                                           const Identifier& prdId,
                                                           const IdentifierHash& hash) const{
      const PrdType_t* container{nullptr};
      if (key.empty() || !SG::get(container, key, ctx).isSuccess()) {
          THROW_EXCEPTION("Failed to retrieve container "<<key.fullKey());
      }
      const auto* coll = container->indexFindPtr(hash);
      if (coll == nullptr){
          ATH_MSG_WARNING("fetchPrd() - Failed to find a valid collection for "<<prdId.getString()<<", key: "<<key.fullKey());
          return nullptr;
      }
      for (const Trk::PrepRawData* prd : *coll) {
          if (prd->identify() == prdId) {
              return prd;
          }
      }
      ATH_MSG_WARNING("fetchPrd() - Prep data object "<<prdId.getString()<<" is not in "<<key.fullKey());
      return nullptr;
  }
}  // namespace ActsTrk
