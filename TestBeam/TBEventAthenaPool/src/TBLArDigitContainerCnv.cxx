/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TBLArDigitContainerCnv.h"
#include "TBTPCnv/TBLArDigitContainer_p1.h"


TBLArDigitContainerCnv::TBLArDigitContainerCnv(ISvcLocator* svcLoc) : 
  TBLArDigitContainerCnvBase(svcLoc)
{}



TBLArDigitContainerPERS* TBLArDigitContainerCnv::createPersistent(TBLArDigitContainer* trans) {
    MsgStream log(msgSvc(), "TBLArDigitContainerCnv");
    log << MSG::DEBUG << "Writing TBLArDigitContainer_p2" << endmsg;
    TBLArDigitContainerPERS* pers=new TBLArDigitContainerPERS();
    m_converter.transToPers(trans,pers,log); 
    return pers;
}
    


TBLArDigitContainer* TBLArDigitContainerCnv::createTransient(const Token* token) {
   MsgStream log(msgSvc(), "TBLArDigitContainerCnv" );
   constexpr Guid p0_guid("B15FFDA0-206D-4062-8B5F-582A1ECD5502"); // GUID of the transient object
   constexpr Guid p1_guid("9F58DDD2-ACDC-4ECF-A714-779B05F94649");  // GUID of the persistent object
   auto trans = std::make_unique<TBLArDigitContainer>();
   TBLArDigitContainer* result{};
   if (compareClassGuid(token, p0_guid)) {
     log << MSG::DEBUG << "Read version p0 of TBLArDigitContainer. token=" 
         << token->toString() << endmsg;
     result = poolReadObject<TBLArDigitContainer>(token);
   } else if (compareClassGuid(token, p1_guid)) {
     log << MSG::DEBUG << "Reading TBLArDigitContainer_p1. token=" 
         << token->toString() << endmsg;
     std::unique_ptr<TBLArDigitContainer_p1> pers (poolReadObject<TBLArDigitContainer_p1>(token));
     m_converter.persToTrans(pers.get(),trans.get(), log);
     result = trans.release();
   } else {
     log << MSG::ERROR << "Unsupported persistent version of TBLArDigitContainer. token="
     << token->toString() << endmsg;
     throw std::runtime_error("Unsupported persistent version of Data Collection");
   }
   return result;
}

