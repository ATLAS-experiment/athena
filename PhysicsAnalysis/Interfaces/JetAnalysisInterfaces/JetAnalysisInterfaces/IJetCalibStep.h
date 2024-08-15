/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// IJetCalibStep.h

//////////////////////////////////////////////////////////
/// class IJetCalibStep
/// 
/// Interface for the tool that applies a single calibration step to a jet container.
///
//////////////////////////////////////////////////////////

#ifndef JETANALYSISINTERFACES_IJETCALIBSTEP_H
#define JETANALYSISINTERFACES_IJETCALIBSTEP_H

#include "AsgTools/IAsgTool.h"

//EDM includes
#include "xAODJet/JetContainer.h"


namespace JetHelper {
  class JetContext;
}

class IJetCalibStep :  virtual public asg::IAsgTool {

  ASG_TOOL_INTERFACE( IJetCalibStep )

public:


  /// Apply calibration to a jet container.
  virtual StatusCode calibrate(xAOD::JetContainer& jets) const = 0;

  // // Get the nominal resolution
  virtual StatusCode getNominalResolutionData(const xAOD::Jet&, const JetHelper::JetContext&, double&) const { return StatusCode::FAILURE; }
  virtual StatusCode getNominalResolutionMC(  const xAOD::Jet&, const JetHelper::JetContext&, double&) const { return StatusCode::FAILURE; }

};

#endif
