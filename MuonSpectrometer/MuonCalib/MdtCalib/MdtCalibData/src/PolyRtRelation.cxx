/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MdtCalibData/PolyRtRelation.h>
#include <GeoModelHelpers/throwExcept.h>
#include <cmath>
namespace MuonCalib{
    PolyRtRelation::PolyRtRelation(const ParVec& pars):
        IRtRelation{pars}{
        // check for consistency //
        if (nPar() < 3) {
            THROW_EXCEPTION("PolyRtRelation::_init() - Not enough parameters!");
        }
        if (tLower() >= tUpper()) {
            THROW_EXCEPTION("Lower time boundary >= upper time boundary!");
        }
    }
    double PolyRtRelation::radius(double t) const {
        double r{0.};
        for (unsigned int k =0; k < nPar() -2; ++k) {
            r+= par(k+2) * std::pow(t, k);
        }
        return r;
    }
    double PolyRtRelation::driftVelocity(double t) const{ 
        double v{0.};
        for (unsigned k = 1; k < nPar() -2; ++k) {
            v+= par(k+2) *k*std::pow(t,k-1);
        }
        return v;
    }
    double PolyRtRelation::driftAcceleration(double t) const{ 
        double a{0.};
        for (unsigned int k =2; k<nPar() -2; ++k) {
            a+=par(k+2)*k*(k-1)*std::pow(t, k-2);
        }
        return a;
    }
    double PolyRtRelation::tLower() const { return par(0); }
    double PolyRtRelation::tUpper() const { return par(1); }
}
