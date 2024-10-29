/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MdtCalibData/RootRtRelation.h>
#include <GeoModelHelpers/throwExcept.h>
#include <cmath>
namespace MuonCalib{
    std::string RootRtRelation::name() const { return "RootRtRelation"; }
    RootRtRelation::RootRtRelation(const ParVec& pars):
        IRtRelation{pars}{
        // check for consistency //
        if (nPar() < 3) {
            THROW_EXCEPTION("PolyRtRelation::_init() - Not enough parameters!");
        }
        if (tLower() >= tUpper()) {
            THROW_EXCEPTION("Lower time boundary >= upper time boundary!");
        }
    }
    double RootRtRelation::radius(double t) const {
        double r{0.};
        for (unsigned int k = 0; k < nPar() -2; ++k) {
            r+= par(k+2) * std::pow( k% 2 ? std::abs(t) : t, 0.5 + k);
        }
        return r;
    }
    double RootRtRelation::driftVelocity(double t) const {
        double v{0.};
        for (unsigned int k = 0; k < nPar() -2; ++k) {
            v+= par(k+2) *(0.5 +k)*  std::pow( k% 2 ? std::abs(t) : t, 0.5 + (k -1));
        }
        return v;
    }
    double RootRtRelation::driftAcceleration(double t) const {
        double a{0.};
        for (unsigned int k = 0; k < nPar() -2; ++k) {
            a+= par(k+2) *(0.5 +k)*(0.5 + k -1) * std::pow( k% 2 ? std::abs(t) : t, 0.5 + (k -2));
        }
        return a;
    }
    double RootRtRelation::tLower() const {return par(0); }
    double RootRtRelation::tUpper() const {return par(1); } 
}