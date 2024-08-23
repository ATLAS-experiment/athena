// for text editors: this file is -*- C++ -*-
/*
  Copyright (C) 2002-2014 CERN for the benefit of the ATLAS collaboration
*/

#ifndef I_VERTEX_DECORATOR_H
#define I_VERTEX_DECORATOR_H

#include "AsgTools/IAsgTool.h"
#include "xAODTracking/VertexFwd.h"

class IVertexDecorator : virtual public asg::IAsgTool
{
ASG_TOOL_INTERFACE(IVertexDecorator)

public:

  /// Destructor.
  virtual ~IVertexDecorator() { };

  /// Method to decorate a vertex.
  virtual void decorate(const xAOD::Vertex& vertex) const = 0;
};


#endif
