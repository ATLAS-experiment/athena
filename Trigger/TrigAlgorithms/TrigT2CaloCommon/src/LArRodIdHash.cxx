/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT2CaloCommon/LArRodIdHash.h" 

#include "eformat/SourceIdentifier.h"
#include "LArIdentifier/LArReadoutModuleService.h"

#include <stdexcept>

using eformat::helper::SourceIdentifier;


void LArRodIdHash::initialize( int offset, const std::vector<HWIdentifier>& roms )  {

  m_offset = offset;

  LArReadoutModuleService larROMService;
  std::vector<ID> rmod;
  rmod.reserve(roms.size());

  for(const HWIdentifier& mId : roms) {
    SourceIdentifier sid{static_cast<eformat::SubDetector>(larROMService.subDet(mId)),
                         static_cast<uint8_t>(larROMService.rodFragId(mId))};
    const uint32_t rod_id = sid.code();
    rmod.push_back(rod_id);
  }

  size_t n = 0;
  for (ID id : rmod) {
    m_lookup[id] = n;
    m_int2id.push_back(id);
    ++n;
  }
}


size_t LArRodIdHash::operator() (ID id) const {

  const auto it = m_lookup.find(id);
  if(it!=m_lookup.end()) return it->second;

  throw std::out_of_range("LArRodIdHash: invalid Rod number" + std::to_string(id));

}
