/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RawData/HGTD_ALTIROC_RDO_Collection.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 *
 * @brief
 * FIXME: not sure yet what to do with assignment and copy? make all private
 * as done in InDet? need to know why first...
 */

#ifndef HGTD_RAWDATA_HGTD_ALTIROC_RDO_COLLECTION_H
#define HGTD_RAWDATA_HGTD_ALTIROC_RDO_COLLECTION_H

#include "AthContainers/DataVector.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO.h"
#include "Identifier/IdentifierHash.h"

class HGTD_ALTIROC_RDO_Collection : public DataVector<HGTD_ALTIROC_RDO> {
  // friend class HGTD_ALTIROC_RDO_RawDataCollectionCnv_p1; //FIXME probably later

public:
  /**
   * @brief Default constructor should NOT be used, but is needed for pool I/O.
   */
  HGTD_ALTIROC_RDO_Collection() = default;
  HGTD_ALTIROC_RDO_Collection(IdentifierHash hash) : m_id_hash(hash) {}

  void setIdentifier(Identifier id) { m_id = id; }

  const IdentifierHash& identifierHash() const { return m_id_hash; }
  const Identifier& identify() const { return m_id; }

private:
  IdentifierHash m_id_hash;
  Identifier m_id;
};

#endif // HGTD_RAWDATA_HGTD_ALTIROC_RDO_COLLECTION_H
