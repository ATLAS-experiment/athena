/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibData/RtLegendre.h"
#include "MuonCalibMath/LegendrePoly.h"
#include "GeoModelKernel/throwExcept.h"
using namespace MuonCalib;

RtLegendre::RtLegendre(const ParVec& vec) : 
    IRtRelation(vec) { 
    // check for consistency //
    if (nPar() < 3) {
        THROW_EXCEPTION("RtLegendre::_init() - Not enough parameters!");
    }
    if (tLower() >= tUpper()) {
        THROW_EXCEPTION("Lower time boundary >= upper time boundary!");
    }
}  // end RtLegendre::_init

std::string RtLegendre::name() const { return "RtLegendre"; }
double RtLegendre::tBinWidth() const { return s_tBinWidth; }

double RtLegendre::radius(double t) const {
    ////////////////////////
    // INITIAL TIME CHECK //
    ////////////////////////
    if (t < tLower()) return 0.0;
    if (t > tUpper()) return 14.6;
 
    ///////////////
    // VARIABLES //
    ///////////////
    // argument of the Legendre polynomials
    double x = getReducedTime(t);
    double rad{0.0};  // auxiliary radius

    ////////////////////
    // CALCULATE r(t) //
    ////////////////////
    for (unsigned int k = 0; k < nDoF(); k++) { 
        rad += par(k+2) * legendrePoly(k, x); 
    }
    return std::max(rad, 0.);
}

//*****************************************************************************
double RtLegendre::driftVelocity(double t) const { 
    return (radius(t + 1.0) - radius(t));
    // Set derivative to 0 outside of the bounds
    if (t < tLower() || t > tUpper()) return 0.0;

    // Argument of the Legendre polynomials
    const double x = getReducedTime(t);
    // Chain rule
    const double dx_dt = dReducedTimeDt();
    double drdt{0.};
    for (unsigned int k = 0; k < nDoF(); ++k) {
        drdt += par(k+2) *  legendreDeriv(k, 1, x) * dx_dt;
    }
    return drdt; 
}
double RtLegendre::driftAcceleration(double t) const {
    double acc{0.};
    // Argument of the Legendre polynomials
    const double x = getReducedTime(t);
    const double dx_dt = std::pow(dReducedTimeDt(), 2);
    for (unsigned int k = 0; k < nDoF(); ++k) {
        acc += par(k+2) *  legendreDeriv(k, 2, x) * dx_dt;
    }
    return acc * t;
}
double RtLegendre::tLower() const { return par(0); }
double RtLegendre::tUpper() const { return par(1); }
unsigned int RtLegendre::nDoF() const { return nPar() -2; }

std::vector<double> RtLegendre::rtParameters() const {
    return std::vector<double>{parameters().begin() +2, parameters().end()};
}
