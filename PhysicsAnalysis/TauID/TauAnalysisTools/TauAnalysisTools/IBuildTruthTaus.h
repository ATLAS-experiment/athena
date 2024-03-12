// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_IBUILDTRUTHTAUS_H
#define TAUANALYSISTOOLS_IBUILDTRUTHTAUS_H

/*
  author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
  documentation in: ../README.rst
*/

// Framework include(s):
#include "AsgTools/IAsgTool.h"

// EDM include(s):
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

namespace TauAnalysisTools
{

class IBuildTruthTaus :
  public virtual asg::IAsgTool
{

  /// Declare the interface that the class provides
  ASG_TOOL_INTERFACE( TauAnalysisTools::IBuildTruthTaus )

public:
  class ITruthTausEvent
  {
  public:
    virtual ~ITruthTausEvent() = default;
  };
  
  // initialize the tool
  virtual StatusCode initialize() = 0;

  // set truth matching mode, instead of truth building mode
  virtual void setTruthMatchingMode() = 0;

  virtual StatusCode retrieveTruthTaus() = 0;
  virtual StatusCode retrieveTruthTaus(ITruthTausEvent& truthTausEvent) const = 0;

}; // class IBuildTruthTaus

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_IBUILDTRUTHTAUS_H
