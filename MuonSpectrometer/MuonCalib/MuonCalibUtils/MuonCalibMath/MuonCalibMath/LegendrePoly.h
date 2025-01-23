/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    }                                          \


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
        constexpr unsigned long factorial(const int n) {
        if (n > 1) {
            unsigned long f = n*factorial(n-1);
            return f;
        } else {
            return 1;
        }
    }
    /** @brief Calculates the binomial coefficient at compile time */
    constexpr unsigned long binomial(const unsigned n, const unsigned k) {
        unsigned long b = factorial(n) / (factorial(k) * factorial(n-k));
        return b;
    }
    /** @brief Calculate the power of a variable x at compile time */
    template <int k> constexpr double pow(const double x) {
        if constexpr (k < 0) {
            return pow<-k>(1./x);
        } else if constexpr(k > 0) {
            return x*pow<k-1>(x);
        } else {
            return 1.;
        }
    }
    /** @brief Calculate the power of a variable at run time */
    constexpr double pow(double x, int power) {
        double res{1.};
        if (power < 0) {
            x = 1./x;
            power = - power;
        }
        for (int iter = 1; iter<=power; ++iter) {
            res*=x;
        }
        return res;
    }


    namespace Legendre{
        /** @brief Calculates the n-th coefficient of the legendre polynomial series
         *  @param l: Order of the legendre polynomial
         *  @param k: Coefficient insides the polynomial representation */
        constexpr double coeff(const unsigned l, const unsigned k) {
            if (k %2 != l %2) {
                return 0.;
            } else if (k > 1) {
                const double a_k = -(1.*(l- k +2)*(l+ k-1)) / (1.*(k * (k-1))) * coeff(l, k-2);
                return a_k;
            } else {
                unsigned fl =  (l - l %2) /2;
                unsigned long binom = binomial(l,fl) * binomial(2*l - 2*fl,l);
                return (fl % 2 ? -1. : 1.) * pow(0.5, l) * (1.*binom);
            }
        }
        /** @brief Assembles the sum of the legendre monomials
         *  @param l: Order of the legendre polynomial
         *  @param k: Term in the polynomial to add to the sum
         *  @param x: Point of evaluation [-1,1] */
        template <unsigned l, unsigned k> 
            constexpr double polySum(const double x) {
                const double a_n = coeff(l,k);
                if constexpr (k > 1) {                    
                    return a_n* pow<k>(x) + polySum<l, k-2>(x);
                } else{
                    return a_n*pow<k>(x);
                }
            }
        /** @brief Assembles the n-th derivative of the legendre polynomial 
          * @param l: Order of the legendre polynomial
          * @param k: Term in the polynomial to add
          * @param d: Order of the derivative
          * @param x: Point of evaluation [-1,1] */
        template <unsigned l, unsigned k, unsigned d> 
            constexpr double derivativeSum(const double x) {
                if constexpr(k <= l && k>=d) {
                    constexpr unsigned long powFac = factorial(k) / factorial(k-d);
                    const double a_n = coeff(l,k) * powFac;
                    return a_n *pow<k-d>(x) + derivativeSum<l, k-2, d>(x);
                } else {
                     return 0.;
                }
            }
        /** @brief Assembles the n-th derivative of a legendre polynomial at run time
          * @param l: Order of the legendre polynomial
          * @param d: Order of the derivative
          * @param x: Point of evaluation [-1,1] */
        constexpr double derivativeSum(const unsigned l, const unsigned d, const double x) {
            double sum{0.};
            for (int k=l; k>= static_cast<int>(d) && k >=0; k-=2) {
                const double a_n = coeff(l,k) * factorial(k) / factorial(k-d);
                sum += a_n * pow(x,k-d);
            }
            return sum;
        }
        /** @brief Assembles the legendre polynomial at run-time */
        constexpr double polySum(const unsigned l, const double x) {
            double sum{0.};
            for (unsigned k = l%2; k<=l; k+=2) {
                sum += pow(x,k) * coeff(l,k);
            }
            return sum;
        }
        /** @brief Evaluates the n-th Legendre polynomial at x
         *  @param l: Order of the Legendre polynoimal 
         *  @param x: Point of evaluation [-1;1]  */ 
        template <unsigned l>
            constexpr double poly(const double x) {
                return polySum<l,l>(x);
        }
        /** @brief Evaluates the d-th derivative of the n-th Legendre polynomial at x 
         *  @param l: Order of the Legendre polynomial
         *  @param d: Order of the respective derivative
         *  @param x: Point of evaluation [-1;1] */
        template <unsigned l, unsigned d>
            constexpr double derivative(const double x) {
                return derivativeSum<l,l,d>(x);
            }
    }
    /** @brief Calculates the legendre polynomial of rank l at x
     *  @param l: Order of the legendre polynomial
     *  @param x: Point of evaluation [-1;1]  */
    constexpr double legendrePoly(const unsigned l, const double x) {
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
                return Legendre::polySum(l, x);
        }
    }
    /** @brief Evaluates the n-th derivative of the l-th Legendre polynomial
     *  @param x: Point of evaluation [-1;1]
     *  @param l: Order of the Legendre polynomial
     *  @param d: Order of the derivative */
    constexpr double legendreDeriv(const unsigned l, const unsigned d, const double x){
        switch (d) {
            case 0:
                return legendrePoly(l,x);
                break;
            DERIVORDERSWITCH(l,1,x)
            DERIVORDERSWITCH(l,2,x)
            DERIVORDERSWITCH(l,3,x)
            DERIVORDERSWITCH(l,4,x)
            DERIVORDERSWITCH(l,5,x)
            default:
                return Legendre::derivativeSum(l, d, x);
        }
        return 0.;
    }
}
#undef POLYSWITCH
#undef DERIVORDERSWITCH
#undef DERIVPOLYSWITICH
#endif