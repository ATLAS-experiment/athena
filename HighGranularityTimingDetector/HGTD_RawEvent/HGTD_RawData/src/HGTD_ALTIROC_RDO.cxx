/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RawData/src/HGTD_ALTIROC_RDO.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 *
 * @brief Implementation of HGTD_ALTIROC_RDO.h
 */

#include "HGTD_RawData/HGTD_ALTIROC_RDO.h"

HGTD_ALTIROC_RDO::HGTD_ALTIROC_RDO(const Identifier rdo_id, const uint64_t word)
    : Identifiable(),
      m_rdo_id(rdo_id),
      m_word(word) {}


HGTD_ALTIROC_RDO::HGTD_ALTIROC_RDO(const Identifier rdo_id, 
                                  const uint8_t crc,
                                  const uint8_t toa,
                                  const uint16_t tot,
                                  const uint8_t l1id,
                                  const uint16_t bcid)
    : Identifiable(),
      m_rdo_id(rdo_id)
    {
      auto u64 = [](auto x){return static_cast<uint64_t>(x);};
      m_word =  ( crc +
                  ((toa & 0x7F) << 8) +
                  ((tot & 0x1FF) << 15) +
                  ((l1id & 0x3F) << 24) +
                  ((u64(bcid) & u64(0x3FF)) << 30) ); 
    }