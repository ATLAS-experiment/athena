/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerTranslatorSimple.h"
#include <CxxUtils/StringUtils.h>

TriggerTranslatorToolSimple::TriggerTranslatorToolSimple(const std::string& type,
					     const std::string& name,
					     const IInterface* parent)
  : AthAlgTool( type, name, parent )
{
  declareInterface<ITriggerTranslatorTool>(this);
  declareProperty("triggerMapping", m_trigmap_property);
}

TriggerTranslatorToolSimple::~TriggerTranslatorToolSimple() {}

StatusCode TriggerTranslatorToolSimple::initialize() {
  std::vector<std::string> junk;
  //m_trigmap[""] = junk;
  for(const auto& item : m_trigmap_property) {
    ATH_MSG_DEBUG( "Key " << item.first << " Value " << item.second );
    std::vector<std::string> triggers = CxxUtils::tokenize(item.second, ",");
    m_trigmap[item.first] = std::move(triggers);
  }
  return StatusCode::SUCCESS;
}


const std::vector<std::string> TriggerTranslatorToolSimple::translate(const std::string& key) const {
  return m_trigmap.at(key);
}
