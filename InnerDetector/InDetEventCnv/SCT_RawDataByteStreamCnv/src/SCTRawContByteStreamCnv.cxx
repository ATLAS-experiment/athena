/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SCTRawContByteStreamCnv.h"
#include "SCTRawContByteStreamToolProviderTool.h"

#include "SCT_RawDataByteStreamCnv/ISCTRawContByteStreamTool.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h" 
#include "ByteStreamData/RawEvent.h" 

#include "AthenaBaseComps/AthCheckMacros.h"
#include "StoreGate/StoreGateSvc.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"

// Constructor

SCTRawContByteStreamCnv::SCTRawContByteStreamCnv(ISvcLocator* svcLoc) :
  AthConstConverter(storageType(), classID(), svcLoc, "SCTRawContByteStreamCnv"),
  m_rawContByteStreamToolProvider{"SCTRawContByteStreamToolProviderTool"},
  m_rawContByteStreamTool(nullptr), // {"SCTRawContByteStreamTool"},  
  m_byteStreamEventAccess{"ByteStreamCnvSvc", "SCTRawContByteStreamCnv"}  
{
}

// Initialize

StatusCode SCTRawContByteStreamCnv::initialize()
{
  ATH_CHECK(AthConstConverter::initialize());
  ATH_MSG_DEBUG( " initialize " );

  ATH_CHECK(m_rawContByteStreamToolProvider.retrieve());
  m_rawContByteStreamTool = &(m_rawContByteStreamToolProvider->getTool());
  if (!m_rawContByteStreamTool) {
     ATH_MSG_FATAL("Failed to get SCTRawContByteStreamTool");
     return StatusCode::FAILURE;
  }  

  // Retrieve ByteStreamCnvSvc
  ATH_CHECK(m_byteStreamEventAccess.retrieve());
  ATH_MSG_INFO( "Retrieved service " << m_byteStreamEventAccess );

  return StatusCode::SUCCESS;
}

// Method to create RawEvent fragments

StatusCode SCTRawContByteStreamCnv::createRepConst(DataObject* pDataObject, IOpaqueAddress*& pOpaqueAddress) const
{
  // Get IDC for SCT Raw Data
  SCT_RDO_Container* sctRDOCont{nullptr};
  if (not SG::fromStorable(pDataObject, sctRDOCont)) {
    ATH_MSG_ERROR( " Can not cast to SCTRawContainer " );
    return StatusCode::FAILURE;
  }

  // Set up the IOpaqueAddress for Storegate
  std::string dataObjectName{pDataObject->registry()->name()};
  if ( pOpaqueAddress != nullptr ) pOpaqueAddress->release();
  pOpaqueAddress = new ByteStreamAddress(classID(), dataObjectName, "");
  pOpaqueAddress->addRef();

  // Use the tool to do the conversion
  ATH_CHECK(m_rawContByteStreamTool->convert(sctRDOCont) );

  return StatusCode::SUCCESS;
}
