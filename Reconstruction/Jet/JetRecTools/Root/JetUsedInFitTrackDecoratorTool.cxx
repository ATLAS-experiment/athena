/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#if !defined(XAOD_STANDALONE) && !defined(XAOD_ANALYSIS)
#include "AsgTools/CurrentContext.h"
// Includes from this package:
#include "JetRecTools/JetUsedInFitTrackDecoratorTool.h"

JetUsedInFitTrackDecoratorTool::JetUsedInFitTrackDecoratorTool(const std::string& name):
  asg::AsgTool(name),
  m_decoTool("InDet::InDetUsedInFitTrackDecoratorTool/IDUsedInFitDecoTool_" + name, this) // TODO Why is the suffix required?
{
  declareProperty("Decorator", m_decoTool, "InDet::InDetUsedInFitTrackDecoratorTool instance");
}

StatusCode JetUsedInFitTrackDecoratorTool::initialize() {
  ATH_MSG_INFO("Initializing tool " << name() << "...");
  ATH_CHECK(m_decoTool.retrieve());
  return StatusCode::SUCCESS;
}

int JetUsedInFitTrackDecoratorTool::execute() const {
  ATH_MSG_DEBUG("Executing tool " << name() << "...");
  const EventContext& ctx = Gaudi::Hive::currentContext();
  m_decoTool->decorate(ctx);
  return 0;
}

#endif
