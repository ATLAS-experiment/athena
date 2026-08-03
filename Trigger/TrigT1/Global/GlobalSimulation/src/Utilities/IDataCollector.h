/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_IDATACOLLECTOR_H
#define GLOBALSIM_IDATACOLLECTOR_H

#include "GaudiKernel/IAlgTool.h"

#include <string>

namespace GlobalSim {
  class IDataCollector {
  public:
        
    virtual ~IDataCollector() = default;
    virtual void collect(const IAlgTool&, const std::string& msg) = 0;
    virtual std::string to_string() const = 0;
  };
}
#endif
