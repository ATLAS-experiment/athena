/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "JetToolHelpers/VarTool.h"

namespace JetHelper {
  VarTool::VarTool(const std::string& name)
       : asg::AsgTool(name)
  {
  }
  
  StatusCode VarTool::initialize()
  {
    m_v = InputVariable::createVariable(m_name,m_type,m_isJetVar, m_scale);
    if (not m_v ) {
      ATH_MSG_ERROR(" could not create Jet Variable "<< m_name << " type: "<< m_type );
      return StatusCode::FAILURE; 
    }
    return StatusCode::SUCCESS;
  }
} //namespace JetHelper

