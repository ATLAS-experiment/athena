/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <MdtCalibData/TrLegendre.h>
#include "MuonCalibMath/LegendrePoly.h"
#include "GeoModelKernel/throwExcept.h"

namespace MuonCalib{
    TrLegendre::TrLegendre(const ParVec& vec) : ITrRelation{vec} {
        if (minRadius() >= maxRadius()) {
            THROW_EXCEPTION("Minimum radius greater than maximum radius!");
        }
    }
    std::string TrLegendre::name() const { return "TrLegendre"; }

    std::optional<double> TrLegendre::driftTime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        const double reducedR = getReducedR(r);
        double time{0.};
        for (unsigned int k = 0; k < nDoF(); ++k) {
            time += par(k+2) * legendrePoly(k, reducedR);
        }
        return std::make_optional(time);


    }
    std::optional<double> TrLegendre::driftTimePrime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        const double reducedR = getReducedR(r);
        const double dt_dr = getReducedRPrime();
        double dtdr{0.};
        for (unsigned int k = 1; k < nDoF(); ++k) {
            dtdr += par(k+2) * legendreDeriv(k, 1, reducedR) * dt_dr;
        }
        return std::make_optional(dtdr);
        
    }
    std::optional<double> TrLegendre::driftTime2Prime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        const double reducedR = getReducedR(r);
        const double dt_dr = std::pow(getReducedRPrime(), 2);
        double d2tdr2{0.};
        for (unsigned int k = 2; k < nDoF(); ++k) {
            d2tdr2 += par(k+2) * legendreDeriv(k, 2, reducedR) * dt_dr;
        }
        return std::make_optional(d2tdr2);
    }
    double TrLegendre::minRadius() const { return par(0); }
    double TrLegendre::maxRadius() const { return par(1); }
    unsigned TrLegendre::nDoF() const { return nPar() -2; }

}




