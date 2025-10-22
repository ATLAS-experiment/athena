/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArDigitContainerCnv.h"
#include "LArTPCnv/LArDigitContainer_p1.h"
#include "LArTPCnv/LArDigitContainer_p2.h"
#include "LArTPCnv/LArDigitContainer_p3.h"
#include "LArTPCnv/LArDigitContainerCnv_p1.h"
#include "LArTPCnv/LArDigitContainerCnv_p2.h"
#include "LArTPCnv/LArDigitContainerCnv_p3.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include <memory>


LArDigitContainerCnv::LArDigitContainerCnv(ISvcLocator* svcLoc) : 
  LArDigitContainerCnvBase(svcLoc, "LArDigitContainerCnv"),
  m_storeGateSvc("StoreGateSvc", "LArDigitContainerCnv")
{}


StatusCode LArDigitContainerCnv::initialize() {

  const LArOnlineID* idHelper=nullptr;
  ATH_CHECK( detStore()->retrieve(idHelper,"LArOnlineID") );
  m_idHelper=idHelper;//cast to base-class

  const LArOnline_SuperCellID* idSCHelper=nullptr;
  ATH_CHECK( detStore()->retrieve(idSCHelper,"LArOnline_SuperCellID") );
  m_idSCHelper=idSCHelper;//cast to base-class

  ATH_CHECK( m_storeGateSvc.retrieve() );

  return LArDigitContainerCnvBase::initialize();
}


LArDigitContainerPERS* LArDigitContainerCnv::createPersistent(LArDigitContainer* trans) {
    ATH_MSG_DEBUG("Writing LArDigitContainer_p3");
    LArDigitContainerPERS* pers=new LArDigitContainerPERS();
    LArDigitContainerCnv_p3 converter(m_idHelper, m_idSCHelper, m_storeGateSvc.get());
    converter.transToPers(trans, pers, msg());
    return pers;
}


LArDigitContainer* LArDigitContainerCnv::createTransient() {

   constexpr pool::Guid p0_guid("B15FFDA0-206D-4062-8B5F-582A1ECD5502");
   constexpr pool::Guid p1_guid("F1876026-CDFE-4110-AA59-E441BAA5DE44");
   constexpr pool::Guid p2_guid("66F5B7AF-595C-4F79-A2B7-56590777C313");
   constexpr pool::Guid p3_guid("24480EBA-1AF1-4646-95A7-11285F09717C");

   if (compareClassGuid(p0_guid)) {
     ATH_MSG_DEBUG("Read version p0 of LArDigitContainer. GUID=" << m_classID.toString());
     return poolReadObject<LArDigitContainer>();
   }
   else if (compareClassGuid(p1_guid)) {
     ATH_MSG_DEBUG("Reading LArDigitContainer_p1. GUID=" << m_classID.toString());
     LArDigitContainer* trans=new LArDigitContainer();
     std::unique_ptr<LArDigitContainer_p1> pers(poolReadObject<LArDigitContainer_p1>());
     LArDigitContainerCnv_p1 converter;
     converter.persToTrans(pers.get(), trans, msg());
     return trans;
   } 
   else if (compareClassGuid(p2_guid)) {
     ATH_MSG_DEBUG("Reading LArDigitContainer_p2. GUID=" << m_classID.toString());
     LArDigitContainer* trans=new LArDigitContainer();
     std::unique_ptr<LArDigitContainer_p2> pers(poolReadObject<LArDigitContainer_p2>());
     LArDigitContainerCnv_p2 converter(m_idHelper);
     converter.persToTrans(pers.get(), trans, msg());
     return trans;
   }
   else if (compareClassGuid(p3_guid)) {
     ATH_MSG_DEBUG("Reading LArDigitContainer_p3. GUID=" << m_classID.toString());
     LArDigitContainer* trans=new LArDigitContainer();
     std::unique_ptr<LArDigitContainer_p3> pers(poolReadObject<LArDigitContainer_p3>());
     LArDigitContainerCnv_p3 converter(m_idHelper, m_idSCHelper, m_storeGateSvc.get());
     converter.persToTrans(pers.get(), trans, msg());
     return trans;
   }
   ATH_MSG_ERROR("Unsupported persistent version of LArDigitContainer. GUID=" << m_classID.toString());
   throw std::runtime_error("Unsupported persistent version of Data Collection");
   // not reached
}
