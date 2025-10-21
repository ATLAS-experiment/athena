/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
        const auto tgContext = gctx->context();

        auto extrapolate = [&](const Acts::BoundTrackParameters& start,
                               const MuonR4::SpacePoint& sp) {
            const auto& trf = sp.msSector()->localToGlobalTrans(*gctx);
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

            const auto* detEl = static_cast<const ActsTrk::IDetectorElementBase*>(target.associatedDetectorElement());
            ATH_MSG_VERBOSE("Propagate "<<Amg::toString(start.position(tgContext))<<" + "
                  <<Amg::toString(start.direction())<<" onto surface: "<<target.toString(tgContext)
                  <<"\n, "<<m_idHelperSvc->toString(detEl->identify())
                  << " geoId: "<<target.geometryId()<<", "<<( lambda.value_or(0.) > 0 ? "forward" : "backward"));
           
            return  m_extrapolationTool->propagate(ctx, start, target, lambda.value_or(0.) > 0 
                                                                ? Acts::Direction::Forward() 
                                                                : Acts::Direction::Backward(), 100._m);

        };
        for (const xAOD::MuonSegment* segment : *segments) {
            const MuonR4::Segment* detSeg = MuonR4::detailedSegment(*segment);

            const auto segPars = localSegmentPars(*segment);
            const auto [locPos, locDir] = makeLine(segPars);

            const MuonGMR4::SpectrometerSector* sector = m_detMgr->getSectorEnvelope(segment->chamberIndex(), 
                                                                                     segment->sector(), 
                                                                                     segment->etaIndex());
            auto startPars = boundSegmentPars(*gctx, *detSeg);
            
            if (msgLvl(MSG::VERBOSE)) {

                std::stringstream sstr{};
                sstr<<"pos: "<<Amg::toString(segment->position())<<", dir: "
                    <<Amg::toString(segment->direction())<<", chi2/nDoF: "
                    <<segment->chiSquared() / segment->numberDoF()<<", nDoF: "<<segment->numberDoF()<<", "
                    <<segment->nPrecisionHits()<<", "<<segment->nPhiLayers()<<std::endl;
                for (const auto& meas : detSeg->measurements()) {
                    sstr<<"  **** "<<(*meas)<<", chi2: "<<SeedingAux::chi2Term(locPos, locDir, *meas)
                        <<", sign: "<<(meas->isStraw() ? 
                                (SeedingAux::strawSign(locPos,locDir, *meas) == 1 ? "R" : "L") : "-")
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
                Amg::Vector2D lPos{Amg::Vector2D::Zero()};
                const auto trf = targetSurf.transform(tgContext).inverse() *
                                 sector->surface().transform(tgContext);
                if (targetSurf.type() == Acts::Surface::SurfaceType::Plane) {
                    lPos = (trf * SeedingAux::extrapolateToPlane(locPos, locDir, *meas)).block<2,1>(0,0);
                    if (!bounds.inside(lPos, Acts::BoundaryTolerance::AbsoluteEuclidean(-2._mm))){
                        ATH_MSG_WARNING("The position "<<Amg::toString(lPos)
                                <<" is outside the trapezoid "<<bounds
                                <<" "<<m_idHelperSvc->toString(sp->identify()));
                            continue;
                    }
                } else if (targetSurf.type() == Acts::Surface::SurfaceType::Straw) {
                    const auto cIsect = lineIntersect<3>(meas->localPosition(),
                                                         meas->sensorDirection(), 
                                                         locPos, locDir);
                    const auto cIsectPos = cIsect.position();
                    const auto cPos = trf * cIsectPos;
                    lPos[0] = cPos.perp() * SeedingAux::strawSign(locPos, locDir, *meas);
                    lPos[1] = cPos.z();
                    const auto& lBounds = static_cast<const Acts::LineBounds&>(bounds);
                    if (std::abs(cPos.z()) > lBounds.get(Acts::LineBounds::eHalfLengthZ) - 2._cm ||
                        cPos.perp() >lBounds.get(Acts::LineBounds::eR)  - 0.2_mm) {
                        ATH_MSG_WARNING("The line is not on measurement "
                            <<m_idHelperSvc->toString(sp->identify())
                            <<" "<<Amg::toString(lPos)<<" vs. "<<bounds<<".");
                        continue;
                    }
                }
                auto extpPars = extrapolate(startPars, *sp);
                if (!extpPars) {
                   ATH_MSG_FATAL("Failed to propagte to "<<(*meas)
                                <<",\n lPos: "<<Amg::toString(trf * meas->localPosition())
                                <<", expected: "<<Amg::toString(lPos)<<", "<<targetSurf.bounds());
                   return StatusCode::FAILURE;
                }
                if (targetSurf.type() == Acts::Surface::SurfaceType::Plane) {
                    ATH_MSG_DEBUG("Position on "<<m_idHelperSvc->toString(sp->identify()) 
                                <<" plane "<<Amg::toString(lPos)<<" vs. "
                                <<Amg::toString((*extpPars).localPosition()));
                    const Amg::Vector2D dPos = (*extpPars).localPosition() - lPos;
                    if (dPos.mag() > 0.1_mm) {
                        ATH_MSG_FATAL("Too large deviation on "<<m_idHelperSvc->toString(sp->identify())
                                    <<", "<<Amg::toString(dPos));
                        return StatusCode::FAILURE;
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
                    if (std::abs(std::abs(dist) - std::abs(extDist))  / std::sqrt(cov) > 0.05 ||
                        std::abs(lPos[1] - extLocZ) > 0.1_mm) {
                        ATH_MSG_FATAL("Too large deviation on "<<m_idHelperSvc->toString(sp->identify())
                                    <<", "<<Amg::toString(lPos)<<" vs. ("<<extDist<<", "<<extLocZ<<")"
                                    <<", deviate R: "<<(std::abs(std::abs(dist) - std::abs(extDist))  / std::sqrt(cov))
                                    <<", deviate Z: "<<std::abs(lPos[1] - extLocZ));
                        return StatusCode::FAILURE;
                    }
                }
            }

        }

        return StatusCode::SUCCESS;
    }

}
