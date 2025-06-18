/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsToTrkConverterTool.h"

// Trk
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
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"

// PACKAGE
#include "ActsCalibration/CalibrationContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometry/ActsTrackingGeometryTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeoUtils/SurfaceCache.h"
#include "ActsInterop/IdentityHelper.h"
#include "ActsEvent/ParticleHypothesisEncoding.h"

// ACTS
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/TransformationHelpers.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Propagator/detail/JacobianEngine.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "Acts/EventData/TrackStatePropMask.hpp"
#include "Acts/EventData/SourceLink.hpp"

#include "ActsCalibration/TrkMeasurementCalibrator.h"
#include "ActsCalibration/TrkPrepRawDataCalibrator.h"
// STL
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <format>

namespace ActsTrk {

// Forward definitions of local functions
// @TODO unused, remove ?
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void ActsMeasurementCheck(const Acts::GeometryContext &gctx,
                                 const Trk::MeasurementBase &measurement,
                                 const Acts::Surface &surface,
                                 const Acts::BoundVector &loc);
#pragma GCC diagnostic pop

static void ActsTrackParameterCheck(
    const Acts::BoundTrackParameters &actsParameter,
    const Acts::GeometryContext &gctx, const Acts::BoundSquareMatrix &covpc,
    const Acts::BoundVector &targetPars, const Acts::BoundSquareMatrix &targetCov,
    const Trk::PlaneSurface *planeSurface);



StatusCode ActsToTrkConverterTool::initialize() {
  ATH_MSG_VERBOSE("Initializing ACTS to ATLAS converter tool");
  if (!m_trackingGeometryTool.empty()) {
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    m_trackingGeometry = m_trackingGeometryTool->trackingGeometry();

    m_trackingGeometry->visitSurfaces([&](const Acts::Surface *surface) {
      // find acts surface with the same detector element ID
      if (!surface)
        return;
      const auto *actsElement = dynamic_cast<const IDetectorElementBase*>(
          surface->associatedDetectorElement());
      if (!actsElement) {
        return;
      }
      // Conversion from Acts to ATLAS surface impossible for the TRT so the TRT
      // surfaces are not stored in this map
      if (actsElement->detectorType() == DetectorType::Trt) {
          return;
      }

      auto [it, ok] =  m_actsSurfaceMap.insert({actsElement->identify(), surface});
      if (!ok) {
        ATH_MSG_WARNING("ATLAS ID " << actsElement->identify()
                                    << " has two ACTS surfaces: "
                                    << it->second->geometryId() << " and "
                                    << surface->geometryId());
      }
    });
  }

  ATH_CHECK(m_trkSummaryTool.retrieve());
  ATH_CHECK(m_boundaryCheckTool.retrieve(EnableTool{!m_boundaryCheckTool.empty()}));
  ATH_CHECK(m_ROTcreator.retrieve());

  ATH_CHECK(m_muonMgrKey.initialize(m_extractMuonSurfaces));
  if (m_extractMuonSurfaces){
    ATH_CHECK(m_idHelperSvc.retrieve());
    const MuonGMR4::MuonDetectorManager* muonMgr{nullptr};    
    ATH_CHECK(detStore()->retrieve(muonMgr));
    unsigned int mapSize = m_actsSurfaceMap.size(); // For debugging message later
    for (auto readoutElement : muonMgr->getAllReadoutElements()) {
      std::vector<std::shared_ptr<Acts::Surface>> reSurfaces = readoutElement->getSurfaces();
      for ( const auto& surf : reSurfaces) {
        const Identifier id = static_cast<const SurfaceCache*>(surf->associatedDetectorElement())->identify();
        m_actsSurfaceMap.insert(std::make_pair(id, surf.get()));
      }
    }
    ATH_MSG_VERBOSE("After adding muon surfaces, the map has grown from "<<mapSize<<" to "<<m_actsSurfaceMap.size());
  }
  return StatusCode::SUCCESS;
}

const Trk::Surface &ActsToTrkConverterTool::actsSurfaceToTrkSurface(
    const Acts::Surface &actsSurface) const {

  const auto *detEleBase= dynamic_cast<const IDetectorElementBase*>(actsSurface.associatedDetectorElement());
  if (!detEleBase) {
    ATH_MSG_ERROR(actsSurface.toString(m_trackingGeometryTool->getNominalGeometryContext().context()));
    throw std::domain_error("ActsToTrkConverterTool() - Surface does not have an associated detector element. ");
  }
  switch (detEleBase->detectorType()) {
      using enum DetectorType;
      case Pixel:
      case Sct:
      case Hgtd:
      case Trt: {
        const auto actsElement = dynamic_cast<const ActsDetectorElement*>(detEleBase);
        if (actsElement) {
            return actsElement->atlasSurface();
        }
        break;
      }
      case Mdt:
      case Rpc:
      case Tgc:
      case Csc:
      case sTgc:
      case Mm: {
        const MuonGM::MuonDetectorManager* detMgr{nullptr};
        if (!SG::get(detMgr, m_muonMgrKey, Gaudi::Hive::currentContext()).isSuccess() || !detMgr) {
            THROW_EXCEPTION("Failed to retrieve the muon detector manager");
        }
        return detMgr->getReadoutElement(detEleBase->identify())->surface(detEleBase->identify());
      
      } default:
        break;
  }
  throw std::domain_error("ActsToTrkConverterTool() - No ATLAS surface corresponding to the Acts one");
}

const Acts::Surface &ActsToTrkConverterTool::trkSurfaceToActsSurface(
    const Trk::Surface &atlasSurface) const {

  Identifier atlasID = atlasSurface.associatedDetectorElementIdentifier();
  auto it = m_actsSurfaceMap.find(atlasID);
  if (it != m_actsSurfaceMap.end()) {
    return *it->second;
  }
  ATH_MSG_ERROR("No Acts surface corresponding to this ATLAS surface: "<<atlasID);
  ATH_MSG_ERROR(atlasSurface);
  throw std::domain_error("No Acts surface corresponding to the ATLAS one");
}

std::vector<Acts::SourceLink>
ActsToTrkConverterTool::trkTrackToSourceLinks(const Trk::Track &track) const {
  std::vector<Acts::SourceLink> sourceLinks{};
  sourceLinks.reserve(track.measurementsOnTrack()->size() +
                      track.outliersOnTrack()->size());
  toSourceLinks(track.measurementsOnTrack()->stdcont(), sourceLinks);
  toSourceLinks(track.outliersOnTrack()->stdcont(), sourceLinks);
  return sourceLinks;
}

 
void ActsToTrkConverterTool::toSourceLinks(const std::vector<const Trk::MeasurementBase*>& measSet,
                                                    std::vector<Acts::SourceLink>& sourceLinks) const{
    if (sourceLinks.capacity() < sourceLinks.size() + measSet.size()) {
      sourceLinks.reserve(sourceLinks.size() + measSet.size());
    }
    std::ranges::transform(measSet, std::back_inserter(sourceLinks), [](const Trk::MeasurementBase* meas) {
                             return detail::TrkMeasurementCalibrator::pack(meas);
                          });
}
void ActsToTrkConverterTool::toSourceLinks(const std::vector<const Trk::PrepRawData*>& prdSet,
                                                    std::vector<Acts::SourceLink>& links) const {
    if (links.capacity() < links.size() + prdSet.size()) {
      links.reserve(links.size() + prdSet.size());
    }
    std::ranges::transform(prdSet, std::back_inserter(links), [](const Trk::PrepRawData* prd) {
                            return detail::TrkPrepRawDataCalibrator::pack(prd);
                          });
}

const Acts::BoundTrackParameters
ActsToTrkConverterTool::trkTrackParametersToActsParameters(const Trk::TrackParameters &atlasParameter, 
                                                                    const Acts::GeometryContext & gctx, 
                                                                    Trk::ParticleHypothesis hypothesis) const {

  using namespace Acts::UnitLiterals;
  std::shared_ptr<const Acts::Surface> actsSurface;
  Acts::BoundVector params;

  // get the associated surface
  if (atlasParameter.hasSurface() &&
      atlasParameter.associatedSurface().owner() == Trk::SurfaceOwner::DetElOwn) {
    try {
      actsSurface = trkSurfaceToActsSurface(atlasParameter.associatedSurface()).getSharedPtr();
    } catch (const std::exception &e) {
      ATH_MSG_ERROR("Could not find ACTS detector surface for this TrackParameter:");
      ATH_MSG_ERROR(atlasParameter);
      throw;  // Nothing we can do, so just pass exception on...
    }
  }
  // no associated surface create a perigee one
  else {
    ATH_MSG_VERBOSE(
        "trkTrackParametersToActsParameters:: No associated surface found (owner: "<<atlasParameter.associatedSurface().owner()<<
        "). Creating a free surface. Trk parameters:");
    ATH_MSG_VERBOSE(atlasParameter);
    const Amg::Transform3D& trf{atlasParameter.associatedSurface().transform()};
    switch (atlasParameter.associatedSurface().type()){
      case Trk::SurfaceType::Plane:
        actsSurface = Acts::Surface::makeShared<const Acts::PlaneSurface>(trf);
        break;
      case Trk::SurfaceType::Perigee:
        actsSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(trf);
        break;
      case Trk::SurfaceType::Line:
        actsSurface = Acts::Surface::makeShared<const Acts::StrawSurface>(trf);
        break;
      // TODO - implement the missing types?
      default: {
        std::stringstream surfStr{};
        atlasParameter.dump(surfStr);  
        throw std::domain_error(std::format("Failed to translate parameters {:}", surfStr.str()));
      }
    }
  }

  // Construct track parameters
  const auto& atlasParam{atlasParameter.parameters()};
  if (actsSurface->bounds().type() == Acts::SurfaceBounds::BoundsType::eAnnulus) {
    // Annulus surfaces are constructed differently in Acts/Trk so we need to
    // convert local coordinates
    const Amg::Vector3D& position{atlasParameter.position()};
    auto result = actsSurface->globalToLocal(gctx, position, atlasParameter.momentum());
    if (result.ok()) {
      params << (*result)[0], (*result)[1], atlasParam[Trk::phi0],
          atlasParam[Trk::theta],
          atlasParameter.charge() / (atlasParameter.momentum().mag() * 1_MeV),
          0.;
    } else {
      ATH_MSG_WARNING("Unable to convert annulus surface - globalToLocal failed");
    }
  } else {
    params << atlasParam[Trk::locX], atlasParam[Trk::locY],
             atlasParam[Trk::phi0], atlasParam[Trk::theta],
             atlasParameter.charge() / (atlasParameter.momentum().mag() * 1_MeV), 0.;
  }

  Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Identity();
  if (atlasParameter.covariance()) {
    cov.topLeftCorner(5, 5) = *atlasParameter.covariance();

    // Convert the covariance matrix from MeV
    // FIXME: This needs to handle the annulus case as well - currently the cov
    // is wrong for annulus surfaces
    for (int i = 0; i < cov.rows(); i++) {
      cov(i, 4) = cov(i, 4) / 1_MeV;
    }
    for (int i = 0; i < cov.cols(); i++) {
      cov(4, i) = cov(4, i) / 1_MeV;
    }
  }

  return Acts::BoundTrackParameters(actsSurface, params,cov, 
                                    ParticleHypothesis::convert(hypothesis));
}

std::unique_ptr<Trk::TrackParameters>
ActsToTrkConverterTool::actsTrackParametersToTrkParameters(
    const Acts::BoundTrackParameters &actsParameter,
    const Acts::GeometryContext &gctx) const {

  using namespace Acts::UnitLiterals;
  std::optional<AmgSymMatrix(5)> cov = std::nullopt;
  if (actsParameter.covariance()) {
    AmgSymMatrix(5) newcov(actsParameter.covariance()->topLeftCorner<5, 5>());
    // Convert the covariance matrix to GeV
    for (int i = 0; i < newcov.rows(); i++) {
      newcov(i, 4) = newcov(i, 4) * 1_MeV;
    }
    for (int i = 0; i < newcov.cols(); i++) {
      newcov(4, i) = newcov(4, i) * 1_MeV;
    }
    cov = std::optional<AmgSymMatrix(5)>(newcov);
  }

  const Acts::Surface &actsSurface = actsParameter.referenceSurface();
  switch (actsSurface.type()) {
    case Acts::Surface::SurfaceType::Cone: {
      const auto &coneSurface = static_cast<const Trk::ConeSurface&>(actsSurfaceToTrkSurface(actsSurface));
      return std::make_unique<Trk::AtaCone>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, coneSurface, cov);
    } case Acts::Surface::SurfaceType::Cylinder: {
      const auto &cylSurface{static_cast<const Trk::CylinderSurface&>(actsSurfaceToTrkSurface(actsSurface))};
      return std::make_unique<Trk::AtaCylinder>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, cylSurface, cov);
    } case Acts::Surface::SurfaceType::Disc: {
      const Trk::Surface& trkSurface{actsSurfaceToTrkSurface(actsSurface)};
      if (trkSurface.type() == Trk::SurfaceType::Disc) {
         const auto& discSurface{static_cast<const Trk::DiscSurface&>(trkSurface)};
         return std::make_unique<Trk::AtaDisc>(
              actsParameter.get<Acts::eBoundLoc0>(),
              actsParameter.get<Acts::eBoundLoc1>(),
              actsParameter.get<Acts::eBoundPhi>(),
              actsParameter.get<Acts::eBoundTheta>(),
              actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, discSurface, cov);
      } else if (trkSurface.type() == Trk::SurfaceType::Plane) {
        auto& planeSurface{static_cast<const Trk::PlaneSurface&>(trkSurface)};
        // need to convert to plane position on plane surface (annulus bounds)
        auto helperSurface = Acts::Surface::makeShared<Acts::PlaneSurface>(planeSurface.transform());

        auto covpc = actsParameter.covariance().value();
        /// Convert to free parameters
        Acts::FreeVector freePars = Acts::transformBoundToFreeParameters(actsSurface, gctx, 
                                                                         actsParameter.parameters());

        /// Back conversion to bound parameters of the helper plane
        Acts::BoundVector targetPars = Acts::transformFreeToBoundParameters(freePars,
                                                  *helperSurface, gctx).value();
        
        Acts::FreeMatrix freeTransportJacobian{Acts::FreeMatrix::Identity()};

        Acts::FreeVector freeToPathDerivatives{Acts::FreeVector::Zero()};
        freeToPathDerivatives.head<3>() = freePars.segment<3>(Acts::eFreeDir0);

        auto boundToFreeJacobian = actsSurface.boundToFreeJacobian(gctx, freePars.segment<3>(Acts::eFreePos0),
                                                                   freePars.segment<3>(Acts::eFreeDir0));

        Acts::BoundMatrix boundToBoundJac = Acts::detail::boundToBoundTransportJacobian(gctx, freePars, 
                  boundToFreeJacobian, freeTransportJacobian, freeToPathDerivatives, *helperSurface);

        Acts::BoundMatrix targetCov{boundToBoundJac * covpc * boundToBoundJac.transpose()};

        auto pars = std::make_unique<Trk::AtaPlane>(
            targetPars[Acts::eBoundLoc0], targetPars[Acts::eBoundLoc1],
            targetPars[Acts::eBoundPhi], targetPars[Acts::eBoundTheta],
            targetPars[Acts::eBoundQOverP] * 1_MeV, planeSurface,
            targetCov.topLeftCorner<5, 5>());

        if (m_visualDebugOutput) {
          ActsTrackParameterCheck(actsParameter, gctx, covpc, targetPars,
                                  targetCov, &planeSurface);
        }
        return pars;

      } else {
        throw std::domain_error("Acts::DiscSurface is not associated with ATLAS disc or plane surface");
      }
      break;
    } case Acts::Surface::SurfaceType::Perigee: {
      const Trk::PerigeeSurface perSurface(actsSurface.center(gctx));
      return std::make_unique<Trk::Perigee>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, perSurface, cov);
    } case Acts::Surface::SurfaceType::Plane: {
      auto &plaSurface{static_cast<const Trk::PlaneSurface&>(actsSurfaceToTrkSurface(actsSurface))};
      return std::make_unique<Trk::AtaPlane>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, plaSurface, cov);
    } case Acts::Surface::SurfaceType::Straw: {
      auto& lineSurface{static_cast<const Trk::StraightLineSurface&>(actsSurfaceToTrkSurface(actsSurface))};
      return std::make_unique<Trk::AtaStraightLine>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, lineSurface, cov);
    } case Acts::Surface::SurfaceType::Curvilinear: {
      return std::make_unique<Trk::CurvilinearParameters>(
          actsParameter.position(gctx), actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, cov);
      break;
    } case Acts::Surface::SurfaceType::Other: {
      break;
    }
  }
  throw std::domain_error("Surface type not found");
}

void ActsToTrkConverterTool::trkTrackCollectionToActsTrackContainer(MutableTrackContainer &tc,
                                                                             const TrackCollection &trackColl,
                                                                             const Acts::GeometryContext & gctx) const {
  ATH_MSG_VERBOSE("Calling trkTrackCollectionToActsTrackContainer with "
                  << trackColl.size() << " tracks.");
  unsigned int trkCount = 0;
  std::vector<Identifier> failedIds; // Keep track of Identifiers of failed conversions
  for (const Trk::Track* trk : trackColl) {
    // Do conversions!
    const Trk::TrackStates *trackStates = trk->trackStateOnSurfaces();
   
    auto actsTrack = tc.getTrack(tc.addTrack());
    auto& trackStateContainer = tc.trackStateContainer();

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
      auto index = Acts::MultiTrajectoryTraits::kInvalid;
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
          const Acts::BoundTrackParameters parameters =
              trkTrackParametersToActsParameters(*(tsos->trackParameters()), gctx);
          ATH_MSG_VERBOSE("Track parameters: " << parameters.parameters());
          // Sanity check on positions
          if (!actsTrackParameterPositionCheck(parameters, *(tsos->trackParameters()), gctx)){
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
            // actsTSOS.typeFlags() |= Acts::TrackStateFlag::ParameterFlag;
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

        measurementsCount++;
        // const Acts::Surface &surface =
        //     trkSurfaceToActsSurface(measurement.associatedSurface());
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
  ATH_MSG_VERBOSE("ACTS Track container has " << tc.size() << " tracks.");
}

bool ActsToTrkConverterTool::actsTrackParameterPositionCheck(
    const Acts::BoundTrackParameters &parameters,
    const Trk::TrackParameters &trkparameters,
    const Acts::GeometryContext &gctx) const {
  auto actsPos = parameters.position(gctx);
  // ATH_MSG_VERBOSE("Acts position: \n"
  //                   << actsPos << " vs trk position: \n"
  //                   << trkparameters.position());
  // ATH_MSG_VERBOSE(parameters.referenceSurface().toString(gctx));
  // ATH_MSG_VERBOSE("GeometryId "<<parameters.referenceSurface().geometryId().value());  

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

// Local functions to check/debug Annulus bounds

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void ActsMeasurementCheck(
    const Acts::GeometryContext &gctx, const Trk::MeasurementBase &measurement,
    const Acts::Surface &surface, const Acts::BoundVector &loc) {
  const Trk::Surface &surf = measurement.associatedSurface();
  // only check Annulus for the moment
  if (surf.bounds().type() != Trk::SurfaceBounds::Annulus) {
    return;
  }
  const auto *bounds = dynamic_cast<const Trk::AnnulusBounds *>(&surf.bounds());
  if (bounds == nullptr) {
    throw std::runtime_error{"Annulus but not XY"};
  }

  Amg::Vector2D locxy = loc.head<2>();

  Acts::ActsMatrix<2, 2> covxy = measurement.localCovariance();

  Amg::Vector3D global = surf.localToGlobal(locxy, Amg::Vector3D{});
  Acts::Vector2 locpc;
  if (auto res = surface.globalToLocal(gctx, global, Acts::Vector3{});
      res.ok()) {
    locpc = *res;
  } else {
    throw std::runtime_error{"Global position not on target surface"};
  }

  // use ACTS jacobian math to convert cluster covariance from cartesian to
  // polar
  auto planeSurface =
      Acts::Surface::makeShared<Acts::PlaneSurface>(surf.transform());
  Acts::BoundVector locxypar;
  locxypar.head<2>() = locxy;
  locxypar[2] = 0;
  locxypar[3] = M_PI_2;
  locxypar[4] = 1;
  locxypar[5] = 1;
  Acts::FreeVector globalxypar = Acts::transformBoundToFreeParameters(
  *planeSurface, gctx, locxypar);
  auto boundToFree = planeSurface->boundToFreeJacobian(
        gctx, globalxypar.segment<3>(Acts::eFreePos0),
        globalxypar.segment<3>(Acts::eFreeDir0));
  Acts::ActsSquareMatrix<2> xyToXyzJac = boundToFree.topLeftCorner<2, 2>();

  Acts::BoundVector locpcpar;
  locpcpar.head<2>() = locpc;
  locpcpar[2] = 0;
  locpcpar[3] = M_PI_2;
  locpcpar[4] = 1;
  locpcpar[5] = 1;
  Acts::FreeVector globalpcpar = Acts::transformBoundToFreeParameters(
  surface, gctx, locpcpar);
 
  boundToFree = surface.boundToFreeJacobian(
        gctx, globalpcpar.segment<3>(Acts::eFreePos0),
  globalpcpar.segment<3>(Acts::eFreeDir0));
  Acts::ActsSquareMatrix<2> pcToXyzJac = boundToFree.topLeftCorner<2, 2>();
  Acts::ActsSquareMatrix<2> xyzToPcJac = pcToXyzJac.inverse();

  // convert cluster covariance
  Acts::ActsMatrix<2, 2> covpc = covxy;
  covpc = xyToXyzJac * covpc * xyToXyzJac.transpose();
  covpc = xyzToPcJac * covpc * xyzToPcJac.transpose();

  std::mt19937 gen{42 + surface.geometryId().value()};
  std::normal_distribution<double> normal{0, 1};
  std::uniform_real_distribution<double> uniform{-1, 1};

  Acts::ActsMatrix<2, 2> lltxy = covxy.llt().matrixL();
  Acts::ActsMatrix<2, 2> lltpc = covpc.llt().matrixL();

  for (size_t i = 0; i < 1e4; i++) {
    std::cout << "ANNULUS COV: ";
    std::cout << surface.geometryId();

    Amg::Vector2D rnd{normal(gen), normal(gen)};

    // XY
    {
      Amg::Vector2D xy = lltxy * rnd + locxy;
      Amg::Vector3D xyz = surf.localToGlobal(xy);
      std::cout << "," << xy.x() << "," << xy.y();
      std::cout << "," << xyz.x() << "," << xyz.y() << "," << xyz.z();
    }
    // PC
    {
      // Amg::Vector2D xy = lltpc * rnd + loc.head<2>();
      Amg::Vector2D rt = lltpc * rnd + locpc;
      Amg::Vector3D xyz = surface.localToGlobal(gctx, rt, Acts::Vector3{});
      // Amg::Vector3D xyz = surface.transform(gctx).rotation() *
      // Acts::Vector3{rt.x(), rt.y(), 0};

      std::cout << "," << rt.x() << "," << rt.y();
      std::cout << "," << xyz.x() << "," << xyz.y() << "," << xyz.z();
    }

    std::cout << std::endl;
  }
}
#pragma GCC diagnostic pop

void ActsTrackParameterCheck(
    const Acts::BoundTrackParameters &actsParameter,
    const Acts::GeometryContext &gctx, const Acts::BoundSquareMatrix &covpc,
    const Acts::BoundVector &targetPars, const Acts::BoundSquareMatrix &targetCov,
    const Trk::PlaneSurface *planeSurface) {

  std::cout << "ANNULUS PAR COV: ";
  std::cout << actsParameter.referenceSurface().geometryId();
  for (unsigned int i = 0; i < 5; i++) {
    for (unsigned int j = 0; j < 5; j++) {
      std::cout << "," << covpc(i, j);
    }
  }
  for (unsigned int i = 0; i < 5; i++) {
    for (unsigned int j = 0; j < 5; j++) {
      std::cout << "," << targetCov(i, j);
    }
  }
  std::cout << std::endl;

  std::mt19937 gen{4242 +
                   actsParameter.referenceSurface().geometryId().value()};
  std::normal_distribution<double> normal{0, 1};

  Acts::ActsMatrix<2, 2> lltxy =
      targetCov.topLeftCorner<2, 2>().llt().matrixL();
  Acts::ActsMatrix<2, 2> lltpc = covpc.topLeftCorner<2, 2>().llt().matrixL();

  for (size_t i = 0; i < 1e4; i++) {
    std::cout << "ANNULUS PAR: ";
    std::cout << actsParameter.referenceSurface().geometryId();

    Acts::ActsVector<2> rnd;
    rnd << normal(gen), normal(gen);

    // XY
    {
      Acts::ActsVector<2> xy =
          lltxy.topLeftCorner<2, 2>() * rnd + targetPars.head<2>();
      Amg::Vector3D xyz;
      planeSurface->localToGlobal(Amg::Vector2D{xy.head<2>()}, Amg::Vector3D{},
                                  xyz);
      for (unsigned int i = 0; i < 2; i++) {
        std::cout << "," << xy[i];
      }
      std::cout << "," << xyz.x() << "," << xyz.y() << "," << xyz.z();
    }
    // PC
    {
      Acts::ActsVector<2> rt = lltpc.topLeftCorner<2, 2>() * rnd +
                               actsParameter.parameters().head<2>();
      Amg::Vector3D xyz = actsParameter.referenceSurface().localToGlobal(
          gctx, Acts::Vector2{rt.head<2>()}, Acts::Vector3{});

      for (unsigned int i = 0; i < 2; i++) {
        std::cout << "," << rt[i];
      }
      std::cout << "," << xyz.x() << "," << xyz.y() << "," << xyz.z();
    }

    std::cout << std::endl;
  }
}

std::unique_ptr<Trk::Track> ActsToTrkConverterTool::convertFitResult(const EventContext& ctx,
                                                                     MutableTrackContainer& tracks,
                                                                     TrackFitResult_t& fitResult,
                                                                     const Trk::TrackInfo::TrackFitter fitAuthor,
                                                                     const detail::SourceLinkType slType) const {
    detail::TrkMeasurementCalibrator measCalib{};
    detail::TrkPrepRawDataCalibrator prdCalib{this, m_ROTcreator.get()};

    if (not fitResult.ok()) {
      ATH_MSG_VERBOSE("Fit did not converge");  
      return nullptr;    
    }
    const Acts::CalibrationContext cctx{getCalibrationContext(ctx)};
    const Acts::GeometryContext gctx{m_trackingGeometryTool->getGeometryContext(ctx).context()};
    Acts::ParticleHypothesis hypothesis{Acts::ParticleHypothesis::pion()};
    
    // Get the fit output object
    const auto& acts_track = fitResult.value();
      
    auto finalTrajectory = std::make_unique<Trk::TrackStates>();
    // initialise the number of dead Pixel and Acts strip
    unsigned int numberOfDeadPixel{0}, numberOfDeadSCT{0};
    int nDoF{0};

    double chi2{0};

    // Loop over all the output state to create track state
    tracks.trackStateContainer().visitBackwards(acts_track.tipIndex(), 
      [&] (const auto &state) -> void {
        // First only consider state with an associated detector element
        const auto* associatedDetEl = dynamic_cast<const IDetectorElementBase*>(
                                        state.referenceSurface().associatedDetectorElement());
       
        if (not associatedDetEl) {
            ATH_MSG_VERBOSE("State is not associated with a measurement sruface");
            return;
        }
        ATH_MSG_VERBOSE("Associated det: "<<to_string(associatedDetEl->detectorType()));
        auto flag = state.typeFlags();
    
        // We need to determine the type of state 
        std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
        std::unique_ptr<Trk::TrackParameters> measPars{};
        std::unique_ptr<Trk::MeasurementBase> measState{};
    
        // State is a hole (no associated measurement), use predicted parameters   
        if (flag.test(Acts::TrackStateFlag::HoleFlag)){
          const Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                                     state.parameters(),
                                                     state.covariance(),
                                                     hypothesis);
          measPars = actsTrackParametersToTrkParameters(actsParam, gctx);
          if (associatedDetEl->detectorType() == DetectorType::Pixel ||
              associatedDetEl->detectorType() == DetectorType::Sct) {
              ATH_MSG_VERBOSE("Check if this is a hole, a dead sensors or a state outside the sensor boundary");
              switch (m_boundaryCheckTool->boundaryCheck(*measPars)) {
                  case Trk::BoundaryCheckResult::DeadElement:
                      numberOfDeadPixel += (associatedDetEl->detectorType() == DetectorType::Pixel);
                      numberOfDeadSCT+= (associatedDetEl->detectorType() == DetectorType::Sct);
                      break;
                  case Trk::BoundaryCheckResult::Candidate:
                      break;
                  default:
                      return;
              }
          }
          typePattern.set(Trk::TrackStateOnSurface::Hole);
        }
        // The state is a measurement state, use smoothed parameters 
        else if (flag.test(Acts::TrackStateFlag::MeasurementFlag)) {
          Acts::BoundTrackParameters actsParam(state.referenceSurface().getSharedPtr(),
                                               state.parameters(),
                                               state.covariance(),
                                               hypothesis);
          typePattern.set(Trk::TrackStateOnSurface::Measurement);
          switch (slType) {
              using enum detail::SourceLinkType;
              case TrkMeasurement: 
                measState = measCalib.unpack(state.getUncalibratedSourceLink())->uniqueClone();
                break;
              case TrkPrepRawData:
                measState = prdCalib.createROT(gctx, cctx, state.getUncalibratedSourceLink(), state);
                break;
              case xAODUnCalibMeas:
                  ATH_MSG_WARNING("Uncalibrated measurement is not implemented");
                  return;
              case nTypes:
                ATH_MSG_WARNING("Invalid type enumaration parsed");
                return;
          }
          nDoF = state.calibratedSize();
          chi2 = state.chi2();
          
        }
        auto perState = std::make_unique<Trk::TrackStateOnSurface>(Trk::FitQualityOnSurface{chi2, nDoF},
                                                                   std::move(measState), 
                                                                   std::move(measPars), nullptr, typePattern);
        // If a state was succesfully created add it to the trajectory 
        ATH_MSG_VERBOSE("State succesfully creates, adding it to the trajectory");
        finalTrajectory->insert(finalTrajectory->begin(), std::move(perState));
      });
      // Convert the perigee state and add it to the trajectory
      const Acts::BoundTrackParameters actsPer(acts_track.referenceSurface().getSharedPtr(), 
                                              acts_track.parameters(), 
                                              acts_track.covariance(),
                                              acts_track.particleHypothesis());
      std::unique_ptr<Trk::TrackParameters> per = actsTrackParametersToTrkParameters(actsPer, gctx);
      std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
      typePattern.set(Trk::TrackStateOnSurface::Perigee);
      auto perState = std::make_unique<Trk::TrackStateOnSurface>(nullptr, std::move(per), nullptr, typePattern);
      finalTrajectory->insert(finalTrajectory->begin(), std::move(perState));
    
      // Create the track using the states
      Trk::TrackInfo newInfo{fitAuthor, Trk::noHypothesis};
      auto newtrack = std::make_unique<Trk::Track>(newInfo, std::move(finalTrajectory), nullptr);
      if (newtrack) {
        // Create the track summary and update the holes information
        if (!newtrack->trackSummary()) {
          newtrack->setTrackSummary(std::make_unique<Trk::TrackSummary>());
          newtrack->trackSummary()->update(Trk::numberOfPixelDeadSensors, numberOfDeadPixel);
          newtrack->trackSummary()->update(Trk::numberOfSCTDeadSensors, numberOfDeadSCT);
        }
        m_trkSummaryTool->updateTrackSummary(ctx, *newtrack, true);
      }
      return newtrack;
  }
}  // namespace ActsTrk
