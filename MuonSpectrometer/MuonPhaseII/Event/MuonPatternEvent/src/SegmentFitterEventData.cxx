/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternEvent/SegmentFitterEventData.h>

#include <MuonPatternEvent/Segment.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <CxxUtils/sincos.h>
#include <vector>
#include <array>
#include <sstream>
#include <format>
using namespace Acts;
namespace MuonR4{
    double houghTanTheta(const Amg::Vector3D& v){ 
        constexpr double eps = std::numeric_limits<float>::epsilon();
        return v.y() /  ( std::abs(v.z()) > eps ? v.z() : eps); 
    }
    double houghTanPhi(const Amg::Vector3D& v){            
        constexpr double eps = std::numeric_limits<float>::epsilon();
        return v.x() /  ( std::abs(v.z()) > eps ? v.z() : eps); 
    }
    namespace SegmentFit {
        Amg::Vector3D dirFromTangents(const double tanPhi, const double tanTheta) {
            return Amg::Vector3D(tanPhi, tanTheta, 1.).unit();
        }
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
        Parameters localSegmentPars(const ActsGeometryContext& gctx,
                                    const Segment& segment) {
            Parameters pars{};
            const Amg::Transform3D globToLoc = segment.msSector()->globalToLocalTrans(gctx);
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
           return Acts::Experimental::detail::CompSpacePointAuxiliaries::parName(a);
        }
       
    }
}
