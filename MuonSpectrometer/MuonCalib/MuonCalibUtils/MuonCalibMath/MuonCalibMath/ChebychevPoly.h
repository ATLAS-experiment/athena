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
                value = 2.*chebyshevPoly1st(order, x) + chebyshevPoly2nd(order-2, x);
        }
        return value;
    }
    /** @brief Returns the first derivative of the n-th Chebycheb polynomial of the first kind 
      *  @param order of the polynomial to evalue
      *  @param x: Place at which the polynomial is evaluated */
    constexpr double chebyshevPoly1stPrime(const unsigned int order, const double x) {
        double value{0.};
        switch (order) {
            case 0:
                break;
            case 1:
                value = 1.;
                break;
            default:
                value = order * chebyshevPoly2nd(order-1, x);
        }
        return value;
    }
    /** @brief Returns the first derivative of the n-th Chebycheb polynomial of the first second kind 
      *  @param order of the polynomial to evalue
      *  @param x: Place at which the polynomial is evaluated */
    constexpr double chebyshevPoly2ndPrime(const unsigned int order, const double x) {
        double value{0.};
        switch (order) {
            case 0:
                break;
            case 1:
                value = 2.;
                break;
            default:
                value = 2.*chebyshevPoly1stPrime(order, x) + chebyshevPoly2ndPrime(order-2, x);
        }
        return value;
    }
    /** @brief Returns the second derivative of the n-th Chebycheb polynomial of the first kind 
      *  @param order of the polynomial to evalue
      *  @param x: Place at which the polynomial is evaluated */
    constexpr double chebyshevPoly1st2Prime(const unsigned int order, const double x) {
        double value{0.};
        switch (order) {
            case 0:
            case 1:
                break;
            default:
                value = order * chebyshevPoly2ndPrime(order-1, x);
        }
        return value;
    }
    /** @brief Returns the second derivative of the n-th Chebycheb polynomial of the first second kind 
      *  @param order of the polynomial to evalue
      *  @param x: Place at which the polynomial is evaluated */
    constexpr double chebyshevPoly2nd2Prime(const unsigned int order, const double x) {
        double value{0.};
        switch (order) {
            case 0:
            case 1:
                break;
            default:
                value = 2.*chebyshevPoly1st2Prime(order, x) + chebyshevPoly2nd2Prime(order-2, x);
        }
        return value;
    }
}
#endif