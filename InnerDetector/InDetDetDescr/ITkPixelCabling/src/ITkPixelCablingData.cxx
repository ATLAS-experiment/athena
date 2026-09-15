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
  return (m_offlineDetectorResourceID2OnlineIdMap.empty());
}

std::size_t 
ITkPixelCablingData::size() const{
  return m_offline2OnlineMap.size() + m_offlineDetectorResourceID2OnlineIdMap.size();
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

void ITkPixelCablingData::addEntryOffOn(const uint32_t& offlineDetectorResourceID, const ITkPixelOnlineId& onlineId){
    m_offlineDetectorResourceID2OnlineIdMap.insert({offlineDetectorResourceID, onlineId});
}

void ITkPixelCablingData::addSourceID(const uint32_t& sourceID){
    m_sourceIDs.push_back(sourceID);
}

void ITkPixelCablingData::addEntryOnOff(const ITkPixelOnlineId& onlineId, const Identifier& offlineId){
    m_online2OfflineMap.insert({onlineId, offlineId});
}

void ITkPixelCablingData::addEntryOnOff(const ITkPixelOnlineId& onlineId, const ITkPixelCabling::ModuleInfo<Identifier>& moduleInfo){
    m_online2ModuleInfoMap.insert({onlineId, moduleInfo});
}

void ITkPixelCablingData::addTransformType(const uint32_t& moduleID, const ITkPixelCabling::TransformType& transform){
    m_module2TransformTypeMap.insert({moduleID, transform});
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

ITkPixelOnlineId ITkPixelCablingData::onlineId(const uint32_t& offlineDetectorResourceID) const {
    std::unordered_map<uint32_t, ITkPixelOnlineId>::const_iterator it = m_offlineDetectorResourceID2OnlineIdMap.find(offlineDetectorResourceID);
    if (it == m_offlineDetectorResourceID2OnlineIdMap.end()){
        return ITkPixelOnlineId();
    }
    
    return it->second;
}

uint8_t ITkPixelCablingData::chipID(const  ITkPixelCabling::TransformType& t, const uint16_t& col, const uint16_t& row) {

    if(t == ITkPixelCabling::TransformType::NominalIECTriplet || t == ITkPixelCabling::TransformType::NominalIBTriplet){
        // Triplets always have chipID =0
        return 0;
    }
    else if(row <= 383 && col <= 399){
        return 0;
    }
    else if(row <= 383 && col <= 799){
        return 1;
    }
    else if(row <= 767 && col <= 399){
        return 2;
    }
    else if(row <= 767 && col <= 799){
        return 3;
    }
    else{
        //ATH_MSG_WARNING("No chip ID recognized for col = "<< col " ; row = " << row);
        //ATH_MSG_WARNING("Return 0");
        return 0;
    }
}

ITkPixelCabling::TransformType ITkPixelCablingData::transformType(const uint32_t& moduleID) const {
    std::unordered_map<uint32_t, ITkPixelCabling::TransformType>::const_iterator it = m_module2TransformTypeMap.find(moduleID);
    if (it == m_module2TransformTypeMap.end()){
        return ITkPixelCabling::TransformType::UndefinedTransform;
    }
    
    return it->second;
}
