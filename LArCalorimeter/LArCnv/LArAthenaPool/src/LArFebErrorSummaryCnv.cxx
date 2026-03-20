/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArFebErrorSummaryCnv.h"

#include "GaudiKernel/StatusCode.h"
#include "StoreGate/StoreGateSvc.h"
#include "LArTPCnv/LArFebErrorSummaryCnv_p1.h"


LArFebErrorSummaryCnv::LArFebErrorSummaryCnv(ISvcLocator* svcLoc) : 
  LArFebErrorSummaryCnvBase(svcLoc, "LArFebErrorSummaryCnv")
{}


LArFebErrorSummary_PERSISTENT*
LArFebErrorSummaryCnv::createPersistent(LArFebErrorSummary* transCont)
{
  LArFebErrorSummary_p1      *persObj = m_converter.createPersistent( transCont, msg() );
  return persObj; 
}

LArFebErrorSummary*
LArFebErrorSummaryCnv::createTransient(const Token* token)
{
  LArFebErrorSummary         *trans = NULL;
  
  // GUID for persistent classes
  static const pool::Guid   guid_p1("9448FB64-AB7E-4995-A5FC-23E9A6C1AF80");

  if( compareClassGuid(token, guid_p1) ) {
      std::unique_ptr<LArFebErrorSummary_p1> col_vect( poolReadObject<LArFebErrorSummary_p1>() );
      trans = m_converter.createTransient( col_vect.get(), msg() );
  }
  else {
    //      log << MSG::ERROR << "failed trying to read : " << m_token << endmsg;
      throw std::runtime_error("Unsupported persistent version of LArFebErrorSummary ") ;
    }
  return trans;
}
