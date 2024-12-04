/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibData/RtResolutionChebyshev.h"
#include "MuonCalibMath/ChebychevPoly.h"
#include "GeoModelKernel/throwExcept.h"

using namespace MuonCalib;

 RtResolutionChebyshev::RtResolutionChebyshev(const ParVec& vec) : 
    IRtResolution(vec) { 
    // check for consistency //
    if (nPar() < 3) {
        THROW_EXCEPTION("Not enough parameters!");
    }
    if (tLower() >= tUpper()) {
        THROW_EXCEPTION("Lower time boundary >= upper time boundary!");
    }
}
std::string RtResolutionChebyshev::name() const { return std::string("RtResolutionChebyshev"); }
double RtResolutionChebyshev::resolution(double t, double /*bgRate*/) const {
    ////////////////////////
    // INITIAL TIME CHECK //
    ////////////////////////

    if (t != tLower() && t != tUpper()) {
        // get resolution for tmin and tmax to get reasonable boundary conditrions
        double res_min(resolution(tLower())), res_max(resolution(tUpper()));

        // if x is out of bounds, return 99999 //
        if (t < tLower()) return res_min;

        if (t > tUpper()) return res_max;
    }
    ///////////////
    // VARIABLES //
    ///////////////
    // argument of the Chebyshev polynomials
    double x(2 * (t - 0.5 * (tUpper() + tLower())) / (tUpper() - tLower()));
    double resol(0.0);  // auxiliary resolution

    ////////////////////
    // CALCULATE r(t) //
    ////////////////////
    for (unsigned int k = 0; k < nPar() - 2; k++) { resol = resol + parameters()[k + 2]  * chebyshevPoly1st(k, x); }

    return resol;
}
double RtResolutionChebyshev::tLower() const { return parameters()[0]; }
double RtResolutionChebyshev::tUpper() const { return parameters()[1]; }
unsigned int RtResolutionChebyshev::numberOfResParameters() const { return nPar() - 2; }
std::vector<double> RtResolutionChebyshev::resParameters() const {
    std::vector<double> alpha(nPar() - 2);
    for (unsigned int k = 0; k < alpha.size(); k++) { alpha[k] = parameters()[k + 2]; }

    return alpha;
}
inline double RtResolutionChebyshev::get_reduced_time(const double  t) const {
    return 2 * (t - 0.5 * (tUpper() + tLower())) / (tUpper() - tLower());
}
