/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_IJETMAKER_H
#define TRIGGEPPERF_IJETMAKER_H

#include <string>

#include "./Jet.h"
#include "./Cluster.h"

namespace Gep{
  class IJetMaker
  {
  public:
    
    virtual std::vector<Gep::Jet>
    makeJets(const std::vector<Gep::Cluster> &clusters) const  = 0;

    virtual std::string toString() const = 0;
    
    virtual ~IJetMaker() {}
    
  };
}

#endif
