///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Framework includes
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "AthenaKernel/ExtendedEventContext.h"

// GeneratorObjectsAthenaPool includes
#include "GeneratorObjectsTPCnv/HepMcParticleLinkCnv_p1.h"
#include "GeneratorObjects/McEventCollection.h"

///////////////////////////////////////////////////////////////////
/// Public methods:
///////////////////////////////////////////////////////////////////


// In the 21.0 branch an eventIndex of zero means that the transient
// HepMcParticleLink looks at the first event in the
// McEventCollection, otherwise the eventIndex represents the
// GenEvent::event_number of a GenEvent in the McEventCollection and
// this is used to find the appropriate GenEvent.
// NB There is a weakness here that if more than one GenEvent has
// the same event number all links will point to the first GenEvent
// which matches.

void HepMcParticleLinkCnv_p1::persToTrans( const HepMcParticleLink_p1* persObj,
                                           HepMcParticleLink* transObj,
                                           MsgStream &/*msg*/ ) const
{
  HepMcParticleLink::PositionFlag flag = HepMcParticleLink::IS_EVENTNUM;

  if (persObj->m_mcEvtIndex == 0) {
    flag = HepMcParticleLink::IS_POSITION;
  }

  transObj->setExtendedBarCode
    ( HepMcParticleLink::ExtendedBarCode( persObj->m_barcode,
                                          persObj->m_mcEvtIndex,
                                          flag) );
}

void HepMcParticleLinkCnv_p1::transToPers( const HepMcParticleLink* transObj,
                                           HepMcParticleLink_p1* persObj,
                                           MsgStream &msg ) const
{
  // NB This method assumes that there all GenEvents are stored in a
  // single McEventCollection, as running with split
  // McEventCollections is not supported in 21.0.
  const EventContext& ctx = Gaudi::Hive::currentContext();
  const IProxyDict* proxy = Atlas::getExtendedEventContext(ctx).proxy();
  unsigned short index{0};
  const HepMcParticleLink::index_type position =
    HepMcParticleLink::getEventPositionInCollection(transObj->eventIndex(),
                                                    proxy).at(0);
  if (position!=0) {
    index = transObj->eventIndex();
    if(transObj->eventIndex()!=static_cast<HepMcParticleLink::index_type>(index)) {
      msg << MSG::WARNING << "Attempting to persistify an eventIndex larger than max unsigned short!" << endmsg;
    }
  }
  persObj->m_mcEvtIndex = index;
  persObj->m_barcode    = transObj->barcode();
}

