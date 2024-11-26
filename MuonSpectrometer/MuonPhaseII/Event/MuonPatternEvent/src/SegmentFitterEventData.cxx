/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternEvent/SegmentFitterEventData.h>
#include <MuonPatternEvent/Segment.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <CxxUtils/sincos.h>
#include <vector>
#include <array>
#include <sstream>
#include <format>

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
            return std::make_pair(Amg::Vector3D(pars[toInt(ParamDefs::x0)], 
                                                pars[toInt(ParamDefs::y0)],0.),
                                  Amg::dirFromAngles(pars[toInt(ParamDefs::phi)],
                                                     pars[toInt(ParamDefs::theta)]));
        }
        Parameters localSegmentPars(const xAOD::MuonSegment& seg) {
            static const SG::Accessor<xAOD::MeasVector<toInt(ParamDefs::nPars)>> acc{"localSegPars"};
            return xAOD::toEigen(xAOD::ConstVectorMap<toInt(ParamDefs::nPars)>{acc(seg).data()});
        }
        Parameters localSegmentPars(const ActsGeometryContext& gctx,
                                    const Segment& segment) {
            Parameters pars{Parameters::Zero()};
            const Amg::Transform3D globToLoc = segment.msSector()->globalToLocalTrans(gctx);
            const Amg::Vector3D locPos = globToLoc * segment.position();
            const Amg::Vector3D locDir = globToLoc.linear() * segment.direction();
            pars[toInt(ParamDefs::x0)] = locPos.x();
            pars[toInt(ParamDefs::y0)] = locPos.y();
            pars[toInt(ParamDefs::theta)] = locDir.theta();
            pars[toInt(ParamDefs::phi)] = locDir.phi();
            pars[toInt(ParamDefs::time)] = segment.segementT0();
            return pars;
        }

        std::string makeLabel(const Parameters&pars) {
            std::stringstream sstr{};
            sstr<<std::format("x_{{0}}={:.2f}", pars[toInt(ParamDefs::x0)])<<", ";
            sstr<<std::format("y_{{0}}={:.2f}", pars[toInt(ParamDefs::y0)])<<", ";
            sstr<<std::format("#theta={:.2f}^{{#circ}}", pars[toInt(ParamDefs::theta)] / Gaudi::Units::deg )<<", ";
            sstr<<std::format("#phi={:.2f}^{{#circ}}", pars[toInt(ParamDefs::phi)] / Gaudi::Units::deg)<<", ";
            sstr<<std::format("t_{{0}}={:.1f}", pars[toInt(ParamDefs::time)]);
            return sstr.str();
        }
        std::string toString(const Parameters& pars) {
            std::stringstream sstr{};
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::x0), pars[toInt(ParamDefs::x0)]);
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::y0), pars[toInt(ParamDefs::y0)]);
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::theta), pars[toInt(ParamDefs::theta)]/Gaudi::Units::deg);
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::phi),  pars[toInt(ParamDefs::phi)]/Gaudi::Units::deg);
            sstr<< std::format("{}={:.2f}, ",toString(ParamDefs::time), pars[toInt(ParamDefs::time)]);
            return sstr.str();
        }
        std::string toString(const ParamDefs a) {
            switch (a){
                case ParamDefs::x0:{
                    return "x0";
                    break;
                } case ParamDefs::y0: {
                    return "y0";
                    break;
                } case ParamDefs::theta: {
                    return "theta";
                    break;
                } case ParamDefs::phi: {
                    return "phi";
                    break;
                } case ParamDefs::time: {
                    return "time";
                    break;
                } case ParamDefs::nPars:
                    break;
            }
            return "";
        }
       
    }
}
