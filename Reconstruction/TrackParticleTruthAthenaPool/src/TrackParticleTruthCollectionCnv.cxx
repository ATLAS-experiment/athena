/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackParticleTruthCollectionCnv.h"

#include "TrackParticleTruthTPCnv/TrackParticleTruthCollection_p0.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollection_p1.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollection_p2.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollection_p3.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollectionCnv_p1.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollectionCnv_p2.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollectionCnv_p3.h"

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"

// Athena
#include "StoreGate/StoreGateSvc.h"

// This is release 12 guid for TrackParticleTruthVector.
constexpr pool::Guid s_p0_guid("B35041E8-D980-458E-AC06-79028CF79D5D");
constexpr pool::Guid s_p1_guid("D62AFEEE-EF2C-437A-B7BE-CA926D38CCFA");
constexpr pool::Guid s_p2_guid("D62AFEEE-EF2C-437A-B7BE-CA926D38CCFB");
constexpr pool::Guid s_p3_guid("018F1ACE-8405-7174-9305-46B503B5EF52");

//================================================================
TrackParticleTruthCollectionCnv::TrackParticleTruthCollectionCnv(ISvcLocator* svcLoc) : 
  TrackParticleTruthCollectionCnvBase(svcLoc)
{}

//================================================================
TrackParticleTruthCollectionPERS* TrackParticleTruthCollectionCnv::createPersistent(TrackParticleTruthCollection* trans) {
  MsgStream log(msgSvc(), "TrackParticleTruthCollectionCnv");
  log<<MSG::DEBUG<<"Writing TrackParticleTruthCollection_p2"<<endmsg;
  TrackParticleTruthCollectionPERS* pers=new TrackParticleTruthCollectionPERS();
  m_converter_p3.transToPers(trans,pers,log);
  return pers;
}

//================================================================
TrackParticleTruthCollection* TrackParticleTruthCollectionCnv::createTransient() {
  MsgStream log(msgSvc(), "TrackParticleTruthCollectionCnv" );
  std::unique_ptr<TrackParticleTruthCollection> trans(new TrackParticleTruthCollection());
  
  if (compareClassGuid(s_p3_guid)) {
    log<<MSG::DEBUG<<"Read TrackParticleTruthCollection_p3. GUID="<<m_classID.toString()<<endmsg;
    Rec::TrackParticleTruthCollection_p3* pers=poolReadObject<Rec::TrackParticleTruthCollection_p3>();
    m_converter_p3.persToTrans(pers, trans.get(), log);
    delete pers;
  }
  else if (compareClassGuid(s_p2_guid)) {
    log<<MSG::DEBUG<<"Read TrackParticleTruthCollection_p2. GUID="<<m_classID.toString()<<endmsg;
    Rec::TrackParticleTruthCollection_p2* pers=poolReadObject<Rec::TrackParticleTruthCollection_p2>();
    m_converter_p2.persToTrans(pers, trans.get(), log);
    delete pers;
  }
  else if (compareClassGuid(s_p1_guid)) {
    log<<MSG::DEBUG<<"Read TrackParticleTruthCollection_p1. GUID="<<m_classID.toString()<<endmsg;
    Rec::TrackParticleTruthCollection_p1* pers=poolReadObject<Rec::TrackParticleTruthCollection_p1>();
    m_converter_p1.persToTrans(pers, trans.get(), log);
    delete pers;
  }
  else if (compareClassGuid(s_p0_guid)) {
    log<<MSG::DEBUG<<"Read version p0 of TrackParticleTruthCollection. GUID="<<m_classID.toString()<<endmsg;
    TrackParticleTruthVector *pers = poolReadObject<TrackParticleTruthVector>();
    m_converter_p0.persToTrans(pers, trans.get(), log);
    delete pers;
  }
  else {
    log<<MSG::ERROR<<"Unsupported persistent version of TrackParticleTruthCollection. GUID="
       <<m_classID.toString()<<endmsg;
    throw std::runtime_error("Unsupported persistent version of Data Collection");
  }
  
  return trans.release();
}
