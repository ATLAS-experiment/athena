/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBMATH_UTILFUNC_H
#define MUONCALIBMATH_UTILFUNC_H

namespace MuonCalib{
    /** @brief Maps the number x which is in [lowEdge;upperEdge] to the interval [-1;1]. 

     *  @param lowerEdge: Lower edge of the
     */
    inline double mapToUnitInterval(const double x, const double lowerEdge, const double upperEdge) {
        return 2. * (x - 0.5 * (upperEdge + lowerEdge)) / (upperEdge - lowerEdge);
    }
    inline double unitIntervalPrime(const double lowerEdge, const double upperEdge) {
        return 2. / (upperEdge - lowerEdge);
    } 
}


#endif