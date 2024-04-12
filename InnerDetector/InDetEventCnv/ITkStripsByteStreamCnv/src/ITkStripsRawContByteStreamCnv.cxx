/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkStripsRawContByteStreamCnv.h"

#include "ITkStripsByteStreamCnv/IITkStripsRawContByteStreamTool.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h" 
#include "ByteStreamData/RawEvent.h" 
#include "ByteStreamCnvSvcBase/IByteStreamEventAccess.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "StoreGate/StoreGateSvc.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"

// Constructor

ITkStripsRawContByteStreamCnv::ITkStripsRawContByteStreamCnv(ISvcLocator* svcLoc) :
  AthConstConverter(storageType(), classID(), svcLoc, "ITkStripsRawContByteStreamCnv"),
  m_rawContByteStreamTool{"ITkStripsRawContByteStreamTool"},
  m_byteStreamEventAccess{"ByteStreamCnvSvc", "ITkStripsRawContByteStreamCnv"}
{
}

// Initialize

StatusCode ITkStripsRawContByteStreamCnv::initialize()
{
  ATH_CHECK(AthConstConverter::initialize());
  ATH_MSG_DEBUG( " initialize " );

  // Retrieve ByteStreamCnvSvc
  ATH_CHECK(m_byteStreamEventAccess.retrieve());
  ATH_MSG_VERBOSE( "Retrieved service " << m_byteStreamEventAccess );

  // Retrieve byte stream tool
  ATH_CHECK(m_rawContByteStreamTool.retrieve());
  ATH_MSG_VERBOSE( "Retrieved tool " << m_rawContByteStreamTool );

  return StatusCode::SUCCESS;
}

// Method to create RawEvent fragments

StatusCode ITkStripsRawContByteStreamCnv::createRepConst(DataObject* pDataObject, IOpaqueAddress*& pOpaqueAddress) const
{
  // Get IDC for ITK Raw Data
  SCT_RDO_Container* itkStripsRDOCont{nullptr};
  if (not SG::fromStorable(pDataObject, itkStripsRDOCont)) {
    ATH_MSG_ERROR( " Can not cast to SCTRawContainer " );
    return StatusCode::FAILURE;
  }

  // Set up the IOpaqueAddress for Storegate
  std::string dataObjectName{pDataObject->registry()->name()};
  pOpaqueAddress = new ByteStreamAddress(classID(), dataObjectName, "");
  pOpaqueAddress->addRef();//avoid memory leak 
  // Use the tool to do the conversion
  ATH_CHECK(m_rawContByteStreamTool->convert(itkStripsRDOCont) );

  return StatusCode::SUCCESS;
}