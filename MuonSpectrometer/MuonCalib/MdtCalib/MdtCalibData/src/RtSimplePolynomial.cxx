/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibData/RtSimplePolynomial.h"
#include "GeoModelKernel/throwExcept.h"
#include <cmath>
using namespace MuonCalib;

RtSimplePolynomial::RtSimplePolynomial(const ParVec& vec) : 
    IRtRelation(vec) { 
    // check for consistency //
    if (nPar() < 3) {
        THROW_EXCEPTION("RtSimplePolynomial::_init() - Not enough parameters!");
    }
    if (tLower() >= tUpper()) {
        THROW_EXCEPTION("Lower time boundary ("<<tLower()<<")>= upper time ("<<tUpper()<<") boundary!");
    }
}  // end RtSimplePolynomial::_init

std::string RtSimplePolynomial::name() const { return "RtSimplePolynomial"; }
double RtSimplePolynomial::tBinWidth() const { return s_tBinWidth; }

double RtSimplePolynomial::radius(double t) const {
    ////////////////////////
    // INITIAL TIME CHECK //
    ////////////////////////
    if (t < tLower()) return 0.0;
    if (t > tUpper()) return 14.6;
 
    double rad{0.0};  // auxiliary radius
    const double x = getReducedTime(t);
    ////////////////////
    // CALCULATE r(t) //
    ////////////////////
    for (unsigned int k = 0; k < nDoF(); k++) { 
        rad += par(k+2) * std::pow(x,k); 
    }
    return std::max(rad, 0.);
}

//*****************************************************************************
double RtSimplePolynomial::driftVelocity(double t) const { 
    // Set derivative to 0 outside of the bounds
    if (t < tLower() || t > tUpper()) return 0.0;

    // Chain rule
    double drdt{0.};
    const double x = getReducedTime(t);
     // Chain rule
    const double dx_dt = dReducedTimeDt();
    for (unsigned int k = 1; k < nDoF(); ++k) {
        drdt += par(k+2) * k * std::pow(x,k-1)*dx_dt;
    }
    return drdt; 
}
double RtSimplePolynomial::driftAcceleration(double t) const {
    double acc{0.};
    const double x = getReducedTime(t);
     // Chain rule
    const double dx_dt = dReducedTimeDt();
    for (unsigned int k = 2; k < nDoF(); ++k) {
        acc += par(k+2) *k * (k-1) * std::pow(x,k-2)*std::pow(dx_dt,2);
    }
    return acc * t;
}
double RtSimplePolynomial::tLower() const { return par(0); }
double RtSimplePolynomial::tUpper() const { return par(1); }
unsigned RtSimplePolynomial::nDoF() const { return nPar() -2; }

std::vector<double> RtSimplePolynomial::rtParameters() const {
    return std::vector<double>{parameters().begin() +2, parameters().end()};
}