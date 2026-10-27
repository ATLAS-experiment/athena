/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
/// class IBTaggingLargeREfficiencyJsonTool
///
/// Interface for the tool that applies the Xbb-tagging efficiency for large-R jets
///
///////////////////////////////////////////////////////////////////

#ifndef IBTAGGINGLARGEREFFICIENCYJSONTOOL_H
#define IBTAGGINGLARGEREFFICIENCYJSONTOOL_H

#include "PATInterfaces/IReentrantSystematicsTool.h"
#include <PATInterfaces/CorrectionCode.h>
#include "xAODJet/Jet.h"

class IBTaggingLargeREfficiencyJsonTool : virtual public CP::IReentrantSystematicsTool {

  // declare the interface that the class provides
  ASG_TOOL_INTERFACE( IBTagLargeREfficiencyJsonTool )

  public:
  virtual ~IBTaggingLargeREfficiencyJsonTool() {};

  virtual CP::CorrectionCode getScaleFactor(const xAOD::Jet& jet, float& sf, const CP::SystematicSet& sys) const = 0;

}; // class IBTAGGINGLARGEREFFICIENCYJSONTOOL_H

#endif
