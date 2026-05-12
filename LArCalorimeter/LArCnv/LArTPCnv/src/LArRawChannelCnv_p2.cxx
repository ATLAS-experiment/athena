/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawEvent/LArRawChannel.h"
#include "CaloIdentifier/CaloGain.h"
#include "LArTPCnv/LArRawChannelCnv_p2.h"
#include <cstdint>
// LArRawChannelCnv_p2, used for T/P separation
// author G.Unal

namespace {

  constexpr uint32_t qualityMask = 0x0000ffffu;
  constexpr uint32_t provenanceMask = 0x3fff0000u;
  constexpr uint32_t gainMask = 0xc0000000u;
  
  uint32_t
  packQualityProvenance(const uint16_t quality, const uint16_t provenance) {
    return static_cast<uint32_t>(quality) |
      ((static_cast<uint32_t>(provenance) << 16) & provenanceMask);
  }
  
  uint16_t
  unpackQuality(const uint32_t packed) {
    return static_cast<uint16_t>(packed & qualityMask);
  }
  
  uint16_t
  unpackProvenance(const uint32_t packed) {
    return static_cast<uint16_t>((packed & provenanceMask) >> 16);
  }

}

void
LArRawChannelCnv_p2::transToPers(const LArRawChannel* trans,
                                 LArRawChannel_p2* pers,
                                 MsgStream& /*log*/) const
{
  pers->m_channelID = trans->identify().get_identifier32().get_compact();
  pers->m_energy = trans->energy();
  pers->m_time = trans->time();

  const uint32_t qualityProvenance =
    packQualityProvenance(trans->quality(), trans->provenance());

  const uint32_t gain =
    static_cast<uint32_t>(trans->gain() & 0x3) << 30;

  pers->m_qualityandgain = static_cast<int>(qualityProvenance | gain);
}

void
LArRawChannelCnv_p2::persToTrans(const LArRawChannel_p2* pers,
                                 LArRawChannel* trans,
                                 MsgStream& /*log*/) const
{
  const uint32_t qualityAndGain =
    static_cast<uint32_t>(pers->m_qualityandgain);

  *trans = LArRawChannel{
    HWIdentifier{Identifier32{pers->m_channelID}},
    pers->m_energy,
    pers->m_time,
    unpackQuality(qualityAndGain),
    unpackProvenance(qualityAndGain),
    static_cast<CaloGain::CaloGain>((qualityAndGain & gainMask) >> 30)
  };
}
