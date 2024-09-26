/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBMATH_POLYNOMIALS_H
#define MUONCALIBMATH_POLYNOMIALS_H

#include <cmath>
namespace MuonCalib{
    /** @brief Returns the n-th Chebyshev polynomial of first kind evaluated at x
     *         (c.f. https://en.wikipedia.org/wiki/Chebyshev_polynomials)
     *  @param order of the polynomial to evalue
     *  @param x: Place at which the polynomial is evaluated */
    constexpr double chebyshevPoly1st(const unsigned int order, const double x) {        
        double value{0.};
        switch(order) {
            case 0:
                value=1.;
                break;
            case 1:
                value = x;
                break;
            default:
                value = 2.*x*chebyshevPoly1st(order-1, x) - chebyshevPoly1st(order-2, x);
            break;
        }
        return value;
    }
     /** @brief Returns the n-th Chebyshev polynomial of second kind evaluated at x
     *         (c.f. https://en.wikipedia.org/wiki/Chebyshev_polynomials)
     *  @param order of the polynomial to evalue
     *  @param x: Place at which the polynomial is evaluated */
    constexpr double chebyshevPoly2nd(const unsigned int order, const double x)  {
        double value{0.};
        switch (order) {
            case 0:
                value = 1.;
                break;
            case 1:
                value = 2.*x;
                break;
            default:
                value = 2.*x*chebyshevPoly2nd(order-1, x) - chebyshevPoly2nd(order-2, x);
        }
        return value;
    }
}
#endif