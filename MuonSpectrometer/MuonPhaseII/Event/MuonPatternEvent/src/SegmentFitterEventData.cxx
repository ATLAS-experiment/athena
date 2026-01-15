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


#include <sstream>
#include <format>
using namespace Acts;
using namespace Acts::UnitLiterals;

namespace {
    constexpr double straightQoverP = 1. / 20._TeV;
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
        std::pair<Amg::Vector3D, Amg::Vector3D> makeLine(const Parameters& pars) {
            using enum ParamDefs;
            return std::make_pair(Amg::Vector3D(pars[toUnderlying(x0)], 
                                                pars[toUnderlying(y0)],0.),
                                  Amg::dirFromAngles(pars[toUnderlying(phi)],
                                                     pars[toUnderlying(theta)]));
        }
        Parameters localSegmentPars(const xAOD::MuonSegment& seg) {
            static const SG::Accessor<xAOD::MeasVector<toUnderlying(ParamDefs::nPars)>> acc{"localSegPars"};
            Parameters segPars{};
            for (std::size_t p =0 ; p < segPars.size(); ++p) {
                segPars[p] = acc(seg)[p];
            }
            return segPars;
        }
        Parameters localSegmentPars(const ActsTrk::GeometryContext& gctx,
                                    const Segment& segment) {
            Parameters pars{};
            const Amg::Transform3D globToLoc = segment.msSector()->globalToLocalTransform(gctx);
            const Amg::Vector3D locPos = globToLoc * segment.position();
            const Amg::Vector3D locDir = globToLoc.linear() * segment.direction();
            pars[toUnderlying(ParamDefs::x0)] = locPos.x();
            pars[toUnderlying(ParamDefs::y0)] = locPos.y();
            pars[toUnderlying(ParamDefs::theta)] = locDir.theta();
            pars[toUnderlying(ParamDefs::phi)] = locDir.phi();
            pars[toUnderlying(ParamDefs::t0)] = segment.segementT0();
            return pars;
        }

        std::string makeLabel(const Parameters&pars) {
            std::stringstream sstr{};
            sstr<<std::format("x_{{0}}={:.2f}", pars[toUnderlying(ParamDefs::x0)])<<", ";
            sstr<<std::format("y_{{0}}={:.2f}", pars[toUnderlying(ParamDefs::y0)])<<", ";
            sstr<<std::format("#theta={:.2f}^{{#circ}}", pars[toUnderlying(ParamDefs::theta)] / Gaudi::Units::deg )<<", ";
            sstr<<std::format("#phi={:.2f}^{{#circ}}", pars[toUnderlying(ParamDefs::phi)] / Gaudi::Units::deg)<<", ";
            sstr<<std::format("t_{{0}}={:.1f}", pars[toUnderlying(ParamDefs::t0)]);
            return sstr.str();
        }
        std::string toString(const Parameters& pars) {
            std::stringstream sstr{};
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::x0), pars[toUnderlying(ParamDefs::x0)]);
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::y0), pars[toUnderlying(ParamDefs::y0)]);
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::theta), pars[toUnderlying(ParamDefs::theta)]/Gaudi::Units::deg);
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::phi),  pars[toUnderlying(ParamDefs::phi)]/Gaudi::Units::deg);
            sstr<< std::format("{}={:.2f}",toString(ParamDefs::t0), pars[toUnderlying(ParamDefs::t0)]);
            return sstr.str();
        }
        std::string toString(const ParamDefs a) {
           return SeedingAux::parName(a);
        }
        Acts::BoundTrackParameters boundSegmentPars(const MuonGMR4::MuonDetectorManager& detMgr,
                                                    const xAOD::MuonSegment& segment,
                                                    std::optional<Acts::BoundMatrix> cov,
                                                    Acts::ParticleHypothesis hypot) {
            
            const auto* msSector = detMgr.getSectorEnvelope(segment.chamberIndex(), 
                                                            segment.sector(),
                                                            segment.etaIndex());
            const Acts::Surface& surface = msSector->surface();

            const auto locSegPars = localSegmentPars(segment);

            const Amg::Vector3D globDir = segment.direction();

            
            Acts::BoundVector boundPars{};
            boundPars[Acts::eBoundLoc0] = locSegPars[toUnderlying(ParamDefs::x0)];
            boundPars[Acts::eBoundLoc1] = locSegPars[toUnderlying(ParamDefs::y0)];
            boundPars[Acts::eBoundPhi] = globDir.phi();
            boundPars[Acts::eBoundTheta] = globDir.theta();
            boundPars[Acts::eBoundQOverP] = straightQoverP;
            boundPars[Acts::eBoundTime] = ActsTrk::timeToActs(segment.position().mag() / Gaudi::Units::c_light + 
                                                              segment.t0());

            return Acts::BoundTrackParameters{surface.getSharedPtr(), std::move(boundPars),
                                              cov, hypot};
        }
        Acts::BoundTrackParameters boundSegmentPars(const ActsTrk::GeometryContext& gctx,
                                                    const Segment& segment,
                                                    const Acts::ParticleHypothesis hypot) {
            const auto& surface = segment.msSector()->surface();

            const Amg::Vector3D locPos = surface.transform(gctx.context()).inverse() * 
                                         segment.position();
             Acts::BoundVector boundPars{};
            boundPars[Acts::eBoundLoc0] = locPos.x();
            boundPars[Acts::eBoundLoc1] = locPos.y();
            boundPars[Acts::eBoundPhi] = segment.direction().phi();
            boundPars[Acts::eBoundTheta] = segment.direction().theta();
            boundPars[Acts::eBoundQOverP] = straightQoverP;
            boundPars[Acts::eBoundTime] = ActsTrk::timeToActs(segment.position().mag() / Gaudi::Units::c_light + 
                                                              segment.segementT0());
            Acts::BoundMatrix cov{Acts::BoundMatrix::Identity()};

            return Acts::BoundTrackParameters{surface.getSharedPtr(), std::move(boundPars),
                                              cov, hypot};
        }
       
    }
}
