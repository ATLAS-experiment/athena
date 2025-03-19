/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArSamples_Definitions_H
#define LArSamples_Definitions_H

#include "TMatrixTSym.h"

namespace LArSamples {
  typedef TMatrixTSym<double> CovMatrix;

  struct Definitions {
    static const unsigned int nChannels;
    static const unsigned int nChannelsSC;
    static const unsigned int samplingInterval;
    static double samplingTime(unsigned int i) { return samplingInterval*i; }
    static const double none;
    static bool isNone(double x) { return (x < 0.999*none); }
  };
  
  enum ShapeErrorType { CellShapeError, 
                        LowGainCellShapeError, MedGainCellShapeError, HighGainCellShapeError, 
                        RingShapeError,
                        LowGainRingShapeError, MedGainRingShapeError, HighGainRingShapeError,
                        NoShapeError, NShapeErrorTypes, BestShapeError };
}

#endif
