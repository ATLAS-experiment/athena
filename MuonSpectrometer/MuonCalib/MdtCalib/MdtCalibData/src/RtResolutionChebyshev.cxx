/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibData/RtResolutionChebyshev.h"
#include "MuonCalibMath/ChebychevPoly.h"
#include "MuonCalibMath/UtilFunc.h"
#include "GeoModelKernel/throwExcept.h"

#include <algorithm>
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
std::string RtResolutionChebyshev::name() const { return "RtResolutionChebyshev"; }
double RtResolutionChebyshev::resolution(double t, double /*bgRate*/) const {
    ////////////////////////
    // INITIAL TIME CHECK //
    ////////////////////////
    t = std::clamp(t, tLower(), tUpper());

    ///////////////
    // VARIABLES //
    ///////////////
    // argument of the Chebyshev polynomials
    const double x =  mapToUnitInterval(t, tLower(), tUpper());
    double resol{0.0};  // auxiliary resolution

    ////////////////////
    // CALCULATE r(t) //
    ////////////////////
    for (unsigned int k = 0; k < nDoF(); k++) { 
        resol = resol + parameters()[k + 2]  * chebyshevPoly1st(k, x); 
    }

    return resol;
}
double RtResolutionChebyshev::tLower() const { return par(0); }
double RtResolutionChebyshev::tUpper() const { return par(1); }
unsigned int RtResolutionChebyshev::nDoF() const { return nPar() -2; }
std::vector<double> RtResolutionChebyshev::resParameters() const {
    std::vector<double> alpha{};
    alpha.insert(alpha.begin(),  parameters().begin()+2, parameters().end());
    return alpha;
}