/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibData/RtChebyshev.h"
#include "Acts/Utilities/detail/Polynomials.hpp"
#include "GeoModelKernel/throwExcept.h"
using namespace MuonCalib;

RtChebyshev::RtChebyshev(const ParVec& vec) : 
    IRtRelation(vec) { 
    // check for consistency //
    if (nPar() < 3) {
        THROW_EXCEPTION("RtChebyshev::_init() - Not enough parameters!");
    }
    if (tLower() >= tUpper()) {
        THROW_EXCEPTION("Lower time boundary ("<<tLower()<<")>= upper time ("<<tUpper()<<") boundary!");
    }
}  // end RtChebyshev::_init

std::string RtChebyshev::name() const { return "RtChebyshev"; }
double RtChebyshev::tBinWidth() const { return s_tBinWidth; }

double RtChebyshev::radius(double t) const {
    ////////////////////////
    // INITIAL TIME CHECK //
    ////////////////////////
    if (t < tLower()) return 0.0;
    if (t > tUpper()) return 14.6;
 
    ///////////////
    // VARIABLES //
    ///////////////
    // argument of the Chebyshev polynomials
    double x = getReducedTime(t);
    double rad{0.0};  // auxiliary radius

    ////////////////////
    // CALCULATE r(t) //
    ////////////////////
    for (unsigned int k = 0; k < nDoF(); k++) { 
        rad += par(k+2) * Acts::detail::chebychevPolyTn(x, k); 
    }
    return std::max(rad, 0.);
}

//*****************************************************************************
double RtChebyshev::driftVelocity(double t) const { 
    // Set derivative to 0 outside of the bounds
    if (t < tLower() || t > tUpper()) return 0.0;

    // Argument of the Chebyshev polynomials
    const double x = getReducedTime(t);
    // Chain rule
    const double dx_dt = dReducedTimeDt();
    double drdt{0.};
    for (unsigned int k = 1; k < nDoF(); ++k) {
        // Calculate the contribution to dr/dt using k * U_{k-1}(x) * dx/dt
        drdt += par(k+2) * Acts::detail::chebychevPolyTn(x, k, 1)  * dx_dt;
    }
    return drdt; 
}
double RtChebyshev::driftAcceleration(double t) const {
    double acc{0.};
    // Argument of the Chebyshev polynomials
    const double x = getReducedTime(t);
    const double dx_dt = std::pow(dReducedTimeDt(), 2);
    for (unsigned int k = 2; k < nDoF(); ++k) {
        acc += par(k+2) * Acts::detail::chebychevPolyTn(x, k, 2) * dx_dt;
    }
    return acc * t;
}
double RtChebyshev::tLower() const { return par(0); }
double RtChebyshev::tUpper() const { return par(1); }
unsigned RtChebyshev::nDoF() const { return nPar() -2; }

std::vector<double> RtChebyshev::rtParameters() const {
    return std::vector<double>{parameters().begin() +2, parameters().end()};
}
