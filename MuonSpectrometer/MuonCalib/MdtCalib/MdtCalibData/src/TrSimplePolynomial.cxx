/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <MdtCalibData/TrSimplePolynomial.h>
#include "GeoModelKernel/throwExcept.h"
#include <cmath>
namespace MuonCalib{
    TrSimplePolynomial::TrSimplePolynomial(const ParVec& vec) : ITrRelation{vec} {
        if (minRadius() >= maxRadius()) {
            THROW_EXCEPTION("Minimum radius greater than maximum radius!");
        }
    }
    std::string TrSimplePolynomial::name() const { return "TrSimplePolynomial"; }

    std::optional<double> TrSimplePolynomial::driftTime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        double time{0.};
        const double x = getReducedR(r);
        for (unsigned int k = 0; k < nDoF(); ++k) {
            time += par(k+2) * std::pow(x,k);
        }
        return std::make_optional(time);


    }
    std::optional<double> TrSimplePolynomial::driftTimePrime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        double dtdr{0.};
        const double dXprime = getReducedRPrime();
        for (unsigned int k = 1; k < nDoF(); ++k) {
            dtdr += par(k+2) * k * std::pow(r,k-1) * dXprime;
        }
        return std::make_optional(dtdr);
        
    }
    std::optional<double> TrSimplePolynomial::driftTime2Prime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        double d2tdr2{0.};
        const double dt_dr = std::pow(getReducedRPrime(), 2);
        for (unsigned int k = 2; k < nDoF(); ++k) {
            d2tdr2 += par(k+2) *k * (k-1)* std::pow(r,k-2) * dt_dr;
        }
        return std::make_optional(d2tdr2);
    }
    double TrSimplePolynomial::minRadius() const { return par(0); }
    double TrSimplePolynomial::maxRadius() const { return par(1); }
    unsigned TrSimplePolynomial::nDoF() const { return nPar() -2; }

}




