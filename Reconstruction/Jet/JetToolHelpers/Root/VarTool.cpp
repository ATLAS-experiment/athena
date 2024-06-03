/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <fstream>
#include <stdio.h>

#include "JetToolHelpers/VarTool.h"

namespace JetHelper {
VarTool::VarTool(const std::string& name)
     : asg::AsgTool(name)
{
}

StatusCode VarTool::initialize()
{

  m_v = InputVariable::createVariable(m_name,m_type,m_isJetVar);
  m_v->setScale(m_scale);

  if (! bool(m_v) ) {
	ATH_MSG_ERROR(" could not create Jet Variable "<< m_name << " type: "<< m_type );
	return StatusCode::FAILURE; 
  }

  return StatusCode::SUCCESS;
}
} //namespace JetHelper

