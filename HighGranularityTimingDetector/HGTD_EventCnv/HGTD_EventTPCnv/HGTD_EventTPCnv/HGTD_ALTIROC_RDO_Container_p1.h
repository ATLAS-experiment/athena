/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_EventTPCnv/HGTD_ALTIROC_RDO_Container_p1.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @brief
 */

#ifndef HGTD_ALTIROC_RDO_CONTAINER_P1_H
#define HGTD_ALTIROC_RDO_CONTAINER_P1_H

#include "HGTD_EventTPCnv/HGTD_ALTIROC_RDO_Collection_p1.h"
#include "HGTD_EventTPCnv/HGTD_ALTIROC_RDO_p1.h"
#include <string>
#include <vector>

class HGTD_ALTIROC_RDO_Container_p1 {
public:
  /// Default constructor
  HGTD_ALTIROC_RDO_Container_p1() = default;

  // holds information to restore the individual collections from the flattened
  // list of persistified clusters
  std::vector<HGTD_ALTIROC_RDO_Collection_p1> m_collection_separator;

  std::vector<HGTD_ALTIROC_RDO_p1> m_rdo_list;
};

#endif
