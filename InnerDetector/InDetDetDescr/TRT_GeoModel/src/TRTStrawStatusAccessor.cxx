/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TRTStrawStatusAccessor.h"
#include <fstream>

void TRTStrawStatusAccessor::fill(const std::string& path)
{
  std::ifstream file{path};
  if(!file.is_open()) throw std::runtime_error{"Failed to open " + path + " for reading"};

  m_statusMap.clear();
  while(!file.eof()) {
    Key key;
    int status;
    file >> key >> status;
    m_statusMap.emplace(key,status);
  }
  file.close();
}

int TRTStrawStatusAccessor::status(const Identifier& id) const
{
  auto it = m_statusMap.find(id.get_compact());
  if(it!=m_statusMap.end()) return it->second;
  return -1;
}
