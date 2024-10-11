/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "GoodRunsLists/TriggerRegistryTool.h"

TriggerRegistryTool::TriggerRegistryTool( const std::string& type, const std::string& name, const IInterface* parent ) 
 : base_class( type, name, parent )
{
}


TriggerRegistryTool::~TriggerRegistryTool()
{
}


bool 
TriggerRegistryTool::registerTriggerSelector(const TString& name, const TString& regexpr, const std::list<TString>& trigpar)
{
  if (m_registry.find(name)!=m_registry.end()) {
    ATH_MSG_WARNING ("registerTriggerSelector() :: trigger selector with name <" << name << "> already registered. Return false.");
    return false;
  }

  ATH_MSG_DEBUG ("registerTriggerSelector() :: trigger selector with name <" << name << "> registered.");
  m_registry[name] = tvtPair(regexpr,trigpar);
  return true;
}

