// for text editors: this file is -*- C++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Base class to provide a decorateWithDefaults method.
//

#ifndef I_DEFAULT_DECORATOR_H
#define I_DEFAULT_DECORATOR_H

#include "xAODBase/IParticle.h"

class IDefaultDecorator
{
public:

  /// Method to decorate a jet with defaults.
  virtual void decorateWithDefaults(const xAOD::IParticle& i_jet) const = 0;

};


#endif
