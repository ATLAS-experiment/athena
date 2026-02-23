/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class UniformShapeErrorGetter
   @brief Liquid Argon base class for shape information
*/

#ifndef LArSamples_UniformShapeErrorGetter_H
#define LArSamples_UniformShapeErrorGetter_H

#include "LArSamplesMon/AbsShapeErrorGetter.h"

namespace LArSamples {

  class UniformShapeErrorGetter : public AbsShapeErrorGetter {
  
    public:
      
      UniformShapeErrorGetter(double k) : m_k(k) { }
      virtual ~UniformShapeErrorGetter() { }

      virtual std::unique_ptr<ShapeErrorData> shapeErrorData(unsigned int hash, CaloGain::CaloGain gain, const Residual* toExclude = 0) const override;
      virtual std::unique_ptr<ShapeErrorData> phiSymShapeErrorData(short /*ring*/, CaloGain::CaloGain /*gain*/, const Residual* /*toExclude*/) const override { return nullptr; }

    private:
      
      double m_k;
  };
}
#endif
