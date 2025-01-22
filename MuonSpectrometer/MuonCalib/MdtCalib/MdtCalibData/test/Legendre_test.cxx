/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <format>
#include <MuonCalibMath/LegendrePoly.h>

constexpr double cutOff(const double value, const double cut) {
    return std::abs(value)> cut ? value : 1.;
}
std::string printCoeff1st(unsigned int l, unsigned int k) {
    if ( k<= l) {
        return std::format("{:2d}-th: {:.5f}, {:}", k, MuonCalib::Legendre::coeff(l,k), printCoeff1st(l, k+2));
    } else {
        return std::format("{:2d}-th: {:.5f}",k, MuonCalib::Legendre::coeff(l,k));
    }
}
std::string printCoeffs1st(unsigned int l) {
    return std::format("coeffs of P_{:d}: {:}", l, printCoeff1st(l, l%2));
}

int main() {
    constexpr unsigned maxOrder = 16;
    constexpr unsigned nSteps = 200;
    constexpr double h = 1.e-7;

    for (unsigned int o = 0 ; o <= maxOrder ; ++o) {
        for(unsigned step = 0; step <= nSteps; ++step) {
            const double x = -1. + 1.*step / nSteps;        
            const double stdImpl = 1.*std::legendre(o, x);
            const double calibLegndre = MuonCalib::legendrePoly(o, x);
            if (std::abs(stdImpl - calibLegndre) / cutOff(stdImpl, h) > h) {
                std::cerr<<"Legendre polynomial "<<o<<" at "<<x<<" deviates from std implementation std: "<<stdImpl<<", calib: "<<calibLegndre<<std::endl;
                std::cout<<printCoeffs1st(o)<<std::endl;             
                return EXIT_FAILURE;
            }
            for (unsigned d = 1; d <= o + 1; ++d) {
                const double deriv = MuonCalib::legendreDeriv(o,d, x);
                const double numDeriv = (MuonCalib::legendreDeriv(o, d-1, x+h) - 
                                         MuonCalib::legendreDeriv(o, d-1, x-h))/ (2.*h);
                if (std::abs(deriv - numDeriv) / cutOff(numDeriv, 10.*h) > 1.e-4) {
                    std::cerr<<"Legendre polynomial "<<o<<" at "<<x<<" "<<d<< "-th derivative deviates from numerical approach "
                             <<deriv<<", calib: "<<numDeriv<<std::endl;
                    std::cout<<printCoeffs1st(o)<<std::endl;
                    return EXIT_FAILURE;
                }
            }
            /** Check whether the polynomials solve the legendre differential equation */
            const double legendreEq = (1- x*x) * MuonCalib::legendreDeriv(o, 2, x) - 2.*x*MuonCalib::legendreDeriv(o,1, x) + o*(o+1) * MuonCalib::legendrePoly(o,x);
            if (std::abs(legendreEq) > h) {
                std::cerr<<" The legendre polynomial does not solve the differential equation "
                        <<"x="<<x<<" --> "<<legendreEq<<std::endl;
                std::cout<<printCoeffs1st(o)<<std::endl;
                return EXIT_FAILURE;
            }

        }
    }

    return EXIT_SUCCESS;
}