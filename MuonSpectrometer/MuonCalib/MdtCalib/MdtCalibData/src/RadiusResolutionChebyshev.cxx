/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibData/RadiusResolutionChebyshev.h"
#include "MuonCalibMath/UtilFunc.h"
#include "Acts/Utilities/detail/Polynomials.hpp"

namespace MuonCalib{
    RadiusResolutionChebyshev::RadiusResolutionChebyshev(const ParVec& vec, const IRtRelationPtr& rtRel):
        IRtResolution{vec}, m_rtRel{rtRel} {}

    std::string RadiusResolutionChebyshev::name() const {
        return "RadiusResolutionChebyshev";        
    }
    double RadiusResolutionChebyshev::resolution(double t, double /*bgRate*/) const {
        double reso{0.};
        const double r = m_rtRel->radius(t);
        const double x = mapToUnitInterval(r, m_r_min, m_r_max);
        for (unsigned int k = 0; k < nDoF(); ++k) {
            reso += par(k) * Acts::detail::chebychevPolyTn(x, k);
        }
        return reso;
    }
    unsigned int RadiusResolutionChebyshev::nDoF() const {
        return parameters().size();
    }
    std::vector<double> RadiusResolutionChebyshev::resParameters() const {
        return parameters();
    }

}