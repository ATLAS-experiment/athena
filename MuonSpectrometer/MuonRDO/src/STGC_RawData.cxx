/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/STGC_RawData.h"
#include "GaudiKernel/MsgStream.h"
#include <iostream>



// P1 ------------------------
// Constructor for Hit
Muon::STGC_RawData::STGC_RawData(const Identifier id)
  :m_id(id)
{
}

Muon::STGC_RawData::STGC_RawData(const Identifier id, const uint16_t bcTag, const float time, 
				 const unsigned int charge, const bool isDead, const bool timeAndChargeInCounts)
  :m_id(id), m_time(time), m_charge(charge), m_bcTag(bcTag), m_isDead(isDead),m_timeAndChargeInCounts(timeAndChargeInCounts)
{
  m_tdo = static_cast<unsigned int>(25.0+time); // place holder for time->tdo from calibration
}

Muon::STGC_RawData::STGC_RawData(const Identifier id, const uint16_t bcTag, const float time, 
				 const unsigned int tdo, const unsigned int charge, const bool isDead, const bool timeAndChargeInCounts)
  :m_id(id), m_time(time), m_tdo(tdo), m_charge(charge), m_bcTag(bcTag), m_isDead(isDead),m_timeAndChargeInCounts(timeAndChargeInCounts)
{
}

MsgStream& operator<<(MsgStream& sl, const Muon::STGC_RawData& data)
{
  sl << "STGC_RawData ("<< &data <<") "
  << ", Strip ID=" << data.identify();
  return sl;
}

std::ostream& operator<<(std::ostream& sl, const Muon::STGC_RawData& data)
{
  sl << "STGC_RawData ("<< &data <<") "
  << ", Strip ID=" << data.identify();
  return sl;
}

