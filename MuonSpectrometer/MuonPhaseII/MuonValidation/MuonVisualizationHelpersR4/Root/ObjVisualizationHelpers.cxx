/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "xAODMuonPrepData/CombinedMuonStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurement.h"

#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"

#include "MuonSpacePoint/SpacePoint.h"
#include "MuonPatternEvent/MuonPatternContainer.h"

#include "Acts/Surfaces/detail/PlanarHelper.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Definitions/Units.hpp"


using namespace Acts::PlanarHelper;
using namespace MuonR4;
using namespace Acts::UnitLiterals;

namespace{
    using CovIdx = MuonR4::SpacePoint::CovIdx;
}

namespace  MuonValR4 {
    void drawPropagation(const std::vector<Acts::detail::Step>& steps,
                         Acts::ObjVisualization3D& visualHelper,
                         const Acts::ViewConfig& viewConfig){
        
        if (steps.empty()) {
            return;
        }
        Amg::Vector3D start = steps.front().position;
        for (std::size_t s = 1; s < steps.size(); ++s) {
            Amg::Vector3D end = steps[s].position;
            if ( (end - start).mag() > Acts::s_epsilon){
                Acts::GeometryView3D::drawSegment(visualHelper, start, end, viewConfig);
                start = std::move(end);          
            }

        }
    }
    void drawSegmentLine(const Acts::GeometryContext& tgContext,
                         const MuonR4::Segment& segment,
                         Acts::ObjVisualization3D& visualHelper,
                         const Acts::ViewConfig& viewConfig) {
        /// Take the local z component of the local to global transform as plane normal
        const Amg::Transform3D& locToGlob = segment.msSector()->localToGlobalTransform(tgContext);
        const Amg::Vector3D firstSurfPos = locToGlob*segment.measurements().front()->localPosition();
        const Amg::Vector3D lastSurfPos  = locToGlob*segment.measurements().back()->localPosition();
        const Amg::Vector3D planeNorm = locToGlob.linear().col(2);
        /// Create the intersections
        const auto firstPlaneIsect = intersectPlane(segment.position(), segment.direction(),
                                                    planeNorm, firstSurfPos);
        const auto lastPlaneIsect = intersectPlane(segment.position(), segment.direction(),
                                                    planeNorm, lastSurfPos);
        
        Acts::GeometryView3D::drawSegment(visualHelper,
                                          segment.position() + firstPlaneIsect.pathLength() * segment.direction(),
                                          segment.position() + lastPlaneIsect.pathLength()  * segment.direction(),
                                          viewConfig);
    }
    void drawSegmentLine(const Acts::GeometryContext& tgContext,
                         const xAOD::MuonSegment& segment,
                         Acts::ObjVisualization3D& visualHelper,
                         const Acts::ViewConfig& viewConfig ,
                         const double standardLength){
        
        std::vector<const xAOD::UncalibratedMeasurement*> assocMeas = collectMeasurements(segment, false);
        if (assocMeas.empty()){
            Acts::GeometryView3D::drawSegment(visualHelper,
                                              segment.position() - 0.5 * standardLength * segment.direction(),
                                              segment.position() + 0.5 * standardLength * segment.direction(),
                                              viewConfig);
            return;
        }
        const xAOD::UncalibratedMeasurement* firstMeas = assocMeas.front()->type() != xAOD::UncalibMeasType::Other ?
                                                         assocMeas.front() : assocMeas[1];
        const xAOD::UncalibratedMeasurement* lastMeas = assocMeas.back();

        const Amg::Vector3D firstSurfPos = xAOD::muonSurface(firstMeas).center(tgContext);
        const Amg::Vector3D lastSurfPos  = xAOD::muonSurface(lastMeas).center(tgContext);

        const Segment* detSeg = detailedSegment(segment);
        /// Take the local z component of the local to global transform as plane normal
        const Amg::Vector3D planeNorm = detSeg->msSector()->localToGlobalTransform(tgContext).linear().col(2);
        /// Create the intersections
        const auto firstPlaneIsect = intersectPlane(segment.position(), segment.direction(),
                                                    planeNorm, firstSurfPos);
        const auto lastPlaneIsect = intersectPlane(segment.position(), segment.direction(),
                                                    planeNorm, lastSurfPos);
        
        Acts::GeometryView3D::drawSegment(visualHelper,
                                          segment.position() + firstPlaneIsect.pathLength() * segment.direction(),
                                          segment.position() + lastPlaneIsect.pathLength()  * segment.direction(),
                                          viewConfig);
    }
    void drawSegmentMeasurements(const Acts::GeometryContext& tgContext,
                                 const xAOD::MuonSegment& segment,
                                 Acts::ObjVisualization3D& visualHelper,
                                 const Acts::ViewConfig& viewConfig) {
        std::vector<const xAOD::UncalibratedMeasurement*> assocMeas = collectMeasurements(segment, false);
        for (const xAOD::UncalibratedMeasurement* meas : assocMeas){
            drawMeasurement(tgContext, meas, visualHelper, viewConfig);
        }
    }
    void drawSegmentMeasurements(const Acts::GeometryContext& tgContext,
                                 const MuonR4::Segment& segment,
                                 Acts::ObjVisualization3D& visualHelper,
                                 const Acts::ViewConfig& viewConfig) {
        for (const auto& meas : segment.measurements()) {
            drawSpacePoint(tgContext, *meas, segment.msSector(), visualHelper, viewConfig);
        }
    }
    void drawMeasurement(const Acts::GeometryContext& tgContext,
                         const xAOD::UncalibratedMeasurement* meas,
                         Acts::ObjVisualization3D& visualHelper,
                         const Acts::ViewConfig& viewConfig) {
        
        ActsTrk::detail::xAODUncalibMeasSurfAcc surfAcc{};
        const Acts::Surface& surf{*surfAcc.get(meas)};
        const auto& bounds = surf.bounds();
        if (meas->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            const auto& lBounds = static_cast<const Acts::LineBounds&>(bounds);
            const auto* driftCirc = static_cast<const xAOD::MdtDriftCircle*>(meas);
            const double dR = driftCirc->driftRadius();
            const double hZ = driftCirc->numDimensions() == 1 ? 
                                    lBounds.get(Acts::LineBounds::eHalfLengthZ) :
                                    std::sqrt(meas->localCovariance<2>()(1,1));
            auto newBounds = std::make_unique<Acts::LineBounds>(dR, hZ);
            auto dummySurface = Acts::Surface::makeShared<Acts::StrawSurface>(surf.localToGlobalTransform(tgContext)*
                                                                              Amg::getTranslate3D(driftCirc->localMeasurementPos()),
                                                                              std::move(newBounds));
            Acts::GeometryView3D::drawSurface(visualHelper, *dummySurface, tgContext,
                                              Amg::Isometry3D::Identity(), viewConfig);
            return;
        }
        double dX{0.}, dY{0.};
        Amg::Vector3D locPos{Amg::Vector3D::Zero()};
        switch (meas->numDimensions()) {
            /// Combined pseudo measurement 
            case 0:{
                const auto* cmbMeas = static_cast<const xAOD::CombinedMuonStrip*>(meas);
                dX = std::sqrt(cmbMeas->localCovariance<2>()(0,0));
                dY = std::sqrt(cmbMeas->localCovariance<2>()(1,1));
                locPos.block<2,1>(0,0) = xAOD::toEigen(cmbMeas->localPosition<2>());
                break;
            } case 1:{
                /// Check whether the measurement is a phi measurement or not
                if (meas->type() == xAOD::UncalibMeasType::RpcStripType) {
                    const auto* rpcClus = static_cast<const xAOD::RpcMeasurement*>(meas);
                    locPos = rpcClus->localMeasurementPos();
                    dX = rpcClus->measuresPhi() ? 0.5* rpcClus->readoutElement()->stripPhiLength() 
                                                : std::sqrt(rpcClus->localCovariance<1>()(0,0));
                    dY = rpcClus->measuresPhi() ? std::sqrt(rpcClus->localCovariance<1>()(0,0))
                                                : 0.5* rpcClus->readoutElement()->stripEtaLength();
                } else if (meas->type() == xAOD::UncalibMeasType::TgcStripType) {
                    const auto* tgcClus = static_cast<const xAOD::TgcStrip*>(meas);
                    const MuonGMR4::TgcReadoutElement* re = tgcClus->readoutElement();
                    const auto& stripLay = re->sensorLayout(tgcClus->measurementHash());
                    locPos = tgcClus->localMeasurementPos();
                    dX = tgcClus->measuresPhi() ? 0.5* stripLay->design(true).stripLength(tgcClus->channelNumber())
                                                : std::sqrt(tgcClus->localCovariance<1>()(0,0));
                    dY = tgcClus->measuresPhi() ? std::sqrt(tgcClus->localCovariance<1>()(0,0))
                                                : 0.5* stripLay->design(false).stripLength(tgcClus->channelNumber());
                } else if (meas->type() == xAOD::UncalibMeasType::MMClusterType) {
                    const auto* mmClust = static_cast<const xAOD::MMCluster*>(meas);
                    locPos = mmClust->localMeasurementPos();
                    dX = std::sqrt(mmClust->localCovariance<1>()(0,0));
                    dY = 0.5* mmClust->readoutElement()->stripLength(mmClust->measurementHash());
                } else if(meas->type() == xAOD::UncalibMeasType::sTgcStripType) {
                    const auto* sTgcClus = static_cast<const xAOD::sTgcMeasurement*>(meas);
                    locPos = sTgcClus->localMeasurementPos();
                    if (sTgcClus->channelType() == sTgcIdHelper::sTgcChannelTypes::Strip){
                        dX = std::sqrt(sTgcClus->localCovariance<1>()(0,0));
                        dY = 0.5* sTgcClus->readoutElement()->stripDesign(sTgcClus->measurementHash()).stripLength(sTgcClus->channelNumber());
                    } else if (sTgcClus->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire) {
                        dY = std::sqrt(sTgcClus->localCovariance<1>()(0,0));
                        dX = 0.5*sTgcClus->readoutElement()->wireDesign(sTgcClus->measurementHash()).stripLength(sTgcClus->channelNumber());
                    }
                } else if (meas->type() == xAOD::UncalibMeasType::Other) {
                    const auto* pseudo = static_cast<const xAOD::AuxiliaryMeasurement*>(meas);
                    using ProjectorType = xAOD::AuxiliaryMeasurement::ProjectorType;
                    constexpr double measLength = 1._m;
                    if (pseudo->calibProjector() == ProjectorType::e1DimNoTime) {
                        dX = std::sqrt(meas->localCovariance<1>()(0,0));
                        dY = measLength;
                    } else if (pseudo->calibProjector() == ProjectorType::e1DimRotNoTime) {
                        dY = std::sqrt(meas->localCovariance<1>()(0,0));
                        dX = measLength;
                    }
                }
                break;
            } 
            /// 2D RPC or sTGC pad
            case 2:{
                locPos.block<2,1>(0, 0) = xAOD::toEigen(meas->localPosition<2>());
                dX = std::sqrt(meas->localCovariance<2>()(0,0));
                dY = std::sqrt(meas->localCovariance<2>()(1,1));
                break;
            }
        }

        std::shared_ptr<Acts::Surface> dummySurf{};
        
        if (surf.type() == Acts::Surface::SurfaceType::Straw) {
            auto newBounds = std::make_unique<Acts::LineBounds>(dX, dY);
            dummySurf = Acts::Surface::makeShared<Acts::StrawSurface>(surf.localToGlobalTransform(tgContext)*
                                                                      Amg::getTranslate3D(locPos),
                                                                      std::move(newBounds));
 
        } else {
            auto newBounds = std::make_unique<Acts::RectangleBounds>(dX, dY);
            dummySurf = Acts::Surface::makeShared<Acts::PlaneSurface>(surf.localToGlobalTransform(tgContext)*
                                                                      Amg::getTranslate3D(locPos),
                                                                      std::move(newBounds));

        }
        Acts::GeometryView3D::drawSurface(visualHelper, *dummySurf, tgContext,
                                          Amg::Isometry3D::Identity(), viewConfig);
    }
    void drawBoundParameters(const Acts::GeometryContext& tgContext,
                             const Acts::BoundTrackParameters& pars,
                             Acts::ObjVisualization3D& visualHelper,
                             const Acts::ViewConfig& viewConfig,
                             const double standardLength){
        const Amg::Vector3D globPos = pars.position(tgContext);
        const Amg::Vector3D start = globPos - 0.5 * standardLength * pars.direction();
        const Amg::Vector3D end   = globPos + 0.5 * standardLength * pars.direction();
        Acts::GeometryView3D::drawSegment(visualHelper, start, end, viewConfig);
    }
    void drawSpacePoint(const Acts::GeometryContext& tgContext,
                        const MuonR4::SpacePoint& spacePoint,
                        Acts::ObjVisualization3D& visualHelper,
                        const Acts::ViewConfig& viewConfig) {
        if (spacePoint.dimension() == 2 && !spacePoint.isStraw() &&
            spacePoint.primaryMeasurement() != spacePoint.secondaryMeasurement()) {
            const auto& locToGlob =  spacePoint.msSector()->localToGlobalTransform(tgContext);
            const double dX = std::sqrt(spacePoint.covariance()[Acts::toUnderlying(CovIdx::phiCov)]);
            const double dY = std::sqrt(spacePoint.covariance()[Acts::toUnderlying(CovIdx::etaCov)]);
            auto bounds = std::make_unique<Acts::RectangleBounds>(dX, dY);
            const Acts::Transform3 trf = locToGlob * Amg::getTranslate3D(spacePoint.localPosition());
            auto surf = Acts::Surface::makeShared<Acts::PlaneSurface>(trf, std::move(bounds));
            Acts::GeometryView3D::drawSurface(visualHelper, *surf, tgContext);
        } else {
            drawMeasurement(tgContext, spacePoint.primaryMeasurement(), 
                            visualHelper, viewConfig);
        }
    }
    void drawSpacePoint(const Acts::GeometryContext& tgContext,
                        const MuonR4::CalibratedSpacePoint& spacePoint,
                        const MuonGMR4::SpectrometerSector* msSector,
                        Acts::ObjVisualization3D& visualHelper,
                        const Acts::ViewConfig& viewConfig) {
        if (spacePoint.type() != xAOD::UncalibMeasType::Other) {
            drawSpacePoint(tgContext, *spacePoint.spacePoint(), visualHelper, viewConfig);
            return;
        }
        const auto& locToGlob = msSector->localToGlobalTransform(tgContext);
        if (spacePoint.isStraw()) {
              const double dR = std::sqrt(spacePoint.covariance()[Acts::toUnderlying(CovIdx::etaCov)]);
              const double hZ = std::sqrt(spacePoint.covariance()[Acts::toUnderlying(CovIdx::phiCov)]);
              auto bounds = std::make_unique<Acts::LineBounds>(dR, hZ);
              const Amg::Isometry3D trf = locToGlob * Amg::getTranslate3D(spacePoint.localPosition());
              auto surface = Acts::Surface::makeShared<Acts::StrawSurface>(trf, std::move(bounds));    
              Acts::GeometryView3D::drawSurface(visualHelper, *surface, tgContext);    
        } else {
            const double dX = std::sqrt(spacePoint.covariance()[Acts::toUnderlying(CovIdx::phiCov)]);
            const double dY = std::sqrt(spacePoint.covariance()[Acts::toUnderlying(CovIdx::etaCov)]);
            auto bounds = std::make_unique<Acts::RectangleBounds>(dX, dY);
            const Acts::Transform3 trf = locToGlob * Amg::getTranslate3D(spacePoint.localPosition());
            auto surf = Acts::Surface::makeShared<Acts::PlaneSurface>(trf, std::move(bounds));
            Acts::GeometryView3D::drawSurface(visualHelper, *surf, tgContext);
        }
    }

    
} // namespace  MuonValR4
