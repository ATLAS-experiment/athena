/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/BTagDecoratorAlg.h"

namespace FlavorTagInference {
  BTagDecoratorAlg::BTagDecoratorAlg(
    const std::string& name, ISvcLocator* svcloc):
    detail::BTag_t(name, svcloc)
  {
  }
  StatusCode BTagDecoratorAlg::initialize() {
    return initializeInternal({"jetLink"});
  }
}
