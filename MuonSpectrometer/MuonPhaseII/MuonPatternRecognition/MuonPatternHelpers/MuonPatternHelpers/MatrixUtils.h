/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPATTERNHELPERS_UTILS_H
#define MUONPATTERNHELPERS_UTILS_H
#include <CxxUtils/ArrayHelper.h>
#include <array>

namespace MuonR4{
    /** @brief Returns the sign of a number */
    constexpr double sign(const double x) {
        return  x > 0. ? 1. : x < 0. ? -1. : 0.;
    }
    /** @brief Calculates the sum of 1 + 2 +3 +4 +... + k */
    constexpr unsigned int sumUp(unsigned k){
        return k*(k+1)/ 2;
    }
    /** @brief If a n-dimensional Matrix is symmetric, it has n* (n+1) /2 parameters
     *         The following function returns the global index when filling the content of the matrix
     *         into a vector via 
     *                 for (i = 0; i < n; ++i) {
     *                     for (k =i; k<n; ++k){
     *                     }
     *                 }
     * @param i: Index of the row
     * @param j : Index of the column */
    template <unsigned int n>
        constexpr unsigned int vecIdxFromSymMat(unsigned int i, unsigned k) {
            assert(i<n);
            assert(k<n);
            if (k < i) {
                return vecIdxFromSymMat<n>(k,i);
            }
            constexpr unsigned int nPars = sumUp(n);
            return  nPars - sumUp(n-i) + (k-i);
        }
    /** @brief Translates back the global index into the two matrix access indices under the assumption that
     *         the matrix is symmetric.
     *  @param k: Global index */
    template <int n>
        constexpr std::array<unsigned, 2> symMatIdxFromVec(unsigned int k) {
            static_assert(n>=0);
            assert (k < sumUp(n));
            if (k <n) {
                return std::array{0u, k};
            }
            if constexpr(n > 0){
                const auto [i, j] = symMatIdxFromVec<n-1>(k-n); 
                return std::array{i+1u, j+1u};
            }
            return make_array<unsigned, 2>(0);
        }
}
#endif