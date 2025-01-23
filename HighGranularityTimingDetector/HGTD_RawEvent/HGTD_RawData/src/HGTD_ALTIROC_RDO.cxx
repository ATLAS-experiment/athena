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
