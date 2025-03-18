/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITkStripsRawDataByteStreamCnv_ITkStripsRawContByteStreamCnv_h
#define ITkStripsRawDataByteStreamCnv_ITkStripsRawContByteStreamCnv_h

#include "ByteStreamCnvSvcBase/IByteStreamEventAccess.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "AthenaBaseComps/AthConstConverter.h"

#include "GaudiKernel/ServiceHandle.h"

class DataObject;
class IITkStripsRawContByteStreamTool;

/**
 * @class ITkStripsRawContByteStreamCnv
 *
 * @brief Converter for writing ByteStream from ITkStrips Raw Data
 *
 * This will do the conversion on demand, triggered by the ByteStreamAddressProviderSvc. 
 * Since it is not possible to configure a Converter with Python Configurables, 
 * we use a tool (ITkStripsRawContByteStreamTool) which in turn uses the lightweight 
 * ITkStripsRodEncoder class, to do the actual converting. 
 */
class ITkStripsRawContByteStreamCnv : public AthConstConverter {
 public:

  /** Constructor */
  ITkStripsRawContByteStreamCnv(ISvcLocator* svcLoc);

  /** Destructor */
  virtual ~ITkStripsRawContByteStreamCnv() = default;
  
  /** Initialize */
  virtual StatusCode initialize() override;

  /** Retrieve the class type of the data store the converter uses. */
  virtual long repSvcType() const override { return i_repSvcType(); }
  /** Storage type */
  static long storageType() { return ByteStreamAddress::storageType(); }
  /** Class ID */
  static const CLID& classID() { return ClassID_traits<SCT_RDO_Container>::ID(); }
  
  /** createObj method (not used!) */
  virtual StatusCode createObjConst(IOpaqueAddress*, DataObject*&) const override { return StatusCode::FAILURE; }

  /** 
   * @brief Method to convert ITkStrips Raw Data into ByteStream
   *
   * Gets pointer to RawEvent, get ID container of ITkStrips RDO.
   * Sets up opaque address for Storegate.
   *
   * Uses ITkStrips RawContByteStreamTool to convert Raw Data to ByteStream.
   *
   * @param pDataObject Pointer to data object.
   * @param pOpaqueAddress Opaque address to object.
   */
  virtual StatusCode createRepConst(DataObject* pDataObject, IOpaqueAddress*& pOpaqueAddress) const override;

 private: 

  /** Tool to do coversion from ITkStrips RDO container to ByteStream */
  ToolHandle<IITkStripsRawContByteStreamTool> m_rawContByteStreamTool;

  /** Interface for accessing raw data */
  ServiceHandle<IByteStreamEventAccess> m_byteStreamEventAccess; 
};

#endif 
