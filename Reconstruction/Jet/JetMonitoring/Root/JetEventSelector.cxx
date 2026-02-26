/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "JetMonitoring/JetEventSelector.h"


JetEventSelector::JetEventSelector(const std::string &t) 
  : asg::AsgTool(t)
  , m_var(this)
{

  declareProperty("CutMin", m_min );
  declareProperty("CutMax", m_max );
  declareProperty("Var", m_var);
}

JetEventSelector:: ~JetEventSelector()= default;

StatusCode JetEventSelector::initialize() {

  for (unsigned int it = 0; it < m_var.size(); it++) {
    ATH_CHECK(m_var[it].retrieve());
    ATH_MSG_DEBUG( "Selecting on var ("<< m_var[it]->varName() << ") in ["<< m_min.at(it) << " , "<< m_max.at(it)<< "]");
  }
  return StatusCode::SUCCESS;
}

// This IJetEventSelector implementation uses EventHistoVarTool to
// retrieve EventInfo attribute.
// Note: xAOD:EventInfo passing not needed as EventHistoVarTool member (m_var)
// configured for and accesses EventInfo attribute directly.
int JetEventSelector::keep(const xAOD::EventInfo&/*not used in this implementation*/ , const xAOD::JetContainer & jets) const {

  for (unsigned int it = 0; it < m_var.size(); it++) {
    float v = m_var[it]->value(jets);
    if ((m_min.at(it) > v) || (m_max.at(it) < v)) return false;
  }
  return true;
}
