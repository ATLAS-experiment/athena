/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class AbsShapeErrorGetter
   @brief Liquid Argon base class for shape information
*/

#ifndef LArSamples_AbsShapeErrorGetter_H
#define LArSamples_AbsShapeErrorGetter_H

#include "LArSamplesMon/ShapeErrorData.h"
#include "LArCafJobs/CaloId.h"
#include "CaloIdentifier/CaloGain.h"
#include <memory>

class TH1D;

namespace LArSamples {

  class Residual;
  
  class AbsShapeErrorGetter {
  
    public:

      virtual ~AbsShapeErrorGetter() { }
      
      virtual std::unique_ptr<ShapeErrorData> shapeErrorData(unsigned int hash, CaloGain::CaloGain gain, const Residual* toExclude = 0) const = 0;
      virtual std::unique_ptr<ShapeErrorData> phiSymShapeErrorData(short ring, CaloGain::CaloGain gain, const Residual* toExclude = 0) const = 0;
  };
}
#endif
