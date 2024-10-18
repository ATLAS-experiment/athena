/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCNV_T_MUON_IDDETDESCRCNV_H
#define MUONCNV_T_MUON_IDDETDESCRCNV_H

#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/StatusCode.h"

#include "AthenaKernel/ClassID_traits.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "DetDescrCnvSvc/DetDescrConverter.h"


/**
 * Templated base class for Muon detector description converters.
 *
 * @tparam IDHELPER Type of the corresponding IdHelper class.
 */
template <class IDHELPER>
class T_Muon_IDDetDescrCnv: public DetDescrConverter {

public:
   T_Muon_IDDetDescrCnv(ISvcLocator* svcloc, const char* name);

  virtual StatusCode initialize() override;
  virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;
  virtual long int   repSvcType() const override { return storageType(); }

  // Storage type and class ID (used by CnvFactory)
  static long  storageType()   { return DetDescr_StorageType; }
  static const CLID& classID() { return ClassID_traits<IDHELPER>::ID(); }
};

#include "T_Muon_IDDetDescrCnv.icc"

#endif
