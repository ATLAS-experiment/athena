/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
* @file ITkPixelCablingData/src/ITkPixelCablingData.cxx
* @author Shaun Roe
* @date June 2024
**/

#include "ITkPixelCabling/ITkPixelCablingData.h"
#include <iostream>


bool 
ITkPixelCablingData::empty() const{
  return m_offline2OnlineMap.empty();
}

std::size_t 
ITkPixelCablingData::size() const{
  return m_offline2OnlineMap.size();
}

ITkPixelOnlineId 
ITkPixelCablingData::onlineId(const Identifier & id) const{
  const ITkPixelOnlineId invalidId;
  const auto result = m_offline2OnlineMap.find(id);
  if (result == m_offline2OnlineMap.end()) return invalidId;
  return result->second;
}

//stream extraction to read value from stream into ITkPixelCablingData
std::istream& 
operator>>(std::istream & is, ITkPixelCablingData & cabling){
  unsigned int onlineInt{}, offlineInt{};
  //very primitive, should refine with regex and value range checking
  while(is>>offlineInt>>onlineInt){
    const Identifier offlineId(offlineInt);
    const ITkPixelOnlineId onlineId(onlineInt);
    cabling.m_offline2OnlineMap[offlineId] = onlineId;
  }
  return is;
}

//stream insertion to output cabling map values
std::ostream& 
operator<<(std::ostream & os, const ITkPixelCablingData & cabling){
  for (const auto & [offlineId, onlineId]:cabling.m_offline2OnlineMap){
    os<<offlineId<<", "<<onlineId<<"\n";
  }
  os<<std::endl;
  return os;
}