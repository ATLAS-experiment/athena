/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeometryRealmConvTool.h"

#include "ActsGeometryInterfaces/IDetectorElement.h"
#include "ActsGeometry/ActsDetectorElement.h"

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
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/DiscSurface.h"

#include "MuonReadoutGeometry/MuonReadoutElement.h"

using namespace Acts::UnitLiterals;
using SurfacePtr_t = ActsTrk::GeometryRealmConvTool::SurfacePtr_t;

namespace ActsTrk{
    GeometryRealmConvTool::~GeometryRealmConvTool() = default;
    StatusCode GeometryRealmConvTool::initialize() {
        if (parent() != toolSvc()) {
              ATH_MSG_ERROR("The tool is initialized as a private tool but should be public");
              return StatusCode::FAILURE;
        }
        ATH_CHECK(m_trackingGeometryTool.retrieve());
        m_trackingGeometryTool->trackingGeometry()->visitSurfaces([&](const Acts::Surface *surface) {
             // find acts surface with the same detector element ID
             if (!surface->isSensitive()) {
                return;
            }
            const auto *actsElement = dynamic_cast<const IDetectorElementBase*>(surface->surfacePlacement());
            if (!actsElement) {
               return;
            }
            // Conversion from Acts to ATLAS surface impossible for the TRT so the TRT
            // surfaces are not stored in this map
            if (actsElement->detectorType() == DetectorType::Trt) {
                return;
            }
            auto [it, ok] = m_actsSurfaceMap.insert(std::make_pair(actsElement->identify(), surface->getSharedPtr()));
            if (!ok) {
                ATH_MSG_WARNING("ATLAS ID " << actsElement->identify()
                                            << " has two ACTS surfaces: "
                                            << it->second->geometryId() << " and "
                                            << surface->geometryId());
            }
        });
        ATH_CHECK(m_muonMgrKey.initialize(m_extractMuonSurfaces));
        return StatusCode::SUCCESS;
    } 
    std::shared_ptr<Trk::SurfaceBounds> 
        GeometryRealmConvTool::translateBounds(const Acts::SurfaceBounds& bounds) const {
        switch (bounds.type()) {
            using enum Acts::SurfaceBounds::BoundsType;
            case eRectangle:{
              using ParEnum_t = Acts::RectangleBounds::BoundValues;
              const auto& cBounds = static_cast<const Acts::RectangleBounds&>(bounds);
              return std::make_shared<Trk::RectangleBounds>(cBounds.get(ParEnum_t::eMaxX), 
                                                            cBounds.get(ParEnum_t::eMaxY));
            } case eTrapezoid: {
                using ParEnum_t = Acts::TrapezoidBounds::BoundValues;
                const auto& cBounds = static_cast<const Acts::TrapezoidBounds&>(bounds);
                return std::make_shared<Trk::TrapezoidBounds>(cBounds.get(ParEnum_t::eHalfLengthXnegY),
                                                              cBounds.get(ParEnum_t::eHalfLengthXposY),
                                                              cBounds.get(ParEnum_t::eHalfLengthY));
          } case eDisc: {
            using ParEnum_t = Acts::RadialBounds::BoundValues;
            const auto& cBounds = static_cast<const Acts::RadialBounds&>(bounds);
            return std::make_shared<Trk::DiscBounds>(cBounds.get(ParEnum_t::eMinR),
                                                     cBounds.get(ParEnum_t::eMaxR),
                                                     cBounds.get(ParEnum_t::eAveragePhi),
                                                     cBounds.get(ParEnum_t::eHalfPhiSector));
        } case eCylinder: {
            using ParEnum_t = Acts::CylinderBounds::BoundValues;
            const auto& cBounds = static_cast<const Acts::CylinderBounds&>(bounds);
            return std::make_shared<Trk::CylinderBounds>(cBounds.get(ParEnum_t::eR),
                                                         cBounds.get(ParEnum_t::eHalfPhiSector),
                                                         cBounds.get(ParEnum_t::eAveragePhi),
                                                         cBounds.get(ParEnum_t::eHalfLengthZ));
        } case eLine: {
            using ParEnum_t = Acts::LineBounds::BoundValues;
            const auto& cBounds = static_cast<const Acts::LineBounds&>(bounds);
            return std::make_shared<Trk::CylinderBounds>(cBounds.get(ParEnum_t::eR),
                                                         cBounds.get(ParEnum_t::eHalfLengthZ));
        } case eDiamond: {
            using ParEnum_t = Acts::DiamondBounds::BoundValues;
            const auto& cBounds = static_cast<const Acts::DiamondBounds&>(bounds);
            return std::make_shared<Trk::DiamondBounds>(cBounds.get(ParEnum_t::eHalfLengthXnegY),
                                                        cBounds.get(ParEnum_t::eHalfLengthXzeroY),
                                                        cBounds.get(ParEnum_t::eHalfLengthXposY),
                                                        cBounds.get(ParEnum_t::eHalfLengthYneg),
                                                        cBounds.get(ParEnum_t::eHalfLengthYpos));
        } default:
            break;
        }
        THROW_EXCEPTION("The bounds "<<bounds<<" cannot be translated");
        return nullptr;
    }
    SurfacePtr_t GeometryRealmConvTool::translateFreeSurface(const EventContext& ctx,
                                                             const Acts::Surface& surface) const {
        const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
        const Amg::Transform3D& trf{surface.localToGlobalTransform(tgContext)};
        switch (surface.type()) {
            using enum Acts::Surface::SurfaceType;
            case Plane:
              return SurfacePtr_t{new Trk::PlaneSurface(trf, translateBounds(surface.bounds()))};
            case Cylinder:
                return SurfacePtr_t{new Trk::CylinderSurface(trf,
                       std::dynamic_pointer_cast<Trk::CylinderBounds>(translateBounds(surface.bounds())))};
            case Perigee:
                return SurfacePtr_t{new Trk::PerigeeSurface(trf)};
            case Disc:
                return SurfacePtr_t{new Trk::DiscSurface(trf,
                       std::dynamic_pointer_cast<Trk::DiscBounds>(translateBounds(surface.bounds())))};
            case Straw: {
                auto bounds = std::dynamic_pointer_cast<Trk::CylinderBounds>(translateBounds(surface.bounds()));
                return SurfacePtr_t{new Trk::StraightLineSurface(trf, bounds->r(), bounds->halflengthZ())};
            } default:
              break;
        }
        THROW_EXCEPTION("ActsToTrkConverterTool() - Surface cannot be translated " 
                        <<surface.toString(tgContext));
 
        return nullptr;
    }

    SurfacePtr_t GeometryRealmConvTool::convertSurfaceToTrk(const EventContext& ctx,
                                                                const Acts::Surface& actsSurface) const{
        const auto *detEleBase= dynamic_cast<const IDetectorElementBase*>(actsSurface.surfacePlacement());
        if (!detEleBase) {
           return translateFreeSurface(ctx, actsSurface);
        }
        switch (detEleBase->detectorType()) {
            using enum DetectorType;
            case Pixel:
            case Sct:
            case Hgtd:
            case Trt: {
                const auto actsElement = dynamic_cast<const ActsDetectorElement*>(detEleBase);
                if (actsElement) {
                    return SurfacePtr_t{&actsElement->atlasSurface()};
                }
                break;
            } case Mdt:
            case Rpc:
            case Tgc:
            case Csc:
            case sTgc:
            case Mm: {
                const MuonGM::MuonDetectorManager* detMgr{nullptr};
                if (!SG::get(detMgr, m_muonMgrKey, ctx).isSuccess() || !detMgr) {
                    THROW_EXCEPTION("Failed to retrieve the muon detector manager");
                }
                return SurfacePtr_t{&detMgr->getReadoutElement(detEleBase->identify())->surface(detEleBase->identify())};
            } default:
                break;
        }
        /// Produce a new free surface
        return translateFreeSurface(ctx, actsSurface);
    }
    std::shared_ptr<const Acts::Surface> GeometryRealmConvTool::convertSurfaceToActs(const Trk::Surface& atlasSurface) const {
        Identifier atlasID = atlasSurface.associatedDetectorElementIdentifier();
        auto it = m_actsSurfaceMap.find(atlasID);
        if (it != m_actsSurfaceMap.end()) {
          return it->second;
        }
        const Amg::Transform3D& trf{atlasSurface.transform()};
        switch (atlasSurface.type()){
            using enum Trk::SurfaceType;
            case Plane:
                return Acts::Surface::makeShared<Acts::PlaneSurface>(trf);
            case Perigee:
                return Acts::Surface::makeShared<Acts::PerigeeSurface>(trf);
            case Line:
                return Acts::Surface::makeShared<Acts::StrawSurface>(trf);
            // TODO - implement the missing types?
            default: {
              break;
            }
        }
        std::stringstream surfStr{};
        atlasSurface.dump(surfStr);  
        throw std::domain_error(std::format("Failed to translate surface {:}", surfStr.str()));
    }
 
    Acts::BoundTrackParameters 
            GeometryRealmConvTool::convertTrackParametersToActs(const EventContext& ctx,
                                                                      const Trk::TrackParameters& atlasParameter, 
                                                                       Trk::ParticleHypothesis hypothesis) const {

        std::shared_ptr<const Acts::Surface> actsSurface{};
        Acts::BoundVector params{};
        const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();

        // get the associated surface
        try {
            actsSurface = convertSurfaceToActs(atlasParameter.associatedSurface());
        } catch (const std::exception &e) {
            ATH_MSG_ERROR("Could not find ACTS detector surface for this TrackParameter:");
            ATH_MSG_ERROR(atlasParameter);
            throw;  // Nothing we can do, so just pass exception on...
        }

        // Construct track parameters
        const auto& atlasParam{atlasParameter.parameters()};
        if (actsSurface->bounds().type() == Acts::SurfaceBounds::BoundsType::eAnnulus) {
            // Annulus surfaces are constructed differently in Acts/Trk so we need to
            // convert local coordinates
            const Amg::Vector3D& position{atlasParameter.position()};
            auto result = actsSurface->globalToLocal(tgContext, position, atlasParameter.momentum());
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

        std::optional<Acts::BoundMatrix> cov{};
        if (atlasParameter.covariance()) {
            cov = Acts::BoundMatrix::Identity();
            cov->topLeftCorner(5, 5) = *atlasParameter.covariance();

            // Convert the covariance matrix from MeV
            // FIXME: This needs to handle the annulus case as well - currently the cov
            // is wrong for annulus surfaces
            for (int i = 0; i < cov->rows(); ++i) {
                (*cov)(i, 4) = (*cov)(i, 4) / 1_MeV;
            }
            for (int i = 0; i < cov->cols(); ++i) {
                (*cov)(4, i) = (*cov)(4, i) / 1_MeV;
            }
        }
        return Acts::BoundTrackParameters{actsSurface, params, std::move(cov), 
                                          ParticleHypothesis::convert(hypothesis)};
    }
    std::unique_ptr<Trk::TrackParameters> 
        GeometryRealmConvTool::convertTrackParametersToTrk(const EventContext& ctx,
                                                                  const Acts::BoundTrackParameters& actsParameter) const {
                                                                  
                                                                  
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
            cov = newcov;
        }

        const Acts::Surface &actsSurface = actsParameter.referenceSurface();
        SurfacePtr_t trkSurface = convertSurfaceToTrk(ctx, actsSurface);
        switch (actsSurface.type()) {
            case Acts::Surface::SurfaceType::Cone: {
                const auto &coneSurface = static_cast<const Trk::ConeSurface&>(*trkSurface);
                return std::make_unique<Trk::AtaCone>(
                    actsParameter.get<Acts::eBoundLoc0>(),
                    actsParameter.get<Acts::eBoundLoc1>(),
                    actsParameter.get<Acts::eBoundPhi>(),
                    actsParameter.get<Acts::eBoundTheta>(),
                    actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, coneSurface, cov);
            } case Acts::Surface::SurfaceType::Cylinder: {
                const auto &cylSurface{static_cast<const Trk::CylinderSurface&>(*trkSurface)};
                return std::make_unique<Trk::AtaCylinder>(
                    actsParameter.get<Acts::eBoundLoc0>(),
                    actsParameter.get<Acts::eBoundLoc1>(),
                    actsParameter.get<Acts::eBoundPhi>(),
                    actsParameter.get<Acts::eBoundTheta>(),
                    actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, cylSurface, cov);
            } case Acts::Surface::SurfaceType::Disc: {
                if (trkSurface->type() == Trk::SurfaceType::Disc) {
                    const auto& discSurface{static_cast<const Trk::DiscSurface&>(*trkSurface)};
                    return std::make_unique<Trk::AtaDisc>(
                        actsParameter.get<Acts::eBoundLoc0>(),
                        actsParameter.get<Acts::eBoundLoc1>(),
                        actsParameter.get<Acts::eBoundPhi>(),
                        actsParameter.get<Acts::eBoundTheta>(),
                        actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, discSurface, cov);
                } else if (trkSurface->type() == Trk::SurfaceType::Plane) {
                    const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
                    auto& planeSurface{static_cast<const Trk::PlaneSurface&>(*trkSurface)};
                    // need to convert to plane position on plane surface (annulus bounds)
                    auto helperSurface = Acts::Surface::makeShared<Acts::PlaneSurface>(planeSurface.transform());

                    auto covpc = actsParameter.covariance().value();
                    /// Convert to free parameters
                    Acts::FreeVector freePars = Acts::transformBoundToFreeParameters(actsSurface, tgContext, 
                                                                                     actsParameter.parameters());

                    /// Back conversion to bound parameters of the helper plane
                    Acts::BoundVector targetPars = Acts::transformFreeToBoundParameters(freePars,
                                                            *helperSurface, tgContext).value();

                                                            
                    Acts::FreeMatrix freeTransportJacobian{Acts::FreeMatrix::Identity()};

                    Acts::FreeVector freeToPathDerivatives{Acts::FreeVector::Zero()};
                    freeToPathDerivatives.head<3>() = freePars.segment<3>(Acts::eFreeDir0);

                    auto boundToFreeJacobian = actsSurface.boundToFreeJacobian(tgContext, 
                                                                               freePars.segment<3>(Acts::eFreePos0),
                                                                               freePars.segment<3>(Acts::eFreeDir0));

                    Acts::BoundMatrix boundToBoundJac = 
                            Acts::detail::boundToBoundTransportJacobian(tgContext, freePars, 
                                                                         boundToFreeJacobian, freeTransportJacobian, 
                                                                         freeToPathDerivatives, *helperSurface);

                    Acts::BoundMatrix targetCov{boundToBoundJac * covpc * boundToBoundJac.transpose()};

                    return std::make_unique<Trk::AtaPlane>(
                        targetPars[Acts::eBoundLoc0], targetPars[Acts::eBoundLoc1],
                        targetPars[Acts::eBoundPhi], targetPars[Acts::eBoundTheta],
                        targetPars[Acts::eBoundQOverP] * 1_MeV, planeSurface,
                        targetCov.topLeftCorner<5, 5>());
                } else {
                    throw std::domain_error("Acts::DiscSurface is not associated with ATLAS disc or plane surface");
                }
                break;
            } case Acts::Surface::SurfaceType::Perigee: {
                const auto& perSurface = static_cast<const Trk::PerigeeSurface&>(*trkSurface);
                return std::make_unique<Trk::Perigee>(
                    actsParameter.get<Acts::eBoundLoc0>(),
                    actsParameter.get<Acts::eBoundLoc1>(),
                    actsParameter.get<Acts::eBoundPhi>(),
                    actsParameter.get<Acts::eBoundTheta>(),
                    actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, perSurface, cov);
            } case Acts::Surface::SurfaceType::Plane: {
                auto &plaSurface{static_cast<const Trk::PlaneSurface&>(*trkSurface)};
                return std::make_unique<Trk::AtaPlane>(
                    actsParameter.get<Acts::eBoundLoc0>(),
                    actsParameter.get<Acts::eBoundLoc1>(),
                    actsParameter.get<Acts::eBoundPhi>(),
                    actsParameter.get<Acts::eBoundTheta>(),
                    actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, plaSurface, cov);
            } case Acts::Surface::SurfaceType::Straw: {
                auto& lineSurface{static_cast<const Trk::StraightLineSurface&>(*trkSurface)};
                return std::make_unique<Trk::AtaStraightLine>(
                    actsParameter.get<Acts::eBoundLoc0>(),
                    actsParameter.get<Acts::eBoundLoc1>(),
                    actsParameter.get<Acts::eBoundPhi>(),
                    actsParameter.get<Acts::eBoundTheta>(),
                    actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, lineSurface, cov);
            } case Acts::Surface::SurfaceType::Curvilinear: {
                const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
                return std::make_unique<Trk::CurvilinearParameters>(
                    actsParameter.position(tgContext), actsParameter.get<Acts::eBoundPhi>(),
                    actsParameter.get<Acts::eBoundTheta>(),
                    actsParameter.get<Acts::eBoundQOverP>() * 1_MeV, cov);
            } case Acts::Surface::SurfaceType::Other: {
                break;
            }
        }
        throw std::domain_error("Surface type not found");
    }
}
