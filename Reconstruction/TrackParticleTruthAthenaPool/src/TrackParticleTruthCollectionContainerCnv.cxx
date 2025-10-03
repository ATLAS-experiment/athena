/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackParticleTruthCollectionContainerCnv.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollectionContainerCnv_tlp1.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollectionContainerCnv_tlp2.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollectionContainerCnv_tlp3.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollection_p1.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollection_p2.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollection_p3.h"
#include "ParticleTruth/TrackParticleTruthVector.h"

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"


TrackParticleTruthCollectionContainerCnv::TrackParticleTruthCollectionContainerCnv(ISvcLocator* svcLoc) : 
  TrackParticleTruthCollectionContainerCnvBase(svcLoc),
  m_converter_p1(new TrackParticleTruthCollectionContainerCnv_tlp1),
  m_converter_p2(new TrackParticleTruthCollectionContainerCnv_tlp2),
  m_converter_p3(new TrackParticleTruthCollectionContainerCnv_tlp3)
{;}


TrackParticleTruthCollectionContainerCnv::~TrackParticleTruthCollectionContainerCnv(){
  delete m_converter_p1;
  delete m_converter_p2;
  delete m_converter_p3;
}

TrackParticleTruthCollectionContainerPERS* TrackParticleTruthCollectionContainerCnv::createPersistent(TrackParticleTruthCollectionContainer* trans) {
  MsgStream log(msgSvc(), "TrackParticleTruthCollectionContainerCnv");
  log<<MSG::DEBUG<<"Writing TrackParticleTruthCollectionContainer_tlp3"<<endmsg;

  TrackParticleTruthCollectionContainerPERS* p_cont =  m_converter_p3->createPersistent( trans, log );
  return p_cont;
}


TrackParticleTruthCollectionContainer* TrackParticleTruthCollectionContainerCnv::createTransient() {
  pool::Guid p1_guid("9F47124C-0033-4556-B14A-D7F28E4249EC");
  pool::Guid p2_guid("9F47124C-0033-4556-B14A-D7F28E4249ED");
  pool::Guid p3_guid("018F1AD6-09F9-7634-A065-2343D74DD289");

  MsgStream log(msgSvc(), "TrackParticleTruthCollectionContainerCnv" );
  TrackParticleTruthCollectionContainer *p_collection = 0;
  if( compareClassGuid( p3_guid ) ){
      poolReadObject< TrackParticleTruthCollectionContainerPERS >(*m_converter_p3);
      p_collection = m_converter_p3->createTransient( log );
  }
  else if( compareClassGuid( p2_guid ) ){
      poolReadObject< TrackParticleTruthCollectionContainerPERS >(*m_converter_p2);
      p_collection = m_converter_p2->createTransient( log );
  }
  else if ( compareClassGuid( p1_guid ) ){
      poolReadObject< TrackParticleTruthCollectionContainerPERS >(*m_converter_p1);
      p_collection = m_converter_p1->createTransient( log );
  }
  else {
    throw std::runtime_error( "Unsupported persistent version of TrigTrackCountsCollection" );
  }
 
  return p_collection;
}
