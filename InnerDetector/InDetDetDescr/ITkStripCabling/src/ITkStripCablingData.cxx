/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
* @file ITkStripCablingData/src/ITkStripCablingData.cxx
* @author Edson Carquin
* @date September 2024
* @brief based on ITkPixelCabling package
**/

#include "ITkStripCabling/ITkStripCablingData.h"
#include <iostream>


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
  //very primitive, should refine with regex and value range checking
  while(is>>offlineInt>>onlineInt){
    const Identifier offlineId(offlineInt);
    const ITkStripOnlineId onlineId(onlineInt);
    cabling.m_offline2OnlineMap[offlineId] = onlineId;
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
