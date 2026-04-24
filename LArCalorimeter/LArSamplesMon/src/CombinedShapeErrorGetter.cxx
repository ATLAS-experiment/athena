/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArSamplesMon/CombinedShapeErrorGetter.h"

#include "TMath.h"

#include <iostream>
using std::cout;
using std::endl;

using namespace LArSamples;


std::unique_ptr<ShapeErrorData> CombinedShapeErrorGetter::shapeErrorData(unsigned int hash, CaloGain::CaloGain gain, const Residual* /*toExclude*/) const
{
  TVectorD offsets(32);
  CovMatrix errors(32);
  std::unique_ptr<ShapeErrorData> sed;
  for (const AbsShapeErrorGetter* getter : m_getters) {
    std::unique_ptr<const ShapeErrorData> other = getter->shapeErrorData(hash, gain);
    if (!other) continue;
    if (!sed) 
      sed = std::make_unique<ShapeErrorData>(*other);
    else {
      sed = sed->add(*other);
      if (!sed) return nullptr;
    }
  }
  return sed;
}
