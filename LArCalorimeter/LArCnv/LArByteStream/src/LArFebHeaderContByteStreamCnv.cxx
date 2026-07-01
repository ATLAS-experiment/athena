/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArByteStream/LArFebHeaderContByteStreamCnv.h"
#include "LArByteStream/LArRawDataContByteStreamTool.h"

#include "CaloIdentifier/CaloGain.h"

#include "ByteStreamCnvSvcBase/IByteStreamCnvSvc.h"
#include "ByteStreamCnvSvcBase/ByteStreamCnvSvcBase.h" 
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h" 
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h" 
#include "ByteStreamData/RawEvent.h" 

#include "LArRawEvent/LArFebHeaderContainer.h"

#include "AthenaKernel/CLASS_DEF.h"

//STL-Stuff
#include <map> 
#include <iostream>


LArFebHeaderContByteStreamCnv::LArFebHeaderContByteStreamCnv(ISvcLocator* svcloc) :
  AthConstConverter(storageType(), classID(),svcloc,"LArFebHeaderContByteStreamCnv"),
  m_tool("LArRawDataContByteStreamTool"),
  m_rdpSvc("ROBDataProviderSvc", name())
{}

CLID LArFebHeaderContByteStreamCnv::classID(){
  return ClassID_traits<LArFebHeaderContainer>::ID() ;
}


StatusCode
LArFebHeaderContByteStreamCnv::initialize()
{
  ATH_CHECK( AthConstConverter::initialize() );

  if ( m_rdpSvc.retrieve().isFailure()) {
    ATH_MSG_WARNING(  " Can't get ByteStreamInputSvc interface Reading of ByteStream Data not possible. " );
  }

  ATH_CHECK( m_tool.retrieve() );

  return StatusCode::SUCCESS;
}


StatusCode LArFebHeaderContByteStreamCnv::createObjConst(IOpaqueAddress* pAddr, DataObject*& pObj)  const
{//Convert Digits from ByteStream to StoreGate
  ATH_MSG_VERBOSE( "Executing CreateObj method for LArFebHeaderContainer " );

  if (!m_rdpSvc) {
    ATH_MSG_ERROR( " ROBDataProviderSvc not loaded. Can't read ByteStream." );
    return StatusCode::FAILURE;
  }

  ByteStreamAddress* pRE_Addr{};
  ATH_CHECK( (pRE_Addr = dynamic_cast<ByteStreamAddress*>(pAddr)) != nullptr );

  const EventContext& ctx = pRE_Addr->getEventContext();
  const RawEvent* re = m_rdpSvc->getEvent(ctx);
  if (!re) {
    ATH_MSG_ERROR( "Could not get raw event from ByteStreamInputSvc" );
    return StatusCode::FAILURE;
  }

  // Convert the RawEvent to  LArFebHeaderContainer
  ATH_MSG_DEBUG( "Converting LArFebHeaders (from ByteStream)." );
  LArFebHeaderContainer *febHeaderContainer=new LArFebHeaderContainer;
  StatusCode sc=m_tool->convert(re,febHeaderContainer,(CaloGain::CaloGain)0);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_WARNING( "Conversion tool returned an error. LArFebHeaderContainer might be empty." );
  }
    
  pObj = SG::asStorable(febHeaderContainer) ;

  return StatusCode::SUCCESS;
}

StatusCode LArFebHeaderContByteStreamCnv::createRepConst(DataObject* /*pObj*/, IOpaqueAddress*& /*pAddr*/)  const
{// convert LArFebHeaders from StoreGate into ByteStream
  ATH_MSG_ERROR( "CreateRep method of LArFebHeaderContainer not implemented!" );
  return StatusCode::SUCCESS;
}
