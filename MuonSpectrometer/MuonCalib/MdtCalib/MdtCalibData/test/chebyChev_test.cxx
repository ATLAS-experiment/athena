/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonCalibMath/ChebychevPoly.h>

#include <iostream>
#include <stdlib.h>

using namespace MuonCalib;

constexpr double cutOff(const double value, const double cut) {
    return std::abs(value)> cut ? value : 1.;
}
int main(){
    constexpr unsigned maxOrder = 16;
    constexpr double h = 5.e-7;
    for (unsigned int o = 0 ; o <= maxOrder ; ++o) {
        for(unsigned step = 0; step <= 200; ++step) {
            const double x = -1. + 1.*step / 100;
            /// Test the first order cheby polynomial
            {
                const double value = chebyshevPoly1st(o, x);
                const double derivative = chebyshevPoly1stPrime(o, x);
                const double numerical = (chebyshevPoly1st(o, x + h) - chebyshevPoly1st(o, x-h))/ (2.*h);

                const double derivative2nd = chebyshevPoly1st2Prime(o, x);
                const double numerical2nd = (chebyshevPoly1stPrime(o, x + h) - chebyshevPoly1stPrime(o, x-h))/ (2.*h);
                if (std::abs(derivative - numerical)/ cutOff(derivative, h) > h*std::max(std::abs(derivative2nd),1.)) {
                    std::cerr<<__FILE__<<":"<<__LINE__<<" Derivative of the "<<o<<"-th polynomial diverges at x="<<x<<", function: "<<value
                            <<", derivative: "<<derivative<<", numerical: "<<numerical<<std::endl;
                    return EXIT_FAILURE;
                }
                if (std::abs(derivative2nd - numerical2nd) / cutOff(derivative2nd, h) > h*std::max(std::abs(derivative2nd),1.)) {
                    std::cerr<<__FILE__<<":"<<__LINE__<<" 2-nd derivative of the "<<o<<"-th polynomial diverges at x="<<x<<", function: "<<value
                            <<", derivative: "<<derivative<<", second derivative: "<<derivative2nd<<"/"<<numerical2nd
                            <<" ---> "<<std::abs(derivative2nd - numerical2nd)<<std::endl;

                    return EXIT_FAILURE;
                }
            }
            {
                const double value = chebyshevPoly2nd(o, x);
                const double derivative = chebyshevPoly2ndPrime(o, x);
                const double numerical = (chebyshevPoly2nd(o, x + h) - chebyshevPoly2nd(o, x-h))/ (2.*h);

                const double derivative2nd = chebyshevPoly2nd2Prime(o, x);
                const double numerical2nd = (chebyshevPoly2ndPrime(o, x + h) - chebyshevPoly2ndPrime(o, x-h))/ (2.*h);
                if (std::abs(derivative - numerical)/ cutOff(derivative, h) > h*std::max(std::abs(derivative),3.)) {
                    std::cerr<<__FILE__<<":"<<__LINE__<<" Derivative of the "<<o<<"-th polynomial diverges at x="<<x<<", function: "<<value
                            <<", derivative: "<<derivative<<", numerical: "<<numerical<<std::endl;
                    return EXIT_FAILURE;
                }
                if (std::abs(derivative2nd - numerical2nd) / cutOff(derivative2nd, h) > h*std::max(std::abs(derivative2nd),1.)) {
                    std::cerr<<__FILE__<<":"<<__LINE__<<" 2-nd derivative of the "<<o<<"-th polynomial diverges at x="<<x<<", function: "<<value
                            <<", derivative: "<<derivative<<", second derivative: "<<derivative2nd<<"/"<<numerical2nd
                            <<" ---> "<<std::abs(derivative2nd - numerical2nd)<<std::endl;

                    return EXIT_FAILURE;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}
