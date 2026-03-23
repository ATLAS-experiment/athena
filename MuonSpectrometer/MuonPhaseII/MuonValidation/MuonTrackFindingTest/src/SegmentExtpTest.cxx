/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SegmentExtpTest.h"

#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "ActsInterop/UnitConverters.h"

#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"

#include "MuonTrackEvent/TrackingHelpers.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"

#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"

#include "ActsInterop/Logger.h"

using namespace Acts::UnitLiterals;
using namespace MuonR4::SegmentFit;
using namespace Acts::detail::LineHelper;

namespace MuonValR4{
    StatusCode SegmentExtpTest::initialize(){
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    StatusCode SegmentExtpTest::execute(const EventContext& ctx) const {
        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_readKey, ctx));
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        const Acts::GeometryContext tgContext{gctx->context()};

        auto extrapolate = [&](const Acts::BoundTrackParameters& start,
                               const MuonR4::SpacePoint& sp) {
            const Amg::Transform3D& trf = sp.msSector()->localToGlobalTransform(*gctx);
            const Acts::Surface& target = xAOD::muonSurface(sp.primaryMeasurement());
            const Amg::Vector3D n = target.normal(tgContext, 
                                                  Amg::Vector3D::Zero(), 
                                                  Amg::Vector3D::Zero());
                                                      
            auto lambda = sp.isStraw() ? Amg::intersect<3>(trf * sp.localPosition(),
                                                           trf.linear() * sp.sensorDirection(),
                                                           start.position(tgContext),
                                                           start.direction())
                                        : Amg::intersect<3>(start.position(tgContext),
                                                            start.direction(), n,
                                                             n.dot(target.center(tgContext)));

            const auto* detEl = static_cast<const ActsTrk::IDetectorElementBase*>(target.surfacePlacement());
            ATH_MSG_VERBOSE("Propagate "<<Amg::toString(start.position(tgContext))<<" + "
                  <<Amg::toString(start.direction())<<" onto surface: "<<target.toString(tgContext)
                  <<"\n, "<<m_idHelperSvc->toString(detEl->identify())
                  << " geoId: "<<target.geometryId()<<", "<<( lambda.value_or(0.) > 0 ? "forward" : "backward"));
           
            return m_extrapolationTool->propagate(ctx, start, target, lambda.value_or(0.) > 0 
                                                            ? Acts::Direction::Forward() 
                                                            : Acts::Direction::Backward(), 100._m);

        };
        StatusCode retCode = StatusCode::SUCCESS;

        SeedingAux::Config cfg{};
        cfg.parsToUse.clear();
        SeedingAux pullCalculator{cfg, makeActsAthenaLogger(this, "PullCalculator")};

        SeedingAux::Line_t line{};

        for (const xAOD::MuonSegment* segment : *segments) {
            const MuonR4::Segment* detSeg = MuonR4::detailedSegment(*segment);

            line.updateParameters(localSegmentPars(*segment));


            const MuonGMR4::SpectrometerSector* sector = m_detMgr->getSectorEnvelope(segment->chamberIndex(), 
                                                                                     segment->sector(), 
                                                                                     segment->etaIndex());
            auto startPars = boundSegmentPars(*gctx, *detSeg);

            Acts::ObjVisualization3D visualHelper{};
            if (m_drawEvent) {
                /// Draw the reference segment as a red line
                drawSegmentLine(*gctx, *segment, visualHelper,
                                Acts::ViewConfig{.color = {220, 0, 0}});
                drawSegmentMeasurements(*gctx, *segment, visualHelper, Acts::s_viewSurface);
            }
            
            if (msgLvl(MSG::VERBOSE)) {
                std::stringstream sstr{};
                sstr<<"pos: "<<Amg::toString(segment->position())
                    <<", dir: "<<Amg::toString(segment->direction())
                    <<", chi2/nDoF: "<<segment->chiSquared() / segment->numberDoF()
                    <<", nDoF: "<<segment->numberDoF()<<", "
                    <<segment->nPrecisionHits()<<", "<<segment->nPhiLayers()<<std::endl;
                for (const auto& meas : detSeg->measurements()) {
                    sstr<<"  **** "<<(*meas)<<", chi2: "<<SeedingAux::chi2Term(line, *meas)
                        <<", sign: "<<(meas->isStraw() ? 
                                (SeedingAux::strawSign(line, *meas) == 1 ? "R" : "L") : "-")
                        <<", geoId: "<<(meas->type() != xAOD::UncalibMeasType::Other ? 
                                           xAOD::muonSurface(meas->spacePoint()->primaryMeasurement()).geometryId()
                                        :  Acts::GeometryIdentifier{})
                        <<std::endl;
                }
                ATH_MSG_VERBOSE("Run propagation test on "<<sector->identString()<<std::endl<<sstr.str());
            }
            for (const auto& meas: detSeg->measurements()) {
                if (!meas->spacePoint() || 
                    meas->fitState() != MuonR4::CalibratedSpacePoint::State::Valid) {
                  continue;
                }
                const auto sp = meas->spacePoint();
                const Acts::Surface& targetSurf{xAOD::muonSurface(sp->primaryMeasurement())};
               
                const auto& bounds = targetSurf.bounds();
                Amg::Vector2D lPos{Amg::Vector2D::Zero()}, mPos{Amg::Vector2D::Zero()};
                const Amg::Transform3D toSurf = targetSurf.localToGlobalTransform(tgContext).inverse() *
                                                sector->surface().localToGlobalTransform(tgContext);
                if (targetSurf.type() == Acts::Surface::SurfaceType::Plane) {
                    lPos = (toSurf * SeedingAux::extrapolateToPlane(line, *meas)).block<2,1>(0,0);
                    if (!bounds.inside(lPos, Acts::BoundaryTolerance::AbsoluteEuclidean(-2._mm))){
                        ATH_MSG_WARNING("The position "<<Amg::toString(lPos)
                                <<" is outside the trapezoid "<<bounds
                                <<" "<<m_idHelperSvc->toString(sp->identify()));
                            continue;
                    }
                    mPos = (toSurf * meas->localPosition()).block<2,1>(0,0);
                } else if (targetSurf.type() == Acts::Surface::SurfaceType::Straw) {
                    const auto cIsect = lineIntersect<3>(meas->localPosition(), meas->sensorDirection(), 
                                                         line.position(), line.direction());
                    const auto cIsectPos = cIsect.position();
                    const Amg::Vector3D closePos = toSurf * cIsectPos;
                    lPos[0] = Acts::copySign(closePos.perp(), SeedingAux::strawSign(line, *meas));
                    lPos[1] = closePos.z();
                    mPos[0] = Acts::copySign(meas->driftRadius(), lPos[0]);
                    if (meas->measuresPhi()) {
                        mPos[1] = meas->localPosition().x();
                    }
                    const auto& lBounds = static_cast<const Acts::LineBounds&>(bounds);
                    if (std::abs(closePos.z()) > lBounds.get(Acts::LineBounds::eHalfLengthZ) - 2._cm ||
                        closePos.perp() >lBounds.get(Acts::LineBounds::eR)  - 0.2_mm) {
                        ATH_MSG_WARNING("The line does not cross tube "
                            <<m_idHelperSvc->toString(sp->identify())
                            <<" "<<Amg::toString(lPos)<<" vs. "<<bounds<<".");
                        continue;
                    }
                }
                auto extpPars = extrapolate(startPars, *sp);
                if (!extpPars.ok()) {
                   ATH_MSG_FATAL("Failed to propagate to "<<(*meas)
                                <<",\n lPos: "<<Amg::toString(toSurf * meas->localPosition())
                                <<", expected: "<<Amg::toString(lPos)<<", "<<targetSurf.bounds());
                   retCode = StatusCode::FAILURE;
                   continue;
                }
                if (m_drawEvent) {
                    /// Draw the true intersection from the extrapolator as blue lines
                    drawBoundParameters(*gctx, *extpPars, visualHelper,
                                        Acts::ViewConfig{.color = {0, 0, 220}}, 6._cm); 
                }

                if (targetSurf.type() == Acts::Surface::SurfaceType::Plane) {
                    ATH_MSG_DEBUG("Position on "<<m_idHelperSvc->toString(sp->identify()) 
                                <<" plane "<<Amg::toString(lPos)<<" vs. "
                                <<Amg::toString((*extpPars).localPosition()));
                    const Amg::Vector2D dPos = (*extpPars).localPosition() - lPos;
                    if (dPos.mag() > 0.1_mm) {
                        ATH_MSG_FATAL("Too large deviation on "<<m_idHelperSvc->toString(sp->identify())
                                    <<", "<<Amg::toString(dPos));
                        retCode = StatusCode::FAILURE;
                    }       
                } else if (targetSurf.type() == Acts::Surface::SurfaceType::Straw) {
                    const double dist = lPos[0];
                    const double extDist = (*extpPars).parameters()[Acts::eBoundLoc0];
                    const double extLocZ = (*extpPars).parameters()[Acts::eBoundLoc1];
                    const double cov = meas->covariance()[Acts::toUnderlying(AxisDefs::etaCov)];
                    ATH_MSG_DEBUG("Distance on surface "<<m_idHelperSvc->toString(sp->identify())
                                   <<" straight: "<<dist<<", extrapolated: "<<extDist
                                <<"--> "<<(extDist - dist ) / std::sqrt(cov)
                                <<", along the tube: "<<lPos[1]<<", extrapolated: "<<extLocZ);
                    if (std::abs(dist - extDist)  / std::sqrt(cov) > 0.05 ||
                        std::abs(lPos[1] - extLocZ) > 0.1_mm) {
                        ATH_MSG_FATAL("Too large deviation on "<<m_idHelperSvc->toString(sp->identify())
                                    <<", "<<Amg::toString(lPos)<<" vs. ("<<extDist<<", "<<extLocZ<<")"
                                    <<", deviate R: "<<(std::abs(dist -extDist)  / std::sqrt(cov))
                                    <<", deviate Z: "<<std::abs(lPos[1] - extLocZ));
                        retCode = StatusCode::FAILURE;
                    }
                }

                pullCalculator.updateSpatialResidual(line, *meas);
                const Amg::Vector2D res = mPos - lPos;
                AmgSymMatrix(2) cov{AmgSymMatrix(2)::Identity()};
                cov(0,0) = 1./meas->covariance()[1];
                cov(1,1) = 1./meas->covariance()[0];
                ATH_MSG_DEBUG("Analyze residual for "<<m_idHelperSvc->toString(sp->identify())
                <<" / "<<targetSurf.geometryId()<<" --- measurement: "<<Amg::toString(mPos)<<", extraploated: "
                    <<Amg::toString(lPos)<<" --> residual: "<<Amg::toString(res)<<" vs. "
                    <<Amg::toString(pullCalculator.residual())<<", chi2: "<<pullCalculator.chi2Term(line, *meas)
                    <<" vs. "<<res.dot(cov*res));
            }
            if (m_drawEvent) {
                visualHelper.write(std::format("ExtTpTest_{:}_{:}_{:}.obj", 
                                          ctx.eventID().event_number(), segment->index(), 
                                          MuonR4::printID(*segment)));
            }
        }

        return retCode;
    }

}
