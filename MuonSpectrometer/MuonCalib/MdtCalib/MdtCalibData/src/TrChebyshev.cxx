/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <MdtCalibData/TrChebyshev.h>
#include "GeoModelKernel/throwExcept.h"
#include "Acts/Utilities/detail/Polynomials.hpp"
namespace MuonCalib{
    TrChebyshev::TrChebyshev(const ParVec& vec) : ITrRelation{vec} {
        if (minRadius() >= maxRadius()) {
            THROW_EXCEPTION("Minimum radius greater than maximum radius!");
        }
    }
    std::string TrChebyshev::name() const { return "TrChebyshev"; }

    std::optional<double> TrChebyshev::driftTime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        const double reducedR = getReducedR(r);
        double time{0.};
        for (unsigned int k = 0; k < nDoF(); ++k) {
            time += par(k+2) * Acts::detail::chebychevPolyTn(reducedR, k);
        }
        return std::make_optional(time);


    }
    std::optional<double> TrChebyshev::driftTimePrime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        const double reducedR = getReducedR(r);
        const double dt_dr = getReducedRPrime();
        double dtdr{0.};
        for (unsigned int k = 1; k < nDoF(); ++k) {
            dtdr += par(k+2) * Acts::detail::chebychevPolyTn(reducedR, k, 1) * dt_dr;
        }
        return std::make_optional(dtdr);
        
    }
    std::optional<double> TrChebyshev::driftTime2Prime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        const double reducedR = getReducedR(r);
        const double dt_dr = std::pow(getReducedRPrime(), 2);
        double d2tdr2{0.};
        for (unsigned int k = 2; k < nDoF(); ++k) {
            d2tdr2 += par(k+2) * Acts::detail::chebychevPolyTn(reducedR, k, 2)  * dt_dr;
        }
        return std::make_optional(d2tdr2);
    }

    double TrChebyshev::minRadius() const { return par(0); }
    double TrChebyshev::maxRadius() const { return par(1); }
    unsigned TrChebyshev::nDoF() const { return nPar() -2; }
}
