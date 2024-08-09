/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternEvent/SegmentFitterEventData.h>


#include <ActsGeometryInterfaces/ActsGeometryContext.h>

#include <vector>
#include <array>
#include <sstream>
namespace MuonR4{
    namespace SegmentFit {
        std::pair<Amg::Vector3D, Amg::Vector3D> makeLine(const Parameters& pars) {
            return std::make_pair(Amg::Vector3D(pars[toInt(AxisDefs::x0)], 
                                                pars[toInt(AxisDefs::y0)],0.),
                                    Amg::Vector3D(pars[toInt(AxisDefs::tanPhi)],
                                                pars[toInt(AxisDefs::tanTheta)], 1.).unit());
        }
        std::string makeLabel(const Parameters&pars) {
            std::stringstream sstr{};
            sstr<<"x_{0}="<<pars[toInt(AxisDefs::x0)]<<", y_{0}="<<pars[toInt(AxisDefs::y0)];
            sstr<<", tan(#theta)="<<pars[toInt(AxisDefs::tanTheta)]<<", tan(#phi)="<<pars[toInt(AxisDefs::tanPhi)];
            sstr<<", t_{0}="<<pars[toInt(AxisDefs::time)];
            return sstr.str();
        }
        std::string toString(const Parameters& pars) {
            std::stringstream sstr{};
            sstr<<"x0="<<pars[toInt(AxisDefs::x0)]<<", y0="<<pars[toInt(AxisDefs::y0)];
            sstr<<", tanTheta="<<pars[toInt(AxisDefs::tanTheta)]<<", tanPhi="<<pars[toInt(AxisDefs::tanPhi)];
            sstr<<", time="<<pars[toInt(AxisDefs::time)];
            return sstr.str();
        }
    }
}
