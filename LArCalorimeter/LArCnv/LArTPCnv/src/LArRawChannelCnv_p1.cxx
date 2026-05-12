/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawEvent/LArRawChannel.h"
#include "CaloIdentifier/CaloGain.h"
#include "LArTPCnv/LArRawChannelCnv_p1.h"
#include <cstdint>

// LArRawChannelCnv_p1, used for T/P separation
// author R.Seuster <seuster@cern.ch>

void LArRawChannelCnv_p1::transToPers(const LArRawChannel* /*trans*/, LArRawChannel_p1* /*pers*/, MsgStream &/*log*/) const
{
    // No longer used
}

void
LArRawChannelCnv_p1::persToTrans(const LArRawChannel_p1* pers,LArRawChannel* trans,
      MsgStream& /*log*/) const
{
  static constexpr uint32_t invalidQuality = 0xFFFFU;
  // Preserve the previous little-endian union layout explicitly:
  // low 16 bits -> quality, high 16 bits -> provenance.
  const uint32_t q = static_cast<uint32_t>(pers->m_qualityandgain) & 0xFFFFU;

  const uint32_t qualProv = (q == invalidQuality) ? 0x00A50000U : (0x20A50000U | q);
  *trans = LArRawChannel{
    HWIdentifier{Identifier32{pers->m_channelID}},
    pers->m_energy,
    pers->m_time,
    static_cast<uint16_t>(qualProv & 0xFFFFU),         // quality
    static_cast<uint16_t>((qualProv >> 16) & 0xFFFFU), // provenance
    static_cast<CaloGain::CaloGain>((pers->m_qualityandgain >> 16) & 0xF)
  };
}
