/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////
/// class IScaleFactorTool
///
/// Interface for the generic SF tool that takes in
/// an object and returns its SF
//////////////////////////////////////////////////////

#ifndef CPISCALEFACTORTOOL_H
#define CPISCALEFACTORTOOL_H

#include "AsgTools/IAsgTool.h"

using xAOD::IParticle;

class IScaleFactorTool : virtual public asg::IAsgTool {
  ASG_TOOL_INTERFACE ( IScaleFactorTool )

  public:
  virtual float getSF( const xAOD::IParticle* p) const = 0;
};
#endif // CPISCALEFACTORTOOL_H
