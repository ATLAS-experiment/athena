/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IPYTHIA8CUSTOM_H
#define IPYTHIA8CUSTOM_H

#include "GaudiKernel/IAlgTool.h"

namespace Pythia8{
  class Pythia;
}


class IPythia8Custom: virtual public extend_interfaces<IAlgTool> {
  public:
    /** Algtool infrastructure */
    DeclareInterfaceID(IPythia8Custom, 1, 0);

    /** Virtual destructor */
    virtual ~IPythia8Custom(){};
  
    /** Update the pythia event */
    virtual StatusCode ModifyPythiaEvent(Pythia8::Pythia& ) const = 0;
    /** Return how much the cross section is modified.
     *  Should only be called once all events have been processed */
    virtual double CrossSectionScaleFactor() const {return 1.;};

    virtual StatusCode InitializePythiaInfo(Pythia8::Pythia& ) const {return StatusCode::SUCCESS;};
  
};

#endif
