/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
/// class IBTaggingSmallRSelectionJsonTool
///
/// Interface for small R json tool
///
///////////////////////////////////////////////////////////////////

#ifndef CPIBTAGGINGSMALLRSELECTIONJSONTOOL_H
#define CPIBTAGGINGSMALLRSELECTIONJSONTOOL_H

#include "AsgTools/IAsgTool.h"
#include "xAODJet/Jet.h"
#include <string>

class IBTaggingSmallRSelectionJsonTool : virtual public asg::IAsgTool {
  ASG_TOOL_INTERFACE( IBTaggingSmallRSelectionJsonTool )
  public:
  virtual int accept(const xAOD::Jet& jet) const = 0;
  virtual int getQuantile(const xAOD::Jet& jet) const = 0;
  virtual float getDiscriminant( const xAOD::Jet& jet, const std::string& discriminantName) const = 0;
};

#endif // CPIBTAGGINGSMALLRSELECTIONJSONTOOL_H