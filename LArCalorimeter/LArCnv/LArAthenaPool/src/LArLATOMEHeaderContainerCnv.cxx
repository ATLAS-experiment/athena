/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArLATOMEHeaderContainerCnv.h"
#include "LArTPCnv/LArLATOMEHeaderContainer_p1.h"
#include "LArTPCnv/LArLATOMEHeaderContainerCnv_p1.h"
#include <memory>

LArLATOMEHeaderContainerCnv::LArLATOMEHeaderContainerCnv(ISvcLocator* svcLoc) : 
  LArLATOMEHeaderContainerCnvBase(svcLoc, "LArLATOMEHeaderContainerCnv"),
  m_p1_guid("7FE06234-8574-4514-86C7-1FD09E97D713")
{}
  
StatusCode LArLATOMEHeaderContainerCnv::initialize() {

  return LArLATOMEHeaderContainerCnvBase::initialize();
}


LArLATOMEHeaderContainerPERS* LArLATOMEHeaderContainerCnv::createPersistent(LArLATOMEHeaderContainer* trans) {
    ATH_MSG_DEBUG("Writing LArDigitContainer_p2");
    LArLATOMEHeaderContainerPERS* pers=new LArLATOMEHeaderContainerPERS();
    LArLATOMEHeaderContainerCnv_p1 converter;
    converter.transToPers(trans, pers, msg());
    return pers;
}
    


LArLATOMEHeaderContainer* LArLATOMEHeaderContainerCnv::createTransient(const Token* token) {
   if (compareClassGuid(token, m_p1_guid)) {
     ATH_MSG_DEBUG("Reading LArLATOMEHeaderContainer_p1. token=" << token->toString());
     LArLATOMEHeaderContainer* trans=new LArLATOMEHeaderContainer();
     std::unique_ptr<LArLATOMEHeaderContainer_p1> pers(poolReadObject<LArLATOMEHeaderContainer_p1>());
     LArLATOMEHeaderContainerCnv_p1 converter;
     converter.persToTrans(pers.get(), trans, msg());
     return trans;
   } 
   ATH_MSG_ERROR("Unsupported persistent version of LArLATOMEHeaderContainer. token=" << token->toString());
   throw std::runtime_error("Unsupported persistent version of Data Collection");
   // not reached
}
