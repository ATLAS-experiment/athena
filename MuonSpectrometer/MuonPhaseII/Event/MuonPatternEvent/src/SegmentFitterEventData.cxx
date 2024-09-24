/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternEvent/SegmentFitterEventData.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <CxxUtils/sincos.h>
#include <vector>
#include <array>
#include <sstream>
#include <format>

namespace MuonR4{
    namespace SegmentFit {
        std::pair<Amg::Vector3D, Amg::Vector3D> makeLine(const Parameters& pars) {
            const CxxUtils::sincos theta{pars[toInt(ParamDefs::theta)]}, phi{pars[toInt(ParamDefs::phi)]}; 
            return std::make_pair(Amg::Vector3D(pars[toInt(ParamDefs::x0)], 
                                                pars[toInt(ParamDefs::y0)],0.),
                                    Amg::Vector3D(phi.cs*theta.sn,phi.sn*theta.sn, theta.cs));
        }
        std::string makeLabel(const Parameters&pars) {
            std::stringstream sstr{};
            sstr<<"x_{0}="<<std::format("{:.2f}", pars[toInt(ParamDefs::x0)])<<", ";
            sstr<<"y_{0}="<<std::format("{:.2f}", pars[toInt(ParamDefs::y0)])<<", ";
            sstr<<std::format("#theta={:.3f}", pars[toInt(ParamDefs::theta)] / Gaudi::Units::deg )<<", ";
            sstr<<std::format("#phi={:.3f}", pars[toInt(ParamDefs::phi)] / Gaudi::Units::deg)<<", ";
            sstr<<"t_{0}="<<std::format("{:.1f}", pars[toInt(ParamDefs::time)]);
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
