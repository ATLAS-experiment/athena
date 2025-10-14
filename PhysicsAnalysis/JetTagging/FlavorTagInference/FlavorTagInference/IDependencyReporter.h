// for text editors: this file is -*- C++ -*-
/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef I_DEPENDENCY_REPORTER
#define I_DEPENDENCY_REPORTER

#include "FTagDataDependencyNames.h"

#include <set>
#include <string>

class IDependencyReporter {
public:

  using DataDependencyNames = FlavorTagInference::FTagDataDependencyNames;

  /// Destructor.
  virtual ~IDependencyReporter() { };

  // Names of the decorations being added
  virtual DataDependencyNames getDependencies() const = 0;
};

#endif
