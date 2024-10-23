/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// *************************************************************************
//
//  This package is unashameless based on the same
//  a equivalent package for the BCM. 
//  
// *************************************************************************

#ifndef LUCID_DIGITBYTESTREAMCNV_H
#define LUCID_DIGITBYTESTREAMCNV_H

#include "GaudiKernel/Converter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "ByteStreamData/RawEvent.h"  //for RawEventWrite typedef
#include "GaudiKernel/StatusCode.h"
#include "ByteStreamCnvSvcBase/IByteStreamEventAccess.h"
#include "ByteStreamCnvSvcBase/FullEventAssembler.h" 
#include "ByteStreamCnvSvcBase/SrcIdMap.h"

class DataObject;
class StoreGateSvc;
class LUCID_DigitContainer;

class LUCID_DigitByteStreamCnv: public Converter, public AthMessaging {

 public:

  LUCID_DigitByteStreamCnv(ISvcLocator* svcloc);

  virtual StatusCode initialize() override;

  // create the RawEvent fragments for LUCID
  virtual StatusCode createRep(DataObject* pObj, IOpaqueAddress*& pAddr) override;

  virtual long repSvcType() const override { return i_repSvcType(); }
  static  long storageType();
  static const CLID& classID();

  StatusCode fillFEA(LUCID_DigitContainer* RDO_container, RawEventWrite* re);

  unsigned short RodBlockVersion(void) { return m_RodBlockVersion; }
  int            BCs_per_LVL1ID (void) { return m_BCs_per_LVL1ID; }
  
  unsigned int getSourceID() { return 0x00820000; }

private:
  
  ServiceHandle<IByteStreamEventAccess> m_ByteStreamEventAccess;

  FullEventAssembler<SrcIdMap> m_fea;
  unsigned short m_RodBlockVersion;
  int            m_BCs_per_LVL1ID;
};
#endif
