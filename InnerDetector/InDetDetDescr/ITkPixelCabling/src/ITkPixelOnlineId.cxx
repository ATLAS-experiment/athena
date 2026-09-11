/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "ITkPixelCabling/ITkPixelOnlineId.h"
#include <iostream>

ITkPixelOnlineId::ITkPixelOnlineId(const std::uint64_t onlineId):m_onlineId(onlineId){
  //nop
}


ITkPixelOnlineId::ITkPixelOnlineId(const std::uint32_t rodId, const std::uint32_t detectorResourceID){
    m_onlineId = 0;
    m_onlineId = (( m_onlineId | rodId ) << 32) | detectorResourceID;
}

std::uint32_t
ITkPixelOnlineId::sourceID() const {
  return static_cast<uint32_t>((m_onlineId & ITkPixelCabling::SOURCE_ID_MASK) >> 32);
}

//
std::uint32_t
ITkPixelOnlineId::detectorResourceID() const {
  return static_cast<uint32_t>(m_onlineId & ITkPixelCabling::DRID_MASK);
}

std::uint32_t
ITkPixelOnlineId::offlineModuleID() const {
    return (detectorResourceID() & ITkPixelCabling::OFFLINE_DRID_MASK) >> 2;
}

bool
ITkPixelOnlineId::isValid() const{
  return m_onlineId != INVALID_ONLINE_ID;
}

std::ostream& operator<<(std::ostream & os, const ITkPixelOnlineId & id){
  os<<std::hex<<std::showbase<<id.m_onlineId<<std::dec<<std::noshowbase;
  return os;
}