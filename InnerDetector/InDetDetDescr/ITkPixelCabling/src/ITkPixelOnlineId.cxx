/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "ITkPixelCabling/ITkPixelOnlineId.h"
#include <iostream>

ITkPixelOnlineId::ITkPixelOnlineId(const std::uint32_t onlineId):m_onlineId(onlineId){
  //nop
}


ITkPixelOnlineId::ITkPixelOnlineId(const std::uint32_t rodId, const std::uint32_t fibre){
    m_onlineId = rodId + (fibre<<24);
}

std::uint32_t
ITkPixelOnlineId::rod() const {
  return m_onlineId & 0xFFFFFF;
}

//
std::uint32_t
ITkPixelOnlineId::fibre() const {
  return m_onlineId>>24;
}

bool
ITkPixelOnlineId::isValid() const{
  return m_onlineId != INVALID_ONLINE_ID;
}

std::ostream& operator<<(std::ostream & os, const ITkPixelOnlineId & id){
  os<<std::hex<<std::showbase<<id.m_onlineId<<std::dec<<std::noshowbase;
  return os;
}