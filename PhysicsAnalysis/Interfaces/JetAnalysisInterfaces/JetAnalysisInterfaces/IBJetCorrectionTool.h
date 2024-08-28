/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// IBJetCorrectionTool.h

//////////////////////////////////////////////////////////
/// class IBJetCorrectionTool
/// 
/// Interface for the tool that applies b-jet corrections.
///
//////////////////////////////////////////////////////////

#ifndef JETCALIBTOOL_IBJETCORRECTIONTOOL_H
#define JETCALIBTOOL_IBJETCORRECTIONTOOL_H

#include "AsgTools/IAsgTool.h"

#include "xAODJet/Jet.h"

class IBJetCorrectionTool : public virtual asg::IAsgTool {
  ASG_TOOL_INTERFACE(CP::IBJetCorrectionTool)

public:

  virtual StatusCode applyBJetCorrection(xAOD::Jet& jet, bool isSemiLep) const = 0;

}; 

#endif //> !JETCALIBTOOL_IBJETCORRECTIONTOOL_H
