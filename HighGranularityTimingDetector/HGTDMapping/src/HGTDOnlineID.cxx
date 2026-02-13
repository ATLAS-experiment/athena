/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "HGTDMapping/HGTDOnlineID.h"
#include <iostream>

// contructors 
HGTDOnlineID::HGTDOnlineID(const std::uint32_t onlineId): m_onlineId(onlineId) {}


HGTDOnlineID::HGTDOnlineID(const std::uint32_t rodId, const std::uint32_t elink){
    m_onlineId = rodId + (elink<<24);
}

std::uint32_t HGTDOnlineID::rod() const {
  return m_onlineId & 0xFFFFFF; // need to check where ROD ID is stored
}

std::uint32_t HGTDOnlineID::elink() const {
  return m_onlineId>>24;
}

bool HGTDOnlineID::isValid() const{
  return m_onlineId != INVALID_ONLINE_ID;
}

std::ostream& operator<<(std::ostream & os, const HGTDOnlineID & id){
  os<<std::hex<<std::showbase<<id.m_onlineId<<std::dec<<std::noshowbase;
  return os;
}
