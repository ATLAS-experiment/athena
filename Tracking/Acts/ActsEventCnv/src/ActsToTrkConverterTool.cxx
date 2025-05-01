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
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometry/ActsTrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsGeoUtils/SurfaceCache.h"
#include "ActsInterop/IdentityHelper.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/TransformationHelpers.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Propagator/detail/JacobianEngine.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "Acts/EventData/TrackStatePropMask.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "ActsGeometry/ATLASSourceLink.h"

// STL
#include <cmath>
#include <iostream>
#include <memory>
#include <random>

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

}  // namespace ActsTrk


StatusCode ActsTrk::ActsToTrkConverterTool::initialize() {
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
      if (actsElement->detectorType() == ActsTrk::DetectorType::Trt) {
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
  ATH_CHECK(m_muonMgrKey.initialize(m_extractMuonSurfaces));
  if (m_extractMuonSurfaces){
    ATH_CHECK(m_idHelperSvc.retrieve());
    const MuonGMR4::MuonDetectorManager* muonMgr{nullptr};    
    ATH_CHECK(detStore()->retrieve(muonMgr));
    unsigned int mapSize = m_actsSurfaceMap.size(); // For debugging message later
    for (auto readoutElement : muonMgr->getAllReadoutElements()) {
      std::vector<std::shared_ptr<Acts::Surface>> reSurfaces = readoutElement->getSurfaces();
      for ( const auto& surf : reSurfaces) {
        const Identifier id = static_cast<const ActsTrk::SurfaceCache*>(surf->associatedDetectorElement())->identify();
        m_actsSurfaceMap.insert(std::make_pair(id, surf.get()));
      }
    }
    ATH_MSG_VERBOSE("After adding muon surfaces, the map has grown from "<<mapSize<<" to "<<m_actsSurfaceMap.size());
  }
  return StatusCode::SUCCESS;
}

const Trk::Surface &ActsTrk::ActsToTrkConverterTool::actsSurfaceToTrkSurface(
    const Acts::Surface &actsSurface) const {

  const auto *detEleBase= dynamic_cast<const IDetectorElementBase*>(actsSurface.associatedDetectorElement());
  if (!detEleBase) {
    throw std::domain_error("No ATLAS surface corresponding to to the Acts one");
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

  throw std::domain_error("No ATLAS surface corresponding to to the Acts one");
}

const Acts::Surface &ActsTrk::ActsToTrkConverterTool::trkSurfaceToActsSurface(
    const Trk::Surface &atlasSurface) const {

  Identifier atlasID = atlasSurface.associatedDetectorElementIdentifier();
  auto it = m_actsSurfaceMap.find(atlasID);
  if (it != m_actsSurfaceMap.end()) {
    return *it->second;
  }
  ATH_MSG_ERROR("No Acts surface corresponding to this ATLAS surface:");
  ATH_MSG_ERROR(atlasSurface);
  throw std::domain_error("No Acts surface corresponding to the ATLAS one");
}

Acts::SourceLink ActsTrk::ActsToTrkConverterTool::trkMeasurementToSourceLink(
    [[maybe_unused]] const Acts::GeometryContext& gctx,
    const Trk::MeasurementBase &measurement) const
{
   return Acts::SourceLink(&measurement);
}


std::vector<Acts::SourceLink>
ActsTrk::ActsToTrkConverterTool::trkTrackToSourceLinks(
    const Acts::GeometryContext &gctx, const Trk::Track &track) const {
  auto hits = track.measurementsOnTrack();
  auto outliers = track.outliersOnTrack();

  std::vector<Acts::SourceLink> sourceLinks;
  sourceLinks.reserve(hits->size() + outliers->size());

  for (auto it = hits->begin(); it != hits->end(); ++it) {
     sourceLinks.push_back(trkMeasurementToSourceLink(gctx, **it));
  }
  for (auto it = outliers->begin(); it != outliers->end(); ++it) {
    sourceLinks.push_back(trkMeasurementToSourceLink(gctx, **it));
  }
  return sourceLinks;
}


const Acts::BoundTrackParameters
ActsTrk::ActsToTrkConverterTool::trkTrackParametersToActsParameters(
    const Trk::TrackParameters &atlasParameter, const Acts::GeometryContext & gctx, Trk::ParticleHypothesis hypothesis) const {

  using namespace Acts::UnitLiterals;
  std::shared_ptr<const Acts::Surface> actsSurface;
  Acts::BoundVector params;

  // get the associated surface
  if (atlasParameter.hasSurface() &&
      atlasParameter.associatedSurface().owner() == Trk::SurfaceOwner::DetElOwn) {
    try {
      actsSurface = trkSurfaceToActsSurface(atlasParameter.associatedSurface())
                        .getSharedPtr();
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

    switch (atlasParameter.associatedSurface().type()){
      case Trk::SurfaceType::Plane:
        actsSurface = Acts::Surface::makeShared<const Acts::PlaneSurface>(
            atlasParameter.associatedSurface().transform());
        break;
      case Trk::SurfaceType::Perigee:
        actsSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(
            atlasParameter.associatedSurface().transform());
        break;
      // TODO - implement the missing types?
      default:
        ATH_MSG_WARNING("No surface type found for this Trk::Surface. Creating a perigee surface.");
        actsSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(
            atlasParameter.associatedSurface().center());
    }
  }

  // Construct track parameters
  auto atlasParam = atlasParameter.parameters();
  if (actsSurface->bounds().type() ==
      Acts::SurfaceBounds::BoundsType::eAnnulus) {
    // Annulus surfaces are constructed differently in Acts/Trk so we need to
    // convert local coordinates
    auto position = atlasParameter.position();
    auto result =
        actsSurface->globalToLocal(gctx, position, atlasParameter.momentum());
    if (result.ok()) {
      params << (*result)[0], (*result)[1], atlasParam[Trk::phi0],
          atlasParam[Trk::theta],
          atlasParameter.charge() / (atlasParameter.momentum().mag() * 1_MeV),
          0.;
    } else {
      ATH_MSG_WARNING(
          "Unable to convert annulus surface - globalToLocal failed");
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

  // convert hypotheses
  float mass = Trk::ParticleMasses::mass[hypothesis] * Acts::UnitConstants::MeV;
  Acts::PdgParticle absPdg = Acts::makeAbsolutePdgParticle(
      static_cast<Acts::PdgParticle>(
        m_pdgToParticleHypothesis.convert(hypothesis, atlasParameter.charge())));
  Acts::ParticleHypothesis actsHypothesis{
    absPdg, mass, Acts::AnyCharge{std::abs(static_cast<float>(atlasParameter.charge()))}};

  return Acts::BoundTrackParameters(actsSurface, params,
                                    cov, actsHypothesis);
}

std::unique_ptr<Trk::TrackParameters>
ActsTrk::ActsToTrkConverterTool::actsTrackParametersToTrkParameters(
    const Acts::BoundTrackParameters &actsParameter,
    const Acts::GeometryContext &gctx) const {

  using namespace Acts::UnitLiterals;
  std::optional<AmgSymMatrix(5)> cov = std::nullopt;
  if (actsParameter.covariance()) {
    AmgSymMatrix(5)
        newcov(actsParameter.covariance()->topLeftCorner<5, 5>());
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
      const Trk::ConeSurface &coneSurface =
          dynamic_cast<const Trk::ConeSurface &>(
              actsSurfaceToTrkSurface(actsSurface));
      return std::make_unique<Trk::AtaCone>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, coneSurface, cov);

      break;
    }
    case Acts::Surface::SurfaceType::Cylinder: {
      const Trk::CylinderSurface &cylSurface =
          dynamic_cast<const Trk::CylinderSurface &>(
              actsSurfaceToTrkSurface(actsSurface));
      return std::make_unique<Trk::AtaCylinder>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, cylSurface, cov);
      break;
    }
    case Acts::Surface::SurfaceType::Disc: {
      if (const auto *discSurface = dynamic_cast<const Trk::DiscSurface *>(
              &actsSurfaceToTrkSurface(actsSurface));
          discSurface != nullptr) {
        return std::make_unique<Trk::AtaDisc>(
            actsParameter.get<Acts::eBoundLoc0>(),
            actsParameter.get<Acts::eBoundLoc1>(),
            actsParameter.get<Acts::eBoundPhi>(),
            actsParameter.get<Acts::eBoundTheta>(),
            actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, *discSurface, cov);
      } else if (const auto *planeSurface =
                     dynamic_cast<const Trk::PlaneSurface *>(
                         &actsSurfaceToTrkSurface(actsSurface));
                 planeSurface != nullptr) {
        // need to convert to plane position on plane surface (annulus bounds)
        auto helperSurface = Acts::Surface::makeShared<Acts::PlaneSurface>(
            planeSurface->transform());

        auto covpc = actsParameter.covariance().value();

        Acts::FreeVector freePars =
            Acts::transformBoundToFreeParameters(
                actsSurface, gctx, actsParameter.parameters());

        Acts::BoundVector targetPars =
            Acts::transformFreeToBoundParameters(freePars,
                                                         *helperSurface, gctx)
                .value();
        
        Acts::FreeMatrix freeTransportJacobian = Acts::FreeMatrix::Identity();

        Acts::FreeVector freeToPathDerivatives = Acts::FreeVector::Zero();
        freeToPathDerivatives.head<3>() = freePars.segment<3>(Acts::eFreeDir0);

	auto boundToFreeJacobian = actsSurface.boundToFreeJacobian(
	    gctx, freePars.segment<3>(Acts::eFreePos0),
	    freePars.segment<3>(Acts::eFreeDir0));

        Acts::BoundMatrix boundToBoundJac = Acts::detail::boundToBoundTransportJacobian(
            gctx, freePars, boundToFreeJacobian, freeTransportJacobian,
            freeToPathDerivatives, *helperSurface);

        Acts::BoundMatrix targetCov = 
          boundToBoundJac * covpc * boundToBoundJac.transpose();

        auto pars = std::make_unique<Trk::AtaPlane>(
            targetPars[Acts::eBoundLoc0], targetPars[Acts::eBoundLoc1],
            targetPars[Acts::eBoundPhi], targetPars[Acts::eBoundTheta],
            targetPars[Acts::eBoundQOverP] * 1_MeV, *planeSurface,
            targetCov.topLeftCorner<5, 5>());

        if (m_visualDebugOutput) {
          ActsTrackParameterCheck(actsParameter, gctx, covpc, targetPars,
                                  targetCov, planeSurface);
        }

        return pars;

      } else {
        throw std::runtime_error{
            "Acts::DiscSurface is not associated with ATLAS disc or plane "
            "surface"};
      }
      break;
    }
    case Acts::Surface::SurfaceType::Perigee: {
      const Trk::PerigeeSurface perSurface(actsSurface.center(gctx));
      return std::make_unique<Trk::Perigee>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, perSurface, cov);

      break;
    }
    case Acts::Surface::SurfaceType::Plane: {
      const Trk::PlaneSurface &plaSurface =
          dynamic_cast<const Trk::PlaneSurface &>(
              actsSurfaceToTrkSurface(actsSurface));
      return std::make_unique<Trk::AtaPlane>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, plaSurface, cov);
      break;
    }
    case Acts::Surface::SurfaceType::Straw: {
      const Trk::StraightLineSurface &lineSurface =
          dynamic_cast<const Trk::StraightLineSurface &>(
              actsSurfaceToTrkSurface(actsSurface));
      return std::make_unique<Trk::AtaStraightLine>(
          actsParameter.get<Acts::eBoundLoc0>(),
          actsParameter.get<Acts::eBoundLoc1>(),
          actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, lineSurface, cov);
      break;
    }
    case Acts::Surface::SurfaceType::Curvilinear: {
      return std::make_unique<Trk::CurvilinearParameters>(
          actsParameter.position(gctx), actsParameter.get<Acts::eBoundPhi>(),
          actsParameter.get<Acts::eBoundTheta>(),
          actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, cov);
      break;
    }
    case Acts::Surface::SurfaceType::Other: {
      break;
    }
  }

  throw std::domain_error("Surface type not found");
}

void ActsTrk::ActsToTrkConverterTool::trkTrackCollectionToActsTrackContainer(
    ActsTrk::MutableTrackContainer &tc,
    const TrackCollection &trackColl,
    const Acts::GeometryContext & gctx) const {
  ATH_MSG_VERBOSE("Calling trkTrackCollectionToActsTrackContainer with "
                  << trackColl.size() << " tracks.");
  unsigned int trkCount = 0;
  std::vector<Identifier> failedIds; // Keep track of Identifiers of failed conversions
  for (auto trk : trackColl) {
    // Do conversions!
    const Trk::TrackStates *trackStates =
        trk->trackStateOnSurfaces();
    if (!trackStates) {
      ATH_MSG_WARNING("No track states on surfaces found for this track.");
      continue;
    }

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
    for (auto tsos : *trackStates) {

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
      auto actsTSOS = trackStateContainer.getTrackState(
          trackStateContainer.addTrackState(mask, index));
      ATH_MSG_VERBOSE("TipIndex: " << actsTrack.tipIndex()
                                   << " TSOS index within trajectory: "
                                   << actsTSOS.index());
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
            actsTrack.setReferenceSurface(
                parameters.referenceSurface().getSharedPtr());
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
        actsTSOS.setUncalibratedSourceLink(Acts::SourceLink(static_cast<ActsTrk::ATLASSourceLink>(tsos->measurementOnTrack())));

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

bool ActsTrk::ActsToTrkConverterTool::actsTrackParameterPositionCheck(
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
static void ActsTrk::ActsMeasurementCheck(
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

void ActsTrk::ActsTrackParameterCheck(
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

