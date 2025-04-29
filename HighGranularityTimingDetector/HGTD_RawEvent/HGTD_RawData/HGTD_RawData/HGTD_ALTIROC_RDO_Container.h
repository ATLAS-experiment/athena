/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RawData/HGTD_ALTIROC_RDO_Container.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @brief
 */

#ifndef HGTD_RAWDATA_HGTD_ALTIROC_RDO_CONTAINER_H
#define HGTD_RAWDATA_HGTD_ALTIROC_RDO_CONTAINER_H

#include "EventContainers/IdentifiableContainer.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Collection.h"
#include "AthenaKernel/CLASS_DEF.h"

class HGTD_ALTIROC_RDO_Container
    : public IdentifiableContainer<HGTD_ALTIROC_RDO_Collection> {
  // friend class HGTD_ALTIROC_RDO_CollectionCnv_p1; //FIXME probably later

public:
  /**
   * @brief Default constructor should NOT be used.
   */
  HGTD_ALTIROC_RDO_Container() = delete;
  ~HGTD_ALTIROC_RDO_Container() = default;

  HGTD_ALTIROC_RDO_Container(unsigned int hashmax);

  static const CLID& classID();

  virtual const CLID& clID() const { return classID(); }
};
// FIXME: What should be the correct ID? (using hgtd id + 1 for now)
CLASS_DEF(HGTD_ALTIROC_RDO_Container, 1218198718, 1)

#endif // HGTD_RAWDATA_HGTD_ALTIROC_RDO_CONTAINER_H