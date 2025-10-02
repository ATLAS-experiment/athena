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

void ITkPixelCablingData::print() const {
    std::cout << "Online -> Offline ModuleInfo map " << m_online2ModuleInfoMap.size() << "\n";
    std::cout << "Offline -> Online ModuleInfo map " << m_offline2ModuleInfoMap.size() << "\n";
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

void ITkPixelCablingData::addEntryOffOn(const Identifier& offlineId, const ITkPixelOnlineId& onlineId){
    m_offline2OnlineMap.insert({offlineId, onlineId});
}

void ITkPixelCablingData::addEntryOffOn(const Identifier& offlineId, const ITkPixelCabling::ModuleInfo<ITkPixelOnlineId>& moduleInfo){
    m_offline2ModuleInfoMap.insert({offlineId, moduleInfo});
}

void ITkPixelCablingData::addEntryOnOff(const ITkPixelOnlineId& onlineId, const Identifier& offlineId){
    m_online2OfflineMap.insert({onlineId, offlineId});
}

void ITkPixelCablingData::addEntryOnOff(const ITkPixelOnlineId& onlineId, const ITkPixelCabling::ModuleInfo<Identifier>& moduleInfo){
    m_online2ModuleInfoMap.insert({onlineId, moduleInfo});
}

ITkPixelCabling::ModuleInfo<ITkPixelOnlineId> ITkPixelCablingData::onlineModuleInfo(const Identifier & id) const {
    std::unordered_map<Identifier, ITkPixelCabling::ModuleInfo<ITkPixelOnlineId>>::const_iterator it = m_offline2ModuleInfoMap.find(id);
    if (it == m_offline2ModuleInfoMap.end()){
        return {ITkPixelOnlineId(0), ITkPixelCabling::ModuleType::Undefined, ITkPixelCabling::TransformType::UndefinedTransform};
    }
    
    return it->second;
}

ITkPixelCabling::ModuleInfo<Identifier> ITkPixelCablingData::offlineModuleInfo(const ITkPixelOnlineId & id) const {
    std::unordered_map<ITkPixelOnlineId, ITkPixelCabling::ModuleInfo<Identifier>>::const_iterator it = m_online2ModuleInfoMap.find(id);
    if (it == m_online2ModuleInfoMap.end()){
        return {Identifier(0), ITkPixelCabling::ModuleType::Undefined, ITkPixelCabling::TransformType::UndefinedTransform};
    }
    
    return it->second;
}