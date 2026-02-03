/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AFP_ByteStream2RawCnv/AFP_WordReadOut.h"

AFP_WordReadOut::AFP_WordReadOut(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type, name, parent)
{
}

AFP_WordReadOut::~AFP_WordReadOut()
{
}

StatusCode AFP_WordReadOut::initialize()
{
  if (m_linkNumTrans.retrieve().isFailure()) {
    ATH_MSG_WARNING("Failed to retrieve service " << m_linkNumTrans );
    return StatusCode::SUCCESS;
  } else {
    ATH_MSG_DEBUG("Retrieved service " << m_linkNumTrans );
  }
  return StatusCode::SUCCESS;
}

StatusCode AFP_WordReadOut::finalize()
{
  return StatusCode::SUCCESS;
}

uint32_t AFP_WordReadOut::getBits(uint32_t the_word, const uint16_t start, const uint16_t stop) const
{
  return (the_word >> stop) & ((1 << (start - stop + 1)) - 1);
}
