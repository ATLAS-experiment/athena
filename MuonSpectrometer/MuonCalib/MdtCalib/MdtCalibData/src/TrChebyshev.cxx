/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <MdtCalibData/TrChebyshev.h>
#include "MuonCalibMath/ChebychevPoly.h"
#include "GeoModelKernel/throwExcept.h"

namespace MuonCalib{
    TrChebyshev::TrChebyshev(const IRtRelationPtr& rtRelation, const ParVec& vec) : ITrRelation{rtRelation, vec} {
        if (minRadius() >= maxRadius()) {
            THROW_EXCEPTION("Minimum radius greater than maximum radius!");
        }
    }
    std::string TrChebyshev::name() const { return "TrChebyshev"; }

    std::optional<double> TrChebyshev::driftTime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        const double reducedR = 2 * (r - 0.5 * (maxRadius() + minRadius())) / (maxRadius() - minRadius());
        double time{0.};
        for (unsigned int k = 0; k < nPar() - 2; ++k) {
            time += par(k+2) * chebyshevPoly1st(k, reducedR);
        }
        return std::make_optional(time);


    }
    std::optional<double> TrChebyshev::driftTimePrime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        const double reducedR = 2 * (r - 0.5 * (maxRadius() + minRadius())) / (maxRadius() - minRadius());
        const double dt_dr = 2. / (maxRadius() - minRadius());
        double dtdr{0.};
        for (unsigned int k = 1; k < nPar() - 2; ++k) {
            dtdr += par(k+2) * chebyshevPoly1stPrime(k, reducedR) * dt_dr;
        }
        return std::make_optional(dtdr);
        
    }
    std::optional<double> TrChebyshev::driftTime2Prime(const double r) const {
        if (r < minRadius() || r > maxRadius()) return std::nullopt;
        const double reducedR = 2 * (r - 0.5 * (maxRadius() + minRadius())) / (maxRadius() - minRadius());
        const double dt_dr = std::pow(2. / (maxRadius() - minRadius()), 2);
        double d2tdr2{0.};
        for (unsigned int k = 2; k < nPar() - 2; ++k) {
            d2tdr2 += par(k+2) * chebyshevPoly1st2Prime(k, reducedR) * dt_dr;
        }
        return std::make_optional(d2tdr2);
    }

    double TrChebyshev::minRadius() const { return par(0); }
    double TrChebyshev::maxRadius() const { return par(1); }
}




