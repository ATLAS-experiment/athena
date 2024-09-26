/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibData/RtChebyshev.h"
#include "MuonCalibMath/ChebychevPoly.h"
#include "GeoModelHelpers/throwExcept.h"
using namespace MuonCalib;

RtChebyshev::RtChebyshev(const ParVec& vec) : 
    IRtRelation(vec) { 
    // check for consistency //
    if (nPar() < 3) {
        THROW_EXCEPTION("RtChebyshev::_init() - Not enough parameters!");
    }
    if (tLower() >= tUpper()) {
        THROW_EXCEPTION("Lower time boundary >= upper time boundary!");
    }
}  // end RtChebyshev::_init

std::string RtChebyshev::name() const { return std::string("RtChebyshev"); }

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
    double x(2 * (t - 0.5 * (tUpper() + tLower())) / (tUpper() - tLower()));
    double rad(0.0);  // auxiliary radius

    ////////////////////
    // CALCULATE r(t) //
    ////////////////////
    for (unsigned int k = 0; k < nPar() - 2; k++) { 
        rad += parameters()[k + 2] * chebyshevPoly1st(k, x); 
    }
    return std::max(rad, 0.);
}

double RtChebyshev::drdt(double t) const {
    // Set derivative to 0 outside of the bounds
    if (t < tLower()) return 0.0;
    if (t > tUpper()) return 0.0;

    // Argument of the Chebyshev polynomials
    double x = 2 * (t - 0.5 * (tUpper() + tLower())) / (tUpper() - tLower());
    // Chain rule
    double dx_dt = 2 / (tUpper() - tLower());
    double drdt = 0.0;
    for (unsigned int k = 1; k < nPar() - 2; ++k) {
        // Calculate the contribution to dr/dt using k * U_{k-1}(x) * dx/dt
        drdt += parameters()[k + 2] * k * chebyshevPoly2nd(k-1, x) * dx_dt;
    }
    return drdt;
}
//*****************************************************************************

double RtChebyshev::driftvelocity(double t) const { return (radius(t + 1.0) - radius(t)); }

double RtChebyshev::tLower() const { return parameters()[0]; }
double RtChebyshev::tUpper() const { return parameters()[1]; }
unsigned int RtChebyshev::numberOfRtParameters() const { return nPar() - 2; }

std::vector<double> RtChebyshev::rtParameters() const {
    std::vector<double> alpha(nPar() - 2);
    for (unsigned int k = 0; k < alpha.size(); k++) { alpha[k] = parameters()[k + 2]; }

    return alpha;
}
double RtChebyshev::get_reduced_time(const double  t) const {
    return 2. * (t - 0.5 * (tUpper() + tLower())) / (tUpper() - tLower());
}
