/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArNoisyROSummaryCnv.h"

#include "GaudiKernel/StatusCode.h"
#include "StoreGate/StoreGateSvc.h"
#include "LArTPCnv/LArNoisyROSummaryCnv_p1.h"
#include "LArTPCnv/LArNoisyROSummaryCnv_p2.h"
#include "LArTPCnv/LArNoisyROSummaryCnv_p3.h"
#include "LArTPCnv/LArNoisyROSummaryCnv_p4.h"
#include "LArTPCnv/LArNoisyROSummaryCnv_p5.h"


LArNoisyROSummaryCnv::LArNoisyROSummaryCnv(ISvcLocator* svcLoc) : 
  LArNoisyROSummaryCnvBase(svcLoc, "LArNoisyROSummaryConverter")
{}


LArNoisyROSummary_PERSISTENT*
LArNoisyROSummaryCnv::createPersistent(LArNoisyROSummary* transCont)
{
  LArNoisyROSummary_p6      *persObj = m_converter.createPersistent( transCont, msg() );
  return persObj; 
}

LArNoisyROSummary*
LArNoisyROSummaryCnv::createTransient(const Token* token)
{
  LArNoisyROSummary         *trans = NULL;
  
  // GUID for persistent classes
  static const pool::Guid   guid_p1("4681BC21-3C00-4540-BED6-58E37700D9B9");
  static const pool::Guid   guid_p2("C33CED2C-2101-4B0C-9BCB-739B004639F4");
  static const pool::Guid   guid_p3("7801CF21-F2F2-4E87-9B87-744F31A37D1B");
  static const pool::Guid   guid_p4("8F9E9A44-699E-4056-96CC-555ADA1179D4");
  static const pool::Guid   guid_p5("4AE11DAE-F40C-4B90-B105-0A7BA5D29C1D");
  static const pool::Guid   guid_p6("D2B7F48F-058E-47C9-902D-0A847F5E2194");

  if( compareClassGuid(token, guid_p6) ) {
     std::unique_ptr<LArNoisyROSummary_p6> col_vect( poolReadObject<LArNoisyROSummary_p6>(token) );
     trans = m_converter.createTransient( col_vect.get(), msg() );
  }
  else if( compareClassGuid(token, guid_p5) ) {
     LArNoisyROSummaryCnv_p5   converter;
     std::unique_ptr<LArNoisyROSummary_p5> col_vect( poolReadObject<LArNoisyROSummary_p5>(token) );
     trans = converter.createTransient( col_vect.get(), msg() );
  }
  else if( compareClassGuid(token, guid_p4) ) {
     LArNoisyROSummaryCnv_p4   converter;
     std::unique_ptr<LArNoisyROSummary_p4> col_vect( poolReadObject<LArNoisyROSummary_p4>(token) );
     trans = converter.createTransient( col_vect.get(), msg() );
  }
  else if( compareClassGuid(token, guid_p3) ) {
      LArNoisyROSummaryCnv_p3   converter;
      std::unique_ptr<LArNoisyROSummary_p3> col_vect( poolReadObject<LArNoisyROSummary_p3>(token) );
      trans = converter.createTransient( col_vect.get(), msg() );
  }
  else if( compareClassGuid(token, guid_p2) ) {
      LArNoisyROSummaryCnv_p2   converter;
      std::unique_ptr<LArNoisyROSummary_p2> col_vect( poolReadObject<LArNoisyROSummary_p2>(token) );
      trans = converter.createTransient( col_vect.get(), msg() );
  }
  else if( compareClassGuid(token, guid_p1) ) {
      LArNoisyROSummaryCnv_p1   converter;
      std::unique_ptr<LArNoisyROSummary_p1> col_vect( poolReadObject<LArNoisyROSummary_p1>(token) );
      trans = converter.createTransient( col_vect.get(), msg() );
  }
  else {
      throw std::runtime_error("Unsupported persistent version of LArNoisyROSummary ") ;
    }
  return trans;
}
