/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_RAWDATABYTESTREAMCNV_TRTRAWCONTRAWEVENTCNV_H
#define TRT_RAWDATABYTESTREAMCNV_TRTRAWCONTRAWEVENTCNV_H

#include "GaudiKernel/ToolHandle.h"  //member
#include "InDetRawData/TRT_RDO_Container.h" //typedef here
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h" //implementation used
#include "AthenaBaseComps/AthConstConverter.h" //inheritance
#include "AthenaKernel/ClassID_traits.h" //implementation used

#include "TRT_RawDataByteStreamCnv/ITRTRawContByteStreamTool.h" //ToolHandle template arg

class DataObject;
class TRTRawContByteStreamTool ; 
class IOpaqueAddress;
class DataObject;

// Abstract factory to create the converter
template <class TYPE> class CnvFactory;

// the converter for writing BS from TRT Raw Data

class TRTRawContByteStreamCnv: public AthConstConverter {
 public:
  TRTRawContByteStreamCnv(ISvcLocator* svcloc);

  typedef TRT_RDO_Container       TRTRawContainer; 

  //! Storage type and class ID
  virtual long repSvcType() const override { return i_repSvcType(); }
  static  long storageType()      { return ByteStreamAddress::storageType(); }
  static CLID classID()    { return ClassID_traits<TRTRawContainer>::ID(); }
  
  //! initialize
  virtual StatusCode initialize() override;
  
  //! create Obj is not used !
  virtual StatusCode createObjConst(IOpaqueAddress* /* pAddr */, DataObject*& /* pObj */) const override
    { return StatusCode::FAILURE;}

  //! this creates the RawEvent fragments for the TRT
  virtual StatusCode createRepConst(DataObject* pObj, IOpaqueAddress*& pAddr) const override;

private: 
  // for BS infrastructure
  ToolHandle<ITRTRawContByteStreamTool>  m_tool;                  // ME: use tool handles
};
#endif

