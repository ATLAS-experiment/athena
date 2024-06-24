/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_IL1TOPOALGTOOL_H
#define GLOBALSIM_IL1TOPOALGTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include <string>

// provide an pure abstract interface to AlgTools implementing
// GlobalSim Algs.

namespace GlobalSim {
  class IL1TopoAlgTool : virtual public ::IAlgTool {

  public:
    DeclareInterfaceID(IL1TopoAlgTool, 1, 0);
    virtual ~IL1TopoAlgTool() = default;

    virtual StatusCode run(const EventContext& ctx) const = 0;

    virtual std::string toString() const = 0;
  };
  
}
#endif
