//  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef GLOBALSIM_GSALG_H
#define GLOBALSIM_GSALG_H

namespace GlobalSim {
  // provide a run() method for all GlobalSim algorihms
  class IGSAlg {
  public:
    virtual ~IGSAlg() = default;
    virtual void run() const = 0;
  };
}

#endif
