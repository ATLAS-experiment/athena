/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TriggerDecisionProxyLoader.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// James Catmore (james.catmore@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////

#include "ExpressionEvaluation/TriggerDecisionProxyLoader.h"
#include <set>
#include <string>

namespace ExpressionParsing {
  TriggerDecisionProxyLoader::~TriggerDecisionProxyLoader()
  {
  }

  void TriggerDecisionProxyLoader::reset()
  {
  }

  IAccessor::VariableType TriggerDecisionProxyLoader::variableTypeFromString(const std::string &varname) const
  {
    static const std::set<std::string> knownPrefixes{"L1", "L2", "EF", "HLT"};

    std::size_t location = varname.find('_');
    if (location == std::string::npos) return VT_UNK;
    std::string prefix(varname.begin(), varname.begin()+location);

    if (std::find(knownPrefixes.begin(), knownPrefixes.end(), prefix) != knownPrefixes.end()) return VT_INT;
    else return VT_UNK;    
  }

  int TriggerDecisionProxyLoader::loadInt([[maybe_unused]] const EventContext& ctx,const std::string &varname) const
  {
    bool passed(false);
    if ( varname == "" ) return(0); // no name => ?
    // trigger decision
    passed = m_trigDec->isPassed(varname);
    if (passed) return(1);
    else return(0); 
  }

  double TriggerDecisionProxyLoader::loadDouble([[maybe_unused]] const EventContext& ctx,const std::string &varname) const
  {
    throw std::runtime_error("Trigger decision can't be a double: check " + varname);
  }

  std::vector<int> TriggerDecisionProxyLoader::loadVecInt([[maybe_unused]] const EventContext& ctx,const std::string &varname) const
  {
    throw std::runtime_error("Trigger decision can't be vector<int>: check " + varname);
  }

  std::vector<double> TriggerDecisionProxyLoader::loadVec([[maybe_unused]] const EventContext& ctx,const std::string &varname) const
  {
    throw std::runtime_error("Trigger decision can't be vector<double>: check " + varname);
  }
}
