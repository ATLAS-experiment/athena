/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODBTaggingEfficiency/BTaggingSmallRSelectionJsonTool.h"
#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "PathResolver/PathResolver.h"

#include <fstream>
#include <cmath>  //std::log
#include <algorithm>  //std::max
#include <limits> //std::numeric_limits
#include <string>

BTaggingSmallRSelectionJsonTool::BTaggingSmallRSelectionJsonTool(const std::string& name)
  : asg::AsgTool(name)  {}

int BTaggingSmallRSelectionJsonTool::accept(const xAOD::Jet& /*jet*/) const {
  throw std::runtime_error("BTaggingSmallRSelectionJsonTool::accept() not yet implemented");
}

int BTaggingSmallRSelectionJsonTool::getQuantile(const xAOD::Jet& /*jet*/) const {
  throw std::runtime_error("BTaggingSmallRSelectionJsonTool::getQuantile() not yet implemented");
}

float BTaggingSmallRSelectionJsonTool::getDiscriminant(const xAOD::Jet& /*jet*/, const std::string& /*discriminantName*/) const {
  throw std::runtime_error("BTaggingSmallRSelectionJsonTool::getDiscriminant() not yet implemented");
}

float BTaggingSmallRSelectionJsonTool::getJetQuantity(const xAOD::Jet& jet, const BTaggingToolUtil::varType vType) const  {
  //Convert MeV to GeV for pt and mass.
  if (vType == BTaggingToolUtil::varType::pT){
    return BTaggingToolUtil::MeVToGeV(jet.pt());
  } else if (vType == BTaggingToolUtil::varType::mass){
    return BTaggingToolUtil::MeVToGeV(jet.m());
  } else if (vType == BTaggingToolUtil::varType::eta){
    return jet.eta();
  } else if (vType == BTaggingToolUtil::varType::abseta){
    return std::abs(jet.eta());
  }
  ATH_MSG_ERROR("Unsupported jet variable quantity requested: '" << BTaggingToolUtil::getVariableName(vType) << "'");
  throw std::runtime_error("BTaggingSmallRSelectionJsonTool::getJetQuantity unsupported jet variable: '" + BTaggingToolUtil::getVariableName(vType) + "'");
}