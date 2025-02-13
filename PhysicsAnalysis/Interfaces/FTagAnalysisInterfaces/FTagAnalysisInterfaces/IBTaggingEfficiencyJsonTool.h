/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
/// class IBTaggingEfficiencyJsonTool
///
/// Interface for the tool that applies the Xbb-tagging efficiency for large-R jets
///
///////////////////////////////////////////////////////////////////

#ifndef IBTAGGINGEFFICIENCYJSONTOOL_H
#define IBTAGGINGEFFICIENCYJSONTOOL_H

#include "PATInterfaces/IReentrantSystematicsTool.h"
#include <PATInterfaces/CorrectionCode.h>
#include "xAODJet/Jet.h"

class IBTaggingEfficiencyJsonTool : virtual public CP::IReentrantSystematicsTool {

  // declare the interface that the class provides
  ASG_TOOL_INTERFACE( IBTaggingEfficiencyJsonTool )

  public:
  virtual ~IBTaggingEfficiencyJsonTool() {};

  virtual CP::CorrectionCode getScaleFactor(const xAOD::Jet& jet, float& sf, const CP::SystematicSet& sys) const = 0;

}; // class IBTaggingEfficiencyJsonTool

#endif
