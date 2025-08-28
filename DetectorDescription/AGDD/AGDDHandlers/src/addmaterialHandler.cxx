/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AGDDHandlers/addmaterialHandler.h"

addmaterialHandler::addmaterialHandler(const std::string& s,
                                       AGDDController& c)
  : XMLHandler(s, c)
{
}

void addmaterialHandler::ElementHandle(AGDDController& c,
                                       xercesc::DOMNode *t)
{
    m_names.push_back (getAttributeAsString(c, t, "material"));
}


std::vector<std::string> addmaterialHandler::GetNames()
{
  std::vector<std::string> v;
  v.swap (m_names);
  return v;
}
