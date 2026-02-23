/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HGTDMapping/HGTDMappingData.h"
#include <iostream>


bool HGTDMappingData::empty() const{
  return m_offline2OnlineMap.empty();
}

std::size_t HGTDMappingData::size() const{
  return m_offline2OnlineMap.size();
}

HGTDOnlineID HGTDMappingData::onlineId(const Identifier & id) const{
  const HGTDOnlineID invalidId;
  const auto result = m_offline2OnlineMap.find(id);
  if (result == m_offline2OnlineMap.end()) return invalidId;
  return result->second;
}

// stream extraction to read value from a stream 
std::istream& operator>>(std::istream & is, HGTDMappingData & cabling){
  unsigned int onlineInt{}, offlineInt{};
  std::string line{};
  int index = 0;

  while(getline(is,line)){ 
    if (line.empty() || line[0] == '#')
      continue;

    std::stringstream ss(line);
    if (!(ss >> offlineInt >> onlineInt))
      continue;
    
    const Identifier offlineId(offlineInt);
    const HGTDOnlineID onlineId(onlineInt);

    // populate MappingData
    cabling.m_offline2OnlineMap[offlineId] = onlineId;
    cabling.m_hash2OnlineIdArray[index++] = onlineId;
    cabling.m_rodIdSet.insert(onlineId.rod());
  }

  return is;
}

// stream insertion to output cabling map values
std::ostream& operator<<(std::ostream & os, const HGTDMappingData & cabling){
  for (const auto & [offlineId, onlineId]:cabling.m_offline2OnlineMap){
    os<<offlineId<<", "<<onlineId<<"\n";
  }
  os<<std::endl;
  return os;
}




