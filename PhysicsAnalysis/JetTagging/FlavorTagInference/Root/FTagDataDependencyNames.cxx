/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>

#include "FlavorTagInference/FTagDataDependencyNames.h"

namespace FlavorTagInference {
  FTagDataDependencyNames FTagDataDependencyNames::operator+(
    FTagDataDependencyNames d2) const {
    FTagDataDependencyNames d1 = *this;
    d1.trackInputs.merge(d2.trackInputs);
    d1.bTagInputs.merge(d2.bTagInputs);
    d1.bTagOutputs.merge(d2.bTagOutputs);
    return d1;
  }
  void FTagDataDependencyNames::operator+=(FTagDataDependencyNames d) {
    FTagDataDependencyNames tmp = *this + std::move(d);
    *this = tmp;
  }
}
