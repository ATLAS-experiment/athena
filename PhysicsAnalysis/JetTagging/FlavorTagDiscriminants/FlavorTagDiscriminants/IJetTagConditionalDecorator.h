// for text editors: this file is -*- C++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// This is similar to the IJetTagDecorator, the difference being that
// it provides a decorateWithDefaults method.
//
// The calling tool must decide whether to call decorate or
// decorateWithDefaults on any given jet. The second case is intended
// to be used on jets where the inputs can't be calculated or would be
// nonsensical for some reason.

#ifndef I_JETTAG_CONDITIONAL_DECORATOR_H
#define I_JETTAG_CONDITIONAL_DECORATOR_H

#include "IDependencyReporter.h"
#include "IJetTagDecorator.h"
#include "IDefaultDecorator.h"

#include "AsgTools/IAsgTool.h"

class IJetTagConditionalDecorator : virtual public asg::IAsgTool,
                                    virtual public IDependencyReporter,
                                    virtual public IJetTagDecorator,
                                    virtual public IDefaultDecorator
{
ASG_TOOL_INTERFACE(IJetTagConditionalDecorator)

public:

  /// Destructor.
  virtual ~IJetTagConditionalDecorator() { };

};


#endif
