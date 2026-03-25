/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/FTagDataDependencyNames.h"
#include <utility> //std::move

namespace FlavorTagInference {

  FTagDataDependencyNames 
  FTagDataDependencyNames::operator+(FTagDataDependencyNames d) const {
    FTagDataDependencyNames out = *this;
    out += std::move(d);
    return out;
  }

  FTagDataDependencyNames& 
  FTagDataDependencyNames::operator+=(FTagDataDependencyNames d){
    trackInputs.merge(d.trackInputs);
    electronInputs.merge(d.electronInputs);
    muonInputs.merge(d.muonInputs);
    bTagInputs.merge(d.bTagInputs);
    bTagOutputs.merge(d.bTagOutputs);
    return *this;
  }

}