/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
* @file ITkStripCablingData/src/ITkStripCablingData.cxx
* @author Edson Carquin
* @author Daniel Torres
* @brief based on ITkPixelCabling package
**/

#include "ITkStripCabling/ITkStripCablingData.h"
#include <iostream>

const IdentifierHash ITkStripCablingData::s_invalidHash{};
const ITkStripOnlineId ITkStripCablingData::s_invalidId{};

bool 
ITkStripCablingData::empty() const{
  return m_offline2OnlineMap.empty();
}

std::size_t 
ITkStripCablingData::size() const{
  return m_offline2OnlineMap.size();
}

ITkStripOnlineId 
ITkStripCablingData::onlineId(const Identifier & id) const{
  const ITkStripOnlineId invalidId;
  const auto result = m_offline2OnlineMap.find(id);
  if (result == m_offline2OnlineMap.end()) return invalidId;
  return result->second;
}

//stream extraction to read value from stream into ITkStripCablingData
std::istream& 
operator>>(std::istream & is, ITkStripCablingData & cabling){
  unsigned int onlineInt{}, offlineInt{};
  std::string extendedId{};
  //very primitive, should refine with regex and value range checking
  std::string line{};
  int indx=0;
  while(getline(is,line)){
    std::stringstream ss(line);
    std::string t{};
    std::vector<std::string> words;
    std::string offIdShort = "";
    if(line[0] == '#') continue;
    while(getline(ss,t,' ')){
      words.push_back(t);
    }
    if(words[0].size()>10) offIdShort=words[0].erase(10);
    else offIdShort=words[0];
    offlineInt=static_cast<unsigned int>(std::strtoul(offIdShort.c_str(),NULL,0));
    onlineInt=static_cast<unsigned int>(std::strtoul(words[2].c_str(),NULL,0));
    const Identifier offlineId(offlineInt);
    const ITkStripOnlineId onlineId(onlineInt);
    cabling.m_offline2OnlineMap[offlineId] = onlineId;
    cabling.m_hash2OnlineIdArray[indx] = onlineId;
    cabling.m_rodIdSet.insert(onlineId.rod());
    words.clear();
    indx++;
    
  }
  return is;
}

//stream insertion to output cabling map values
std::ostream& 
operator<<(std::ostream & os, const ITkStripCablingData & cabling){
  for (const auto & [offlineId, onlineId]:cabling.m_offline2OnlineMap){
    os<<offlineId<<", "<<onlineId<<"\n";
  }
  os<<std::endl;
  return os;
}

ITkStripOnlineId ITkStripCablingData::getOnlineIdFromHash(const IdentifierHash& hash) const {
  return hash.is_valid()? m_hash2OnlineIdArray[hash] : s_invalidId;
}

void ITkStripCablingData::getRods(std::vector<std::uint32_t>& usersVector) const {
  std::copy(m_rodIdSet.begin(), m_rodIdSet.end(), std::back_inserter(usersVector));
}
