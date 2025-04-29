/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RawData/src/HGTD_ALTIROC_RDO_Container.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>

 *
 * @brief Implementation of HGTD_ALTIROC_RDO_Container.h
 */

#include "HGTD_RawData/HGTD_ALTIROC_RDO_Container.h"

HGTD_ALTIROC_RDO_Container::HGTD_ALTIROC_RDO_Container(unsigned int hashmax)
  : IdentifiableContainer<HGTD_ALTIROC_RDO_Collection>(hashmax) {}

const CLID& HGTD_ALTIROC_RDO_Container::classID() {
  return ClassID_traits<HGTD_ALTIROC_RDO_Container>::ID();
}
