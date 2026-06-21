/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternEvent/SegmentFitterEventData.h>

#include <MuonPatternEvent/Segment.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <ActsInterop/UnitConverters.h>
#include <GaudiKernel/PhysicalConstants.h>

#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/UnitVectors.hpp"


#include <sstream>
#include <format>
using namespace Acts;
using namespace Acts::UnitLiterals;

namespace {
    constexpr double straightQoverP = 1. / 20._TeV;
    constexpr double inDeg(const double rad) { return rad / 1._degree; }
}

namespace MuonR4{
    double houghTanBeta(const Amg::Vector3D& v){ 
        constexpr double eps = std::numeric_limits<float>::epsilon();
        return v.y() /  ( std::abs(v.z()) > eps ? v.z() : eps); 
    }
    double houghTanAlpha(const Amg::Vector3D& v){            
        constexpr double eps = std::numeric_limits<float>::epsilon();
        return v.x() /  ( std::abs(v.z()) > eps ? v.z() : eps); 
    }
    namespace SegmentFit {
        constexpr std::size_t N = Acts::toUnderlying(ParamDefs::nPars);
        
        inline std::optional<Acts::BoundMatrix> 
            translateCovariance(const Parameters& locSegPars,
                                std::optional<Covariance>&& localCov,
                                const Amg::Transform3D& localToGlobal) {
            if (!localCov) {
                return std::nullopt;
            }
            const SeedingAux::Line_t line{locSegPars};

            Acts::Matrix<3,2> basisTrf{};
            basisTrf.block<3,1>(0,1) = line.gradient(SeedingAux::Line_t::ParIndex::theta);
            basisTrf.block<3,1>(0,0) = line.gradient(SeedingAux::Line_t::ParIndex::phi);

            BoundMatrix jacobian{BoundMatrix::Identity()};
            jacobian.block<2,2>(Acts::eBoundPhi, Acts::eBoundPhi) = basisTrf.transpose() * 
                                                                    localToGlobal.linear() * 
                                                                    basisTrf;
            BoundMatrix cov{BoundMatrix::Identity()};
            cov.block<N,N>(0,0) = std::move(*localCov);
            BoundMatrix translator{BoundMatrix::Zero()};
            translator( Acts::toUnderlying(ParamDefs::x0), Acts::eBoundLoc0) =
            translator( Acts::toUnderlying(ParamDefs::y0), Acts::eBoundLoc1) =
            translator( Acts::toUnderlying(ParamDefs::theta), Acts::eBoundTheta) =
            translator( Acts::toUnderlying(ParamDefs::phi), Acts::eBoundPhi) =
            translator( Acts::toUnderlying(ParamDefs::t0), Acts::eBoundTime) =
            translator( Acts::toUnderlying(ParamDefs::nPars), Acts::eBoundQOverP) = 1.;
            translator = jacobian * translator;
            return translator.transpose() * cov * translator;
        }

        std::pair<Amg::Vector3D, Amg::Vector3D> makeLine(const Parameters& pars) {
            using enum ParamDefs;
            return std::make_pair(Amg::Vector3D{pars[Acts::toUnderlying(x0)], 
                                                pars[Acts::toUnderlying(y0)],0.},
                                  Acts::makeDirectionFromPhiTheta(pars[Acts::toUnderlying(phi)],
                                                                  pars[Acts::toUnderlying(theta)]));
        }
        Parameters localSegmentPars(const xAOD::MuonSegment& seg) {
            static const xAOD::PosAccessor<N> acc{"localSegPars"};
            Parameters segPars{};
            for (std::size_t p =0 ; p < N; ++p) {
                segPars[p] = acc(seg)[p];
            }   
            return segPars;
        }
        std::optional<Covariance> localSegmentCov(const xAOD::MuonSegment& seg) {
            static const xAOD::PosAccessor<Acts::sumUpToN(N)> acc{"localSegCov"};
            if (!acc.isAvailable(seg)) {
                return std::nullopt;
            }
            Covariance cov{Covariance::Zero()};
            const auto& covVec{acc(seg)};
            for (std::size_t i = 0 ; i < covVec.size(); ++i) {
                const auto [p, p1] = Acts::symMatIndices<N>(i);
                cov(p,p1) = cov(p1,p) = covVec[i];
            }
            return cov;
        }

        Parameters localSegmentPars(const Acts::GeometryContext& tgContext,
                                    const Segment& segment) {
            return localSegmentPars(*tgContext.get<const ActsTrk::GeometryContext*>(), segment);
        }

        Parameters localSegmentPars(const ActsTrk::GeometryContext& gctx,
                                    const Segment& segment) {
            Parameters pars{};
            const Amg::Transform3D globToLoc = segment.msSector()->globalToLocalTransform(gctx);
            const Amg::Vector3D locPos = globToLoc * segment.position();
            const Amg::Vector3D locDir = globToLoc.linear() * segment.direction();
            pars[Acts::toUnderlying(ParamDefs::x0)] = locPos.x();
            pars[Acts::toUnderlying(ParamDefs::y0)] = locPos.y();
            pars[Acts::toUnderlying(ParamDefs::theta)] = locDir.theta();
            pars[Acts::toUnderlying(ParamDefs::phi)] = locDir.phi();
            pars[Acts::toUnderlying(ParamDefs::t0)] = segment.segementT0();
            return pars;
        }

        std::string makeLabel(const Parameters&pars) {
            std::stringstream sstr{};
            sstr<<std::format("x_{{0}}={:.2f}", pars[Acts::toUnderlying(ParamDefs::x0)])<<", ";
            sstr<<std::format("y_{{0}}={:.2f}", pars[Acts::toUnderlying(ParamDefs::y0)])<<", ";
            sstr<<std::format("#theta={:.2f}^{{#circ}}",inDeg(pars[Acts::toUnderlying(ParamDefs::theta)]))<<", ";
            sstr<<std::format("#phi={:.2f}^{{#circ}}",  inDeg(pars[Acts::toUnderlying(ParamDefs::phi)]))<<", ";
            sstr<<std::format("t_{{0}}={:.1f}", pars[Acts::toUnderlying(ParamDefs::t0)]);
            return sstr.str();
        }
        std::string toString(const Parameters& pars) {
            std::stringstream sstr{};
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::x0), pars[Acts::toUnderlying(ParamDefs::x0)]);
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::y0), pars[Acts::toUnderlying(ParamDefs::y0)]);
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::theta), inDeg(pars[Acts::toUnderlying(ParamDefs::theta)]));
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::phi),  inDeg(pars[Acts::toUnderlying(ParamDefs::phi)]));
            sstr<< std::format("{}={:.2f}",toString(ParamDefs::t0), pars[Acts::toUnderlying(ParamDefs::t0)]);
            return sstr.str();
        }
        std::string toString(const ParamDefs a) {
           return SeedingAux::parName(a);
        }

        Acts::BoundTrackParameters boundSegmentPars(const Acts::GeometryContext& tgContext,
                                                    const MuonGMR4::MuonDetectorManager& detMgr,
                                                    const xAOD::MuonSegment& segment,
                                                    const Acts::ParticleHypothesis hypot) {
            return boundSegmentPars(*tgContext.get<const ActsTrk::GeometryContext*>(),
                                     detMgr, segment, hypot);
        }
        Acts::BoundTrackParameters boundSegmentPars(const ActsTrk::GeometryContext& gctx,
                                                    const MuonGMR4::MuonDetectorManager& detMgr,
                                                    const xAOD::MuonSegment& segment,
                                                    const Acts::ParticleHypothesis hypot) {
            
            const auto* msSector = detMgr.getSectorEnvelope(segment.chamberIndex(), 
                                                            segment.sector(),
                                                            segment.etaIndex());

            const Parameters locSegPars = localSegmentPars(segment);
            const Amg::Vector3D globDir = segment.direction();
            
            Acts::BoundVector boundPars{};
            boundPars[Acts::eBoundLoc0] = locSegPars[Acts::toUnderlying(ParamDefs::x0)];
            boundPars[Acts::eBoundLoc1] = locSegPars[Acts::toUnderlying(ParamDefs::y0)];
            boundPars[Acts::eBoundPhi] = globDir.phi();
            boundPars[Acts::eBoundTheta] = globDir.theta();
            boundPars[Acts::eBoundQOverP] = straightQoverP;
            boundPars[Acts::eBoundTime] = ActsTrk::timeToActs(segment.position().mag() / Gaudi::Units::c_light +  segment.t0());
            
            // std::optional<Acts::BoundMatrix> cov = translateCovariance(localSegmentPars()
            return Acts::BoundTrackParameters{msSector->surface().getSharedPtr(), 
                                              std::move(boundPars),
                                              translateCovariance(locSegPars, localSegmentCov(segment),
                                                                  msSector->localToGlobalTransform(gctx)), hypot};
        }
        Acts::BoundTrackParameters boundSegmentPars(const ActsTrk::GeometryContext& gctx,
                                                    const Segment& segment,
                                                    const Acts::ParticleHypothesis hypot) {
 
            const Amg::Vector3D locPos = segment.msSector()->globalToLocalTransform(gctx) * segment.position();
            Acts::BoundVector boundPars{};
            boundPars[Acts::eBoundLoc0] = locPos.x();
            boundPars[Acts::eBoundLoc1] = locPos.y();
            boundPars[Acts::eBoundPhi] = segment.direction().phi();
            boundPars[Acts::eBoundTheta] = segment.direction().theta();
            boundPars[Acts::eBoundQOverP] = straightQoverP;
            boundPars[Acts::eBoundTime] = ActsTrk::timeToActs(segment.position().mag() / Gaudi::Units::c_light + 
                                                              segment.segementT0());
           
            return Acts::BoundTrackParameters{segment.msSector()->surface().getSharedPtr(), 
                                              std::move(boundPars),
                                              translateCovariance(localSegmentPars(gctx, segment),
                                                                  segment.covariance(),
                                                                 segment.msSector()->localToGlobalTransform(gctx)), hypot};
        }
       
    }
}
