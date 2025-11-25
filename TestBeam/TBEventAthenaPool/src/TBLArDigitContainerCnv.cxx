/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TBLArDigitContainerCnv.h"
#include "TBTPCnv/TBLArDigitContainer_p1.h"
//#include "LArRawEvent/LArDigitContainer.h"


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
    


TBLArDigitContainer* TBLArDigitContainerCnv::createTransient() {
   MsgStream log(msgSvc(), "TBLArDigitContainerCnv" );
   constexpr pool::Guid p0_guid("B15FFDA0-206D-4062-8B5F-582A1ECD5502"); // GUID of the transient object
   constexpr pool::Guid p1_guid("9F58DDD2-ACDC-4ECF-A714-779B05F94649");  // GUID of the persistent object
   auto trans = std::make_unique<TBLArDigitContainer>();
   if (compareClassGuid(p0_guid)) {
     log << MSG::DEBUG << "Read version p0 of TBLArDigitContainer. GUID=" 
         << m_classID.toString() << endmsg;
     return poolReadObject<TBLArDigitContainer>();
   }
   else if (compareClassGuid(p1_guid)) {
     log << MSG::DEBUG << "Reading TBLArDigitContainer_p1. GUID=" 
         << m_classID.toString() << endmsg;
     std::unique_ptr<TBLArDigitContainer_p1> pers (poolReadObject<TBLArDigitContainer_p1>());
     m_converter.persToTrans(pers.get(),trans.get(), log);
     return trans.release();
   }
   else {
     log << MSG::ERROR << "Unsupported persistent version of TBLArDigitContainer. GUID="
     << m_classID.toString() << endmsg;
     throw std::runtime_error("Unsupported persistent version of Data Collection");
   }
   return trans.release();
}

