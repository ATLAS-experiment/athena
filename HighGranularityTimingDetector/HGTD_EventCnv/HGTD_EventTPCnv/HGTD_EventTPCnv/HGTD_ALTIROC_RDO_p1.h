/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_EventTPCnv/HGTD_ALTIROC_RDO_p1.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @brief Represents the persitified version of HGTD_ALTIROC_RDO.
 */

#ifndef HGTD_EVENTTPCNV_HGTD_ALTIROC_RDO_P1_H
#define HGTD_EVENTTPCNV_HGTD_ALTIROC_RDO_P1_H

#include "HGTD_RawData/HGTD_ALTIROC_RDO.h"
#include "Identifier/Identifier.h"

class HGTD_ALTIROC_RDO_p1 {
public:
  typedef Identifier::value_type IdType_t;

  HGTD_ALTIROC_RDO_p1() = default;

  friend class HGTD_ALTIROC_RDO_Cnv_p1;

private:
  IdType_t m_rdo_id{};
  uint64_t m_word{};
};

#endif // HGTD_ALTIROC_RDO_P1_H
