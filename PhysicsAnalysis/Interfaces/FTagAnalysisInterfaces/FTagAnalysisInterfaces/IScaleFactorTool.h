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

#include "PATInterfaces/IReentrantSystematicsTool.h"
#include "PATInterfaces/SystematicSet.h"
#include "xAODBase/IParticle.h"
#include <map>

class IScaleFactorTool : virtual public CP::IReentrantSystematicsTool {
  ASG_TOOL_INTERFACE ( IScaleFactorTool )

  public:
  virtual std::map<CP::SystematicSet, float> getSF( const xAOD::IParticle* p ) const = 0;
  virtual std::unordered_map<std::string, int> inferWPs( const xAOD::IParticle* p ) const = 0; 
};
#endif // CPISCALEFACTORTOOL_H
