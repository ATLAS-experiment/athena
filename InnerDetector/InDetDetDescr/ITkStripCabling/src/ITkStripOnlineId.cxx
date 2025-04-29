/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "ITkStripCabling/ITkStripOnlineId.h"
#include <iostream>

ITkStripOnlineId::ITkStripOnlineId(const std::uint32_t onlineId):m_onlineId(onlineId){
  //nop
}


ITkStripOnlineId::ITkStripOnlineId(const std::uint32_t rodId, const std::uint32_t fibre){
    m_onlineId = rodId + (fibre<<24);
}

std::uint32_t
ITkStripOnlineId::rod() const {
  return m_onlineId & 0xFFFFFF;
}

//
std::uint32_t
ITkStripOnlineId::fibre() const {
  return m_onlineId>>24;
}

bool
ITkStripOnlineId::isValid() const{
  return m_onlineId != INVALID_ONLINE_ID;
}

std::ostream& operator<<(std::ostream & os, const ITkStripOnlineId & id){
  os<<std::hex<<std::showbase<<id.m_onlineId<<std::dec<<std::noshowbase;
  return os;
}
