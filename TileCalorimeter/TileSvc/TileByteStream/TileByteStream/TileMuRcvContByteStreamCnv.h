/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEBYTESTREAM_TILEMURCVCONTBYTESTREAMCNV_H
#define TILEBYTESTREAM_TILEMURCVCONTBYTESTREAMCNV_H

#include "AthenaBaseComps/AthConstConverter.h"
#include "TileEvent/TileContainer.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

class DataObject;
class StatusCode;
class IAddressCreator;
class StoreGateSvc;
class IROBDataProviderSvc; 
class TileMuRcvContByteStreamTool ; 
class IByteStreamCnvSvc;
class TileROD_Decoder;

// Abstract factory to create the converter
template <class TYPE> class CnvFactory;


/**
 * @class TileMuRcvContByteStreamCnv
 * @brief This AthConstConverter class provides conversion between ByteStream and TileMuRcvCont
 * @author Joao Gentil Saraiva
 *
 * This class provides methods to convert the bytestream data into TileMuRcv objects and vice versa.
 */

class TileMuRcvContByteStreamCnv
  : public AthConstConverter
{
 public:
  TileMuRcvContByteStreamCnv(ISvcLocator* svcloc);

  typedef TileMuRcvContByteStreamTool  BYTESTREAMTOOL ; 

  virtual StatusCode initialize() override;
  virtual StatusCode createObjConst(IOpaqueAddress* pAddr, DataObject*& pObj) const override;
  virtual StatusCode createRepConst(DataObject* pObj, IOpaqueAddress*& pAddr) const override;
  virtual StatusCode finalize() override;

  /// Storage type and class ID
  virtual long repSvcType() const override { return i_repSvcType(); }
  static long storageType();
  static const CLID& classID();

 private: 
  /** Pointer to TileMuRcvContByteStreamTool */
  ToolHandle<BYTESTREAMTOOL> m_tool;

  ServiceHandle<IByteStreamCnvSvc> m_byteStreamCnvSvc;

  /** Pointer to StoreGateSvc */
  ServiceHandle<StoreGateSvc> m_storeGate;

  /** Pointer to IROBDataProviderSvc */
  ServiceHandle<IROBDataProviderSvc> m_robSvc;

  /** Pointer to TileROD_Decoder */
  ToolHandle<TileROD_Decoder> m_decoder;
};
#endif

