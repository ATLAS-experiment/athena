/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// IJetCalibTool.h

//////////////////////////////////////////////////////////
/// class IJetCalibTool
/// 
/// Interface for the tool that applies jet calibrations.
///  
//////////////////////////////////////////////////////////

#ifndef JETCALIBTOOL_IJETCALIBTOOL_H
#define JETCALIBTOOL_IJETCALIBTOOL_H

#include "AsgTools/IAsgTool.h"
#include "JetInterface/IJetModifier.h"

//EDM includes
#include "xAODJet/Jet.h"

//Package includes
#include "JetCalibTools/JetCalibUtils.h"

namespace JetHelper {
  class JetContext;
}

/// This interface is superseding the old version, IJetCalibrationTool 
class IJetCalibTool : virtual public IJetModifier {

  ASG_TOOL_INTERFACE( IJetCalibTool )

public:

  /// Apply calibration to a jet container (for `IJetModifier` interface).
  virtual StatusCode modify(xAOD::JetContainer& jets) const override final {return calibrate(jets);}

  /// Apply calibration to a jet container.
  virtual StatusCode calibrate(xAOD::JetContainer& jets) const = 0;

  // Get the nominal resolution
  virtual StatusCode getNominalResolutionData(const xAOD::Jet&,  const JetHelper::JetContext&, double&) const { return StatusCode::FAILURE; }
  virtual StatusCode getNominalResolutionMC(  const xAOD::Jet&,  const JetHelper::JetContext&, double&) const { return StatusCode::FAILURE; }

};

#endif
