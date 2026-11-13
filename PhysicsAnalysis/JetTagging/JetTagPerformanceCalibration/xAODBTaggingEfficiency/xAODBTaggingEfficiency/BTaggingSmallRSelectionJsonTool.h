/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPBTAGGINGSMALLRSELECTIONJSONTOOL_H
#define CPBTAGGINGSMALLRSELECTIONJSONTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingSmallRSelectionJsonTool.h"
#include "BTaggingToolUtil.h"
#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::ordered_json;

class BTaggingSmallRSelectionJsonTool: public asg::AsgTool,  public virtual  IBTaggingSmallRSelectionJsonTool{
  ASG_TOOL_CLASS( BTaggingSmallRSelectionJsonTool, IBTaggingSmallRSelectionJsonTool )

public:
  BTaggingSmallRSelectionJsonTool(const std::string& name);

  virtual int accept(const xAOD::Jet& jet) const override;
  virtual int getQuantile(const xAOD::Jet& jet) const override;
  virtual float getDiscriminant(const xAOD::Jet& jet, const std::string& discriminantName) const override;

  float getJetQuantity(const xAOD::Jet& jet, const BTaggingToolUtil::varType vType)  const;

};

#endif // CPBTAGGINGSMALLRSELECTIONJSONTOOL_H