

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBMATH_LEGENDREPOLYS_H
#define MUONCALIBMATH_LEGENDREPOLYS_H

#include <cmath>


#define POLYSWITCH(order , x)             \
    case order: {                         \
         return Legendre::poly<order>(x); \
         break;                           \
    }
#define DERIVPOLYSWITICH(l, d, x)              \
    case l: {                                  \
        return Legendre::derivative<l,d>(x);   \
        break;                                 \
    }   	                                   \


#define DERIVORDERSWITCH(l, d, x)              \
    case d: {                                  \
        switch (l) {                           \
            DERIVPOLYSWITICH(0, d, x)          \
            DERIVPOLYSWITICH(1, d, x)          \
            DERIVPOLYSWITICH(2, d, x)          \
            DERIVPOLYSWITICH(3, d, x)          \
            DERIVPOLYSWITICH(4, d, x)          \
            DERIVPOLYSWITICH(5, d, x)          \
            DERIVPOLYSWITICH(6, d, x)          \
            DERIVPOLYSWITICH(7, d, x)          \
            DERIVPOLYSWITICH(8, d, x)          \
            DERIVPOLYSWITICH(9, d, x)          \
            DERIVPOLYSWITICH(10, d, x)         \
            DERIVPOLYSWITICH(11, d, x)         \
            DERIVPOLYSWITICH(12, d, x)         \
            DERIVPOLYSWITICH(13, d, x)         \
            DERIVPOLYSWITICH(14, d, x)         \
            DERIVPOLYSWITICH(15, d, x)         \
            DERIVPOLYSWITICH(16, d, x)         \
            default:                           \
                break;                         \
        }                                      \
        break;                                 \
    }



namespace MuonCalib{
    /** @brief Evaluated the n-th factorial at compile time */
        constexpr unsigned long factorial(int n) {
        if (n > 1) {
            unsigned long f = n*factorial(n-1);
            return f;
        } else {
            return 1;
        }
    }
    /** @brief Calculates the binomial coefficient at compile time */
    constexpr unsigned long binomial(unsigned int n, unsigned k) {
        unsigned long b = factorial(n) / (factorial(k) * factorial(n-k));
        return b;
    }


    namespace Legendre{
        /** @brief Calculates the n-th coefficient of the legendre polynomial series
         *  @param l: Order of the legendre polynomial
         *  @param k: Coefficient of the polynomial representation */
        constexpr double coeff(unsigned int l, unsigned int k) {
            if (k %2 != l %2) {
                return 0.;
            } else if (k > 1) {
                const double a_k = -(1.*(l- k +2)*(l+ k-1)) / (1.*(k * (k-1))) * coeff(l, k-2);
                return a_k;
            } else {
                unsigned fl =  (l - l %2) /2;
                unsigned long binom = binomial(l,fl) * binomial(2*l - 2*fl,l);
                return (fl % 2 ? -1. : 1.) * std::pow(0.5, l) * (1.*binom);
            }
        }
        /** @brief Assembles the sum of the legendre monomials
         *  @param l: Order of the legendre polynomial
         *  @param k: Term in the polynomial to add to the sum
         *  @param x: Dependency of the polynomial */
        template <unsigned int l, unsigned int k> 
            constexpr double polySum(const double x) {
                constexpr double a_n = coeff(l,k);
                if constexpr (k > 1) {                    
                    return a_n* std::pow(x,k) + polySum<l, k-2>(x);
                } else{
                    return a_n*std::pow(x,k);
                }
            }
         /** @brief Assembles the n-th derivative of the legendre polynomial 
           * @param l: Order of the legendre polynomial
          *  @param k: Term in the polynomial to add to the sum
          *  @param k: Order of the derivative
          *  @param x: Dependency of the polynomial */
         template <unsigned int l, unsigned int k, unsigned int d> 
            constexpr double derivativeSum(const double x) {
                static_assert(d> 0);
                if constexpr(k <= l && k>=d) {
                    constexpr unsigned long powFac = factorial(k) / factorial(k-d);
                    constexpr double a_n = coeff(l,k) * powFac;
                    return a_n *std::pow(x,k-d) + derivativeSum<l, k-2, d>(x);
                } else {
                     return 0.;
                }
            }        
        
        template <unsigned int l>
            constexpr double poly(const double x) {
                return polySum<l,l>(x);
        }
        template <unsigned int l, unsigned d>
            constexpr double derivative(const double x) {
                return derivativeSum<l,l,d>(x);
            }
    }
    
    constexpr double legendrePoly(const unsigned int l, const double x) {
        double value{0.};
        switch (l) {
            POLYSWITCH( 0, x);
            POLYSWITCH( 1, x);
            POLYSWITCH( 2, x);
            POLYSWITCH( 3, x);
            POLYSWITCH( 4, x);
            POLYSWITCH( 5, x);
            POLYSWITCH( 6, x);
            POLYSWITCH( 7, x);
            POLYSWITCH( 8, x);
            POLYSWITCH( 9, x);
            POLYSWITCH(10, x);
            POLYSWITCH(11, x);
            POLYSWITCH(12, x);
            POLYSWITCH(13, x);
            POLYSWITCH(14, x);
            POLYSWITCH(15, x);
            POLYSWITCH(16, x);
            default:
                value = std::legendre(l, x);
        }
        return value;
    }
    constexpr double legendreDeriv(const unsigned int l, 
                                   const double x, 
                                   const unsigned int derivOrder){
        double value{0.};
        switch (derivOrder) {
            case 0:
                value = legendrePoly(l,x);
                break;
            DERIVORDERSWITCH(l,1,x)
            DERIVORDERSWITCH(l,2,x)
            DERIVORDERSWITCH(l,3,x)
            DERIVORDERSWITCH(l,4,x)
            DERIVORDERSWITCH(l,5,x)
            default:
                break;

        }
        return value;
    
    }
                   

}
#undef POLYSWITCH
#undef DERIVPOLYSWITICH
#endif
