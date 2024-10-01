/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloTriggerTool/CaloTTOnOffIdMap.h"
#include "CaloIdentifier/TTOnlineID.h"
#include "CaloIdentifier/CaloLVL1_ID.h"
#include "CaloIdentifier/CaloID_Exception.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IMessageSvc.h"

#include "AthenaKernel/errorcheck.h"
#include "StoreGate/StoreGateSvc.h"

#include <iostream>

CaloTTOnOffIdMap::CaloTTOnOffIdMap() {
}


CaloTTOnOffIdMap::~CaloTTOnOffIdMap() {
}

//------------------------------------------//
void CaloTTOnOffIdMap::set( const CaloTTOnOffId& m ) {

  convert_to_P(m);

  SmartIF<IMessageSvc> msgSvc{Gaudi::svcLocator()->service("MessageSvc")};
  if(!msgSvc){
      throw std::runtime_error("Cannot locate MessageSvc");
  }
  MsgStream log( msgSvc, "CaloTTOnOffIdMap");

  log<<MSG::DEBUG<<" CaloTTOnOffId size = "<<m.size() <<endmsg;

  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
  if(!detStore){
      log << MSG::ERROR <<  "Cannot locate DetectorStore" << endmsg;
  }

  const TTOnlineID* online_id = nullptr;
  const CaloLVL1_ID* offline_id = nullptr;

  StatusCode status=detStore->retrieve(online_id);
  if(status.isFailure()){
    log << MSG::ERROR <<  "Cannot retrieve online_id" << endmsg;
  }
  status=detStore->retrieve(offline_id);
  if(status.isFailure()){
    log << MSG::ERROR <<  "Cannot retrieve offline_id" << endmsg;
  }

  CaloTTOnOffId::const_iterator it  = m.begin();
  CaloTTOnOffId::const_iterator it_e  = m.end();

    try {
        for (; it!=it_e; ++it) {
            const CaloTTOnOffId_t& t = *it;

            Identifier id = offline_id->tower_id(t.pn, t.sampling, t.region, t.eta, t.phi);
            HWIdentifier sid = online_id->channelId(t.crate, t.module, t.submodule, t.channel);

            if (log.level()<=MSG::VERBOSE) {
              log<<MSG::VERBOSE
              << " db struct= "
              <<" pn="<<t.pn<<" sampling="<<t.sampling
              <<" region="<<t.region
              <<" eta="<<t.eta<<" phi="<<t.phi<<" layer="<<t.layer<<" | "
              <<" crate="<<t.crate<<" module="<<t.module
              <<" submodule="<<t.submodule
              <<" channel="<<t.channel
              << endmsg;

              log<<MSG::VERBOSE<< " onl id = " << sid<<" offline id ="<<id<<endmsg;
            }

            m_off2onIdMap[id] = sid;
            m_on2offIdMap[sid] = id;
        }
        if (log.level()<=MSG::DEBUG) {
            log<<MSG::DEBUG<<" CaloTTOnOffIdMap::set : number of Ids="<<m_on2offIdMap.size()<<std::endl;
        }
    } catch (CaloID_Exception& except) {
        log<<MSG::ERROR<<" Failed in CaloTTOnOffIdMap::set " << endmsg;
        log<<MSG::ERROR<< (std::string) except  << endmsg ;
    }
  return;
}

//--------------------------------------------------------------------------//
HWIdentifier CaloTTOnOffIdMap::createSignalChannelID(const Identifier& id, bool bQuiet) const {

    std::map<Identifier,HWIdentifier>::const_iterator it =m_off2onIdMap.find(id);

    if(it!=m_off2onIdMap.end()){
        return (*it).second;
    }

    if(bQuiet) {
    	return HWIdentifier(0);

    } else {

        REPORT_MESSAGE_WITH_CONTEXT(MSG::ERROR, "CaloTTOnOffIdMap") <<
          "Offline ID not found "<< id <<endmsg;

		return HWIdentifier(0);
	}
}


//---------------------------------------------------------------------//
Identifier CaloTTOnOffIdMap::cnvToIdentifier(const HWIdentifier & sid, bool bQuiet)const {

    std::map<HWIdentifier,Identifier>::const_iterator it=m_on2offIdMap.find(sid);

    if(it!=m_on2offIdMap.end()){
        return (*it).second;
    }

    if(bQuiet) {
    	return Identifier(0);

    } else {

        // ERROR, can not find the channelId.
        REPORT_MESSAGE_WITH_CONTEXT(MSG::ERROR, "CaloTTOnOffIdMap") <<
          "Online ID not found, id = " <<sid.get_compact()<< endmsg;

		return Identifier(0) ;
    }
}

CaloTTOnOffId_P* CaloTTOnOffIdMap::getP() {
        return &m_persData;
}

void CaloTTOnOffIdMap::convert_to_P( const CaloTTOnOffId& d ) {

    CaloTTOnOffId::const_iterator it = d.begin();
    CaloTTOnOffId::const_iterator it_e = d.end();

    for ( ;it!=it_e;++it){
        const CaloTTOnOffId_t& t = *it;
        CaloTTOnOffId_P::__t t2 ;

        t2.pn  = t.pn;
        t2.sampling = t.sampling;
        t2.region = t.region;
        t2.eta = t.eta;
        t2.phi = t.phi;
        t2.layer = t.layer;

        t2.crate = t.crate ;
        t2.module = t.module;
        t2.submodule= t.submodule;
        t2.channel = t.channel;

        m_persData.m_v.push_back(t2);
    }
}


void CaloTTOnOffIdMap::convert_to_D( const CaloTTOnOffId_P& p,  CaloTTOnOffId& d ) {

    std::vector<CaloTTOnOffId_P::__t>::const_iterator it = p.m_v.begin();
    std::vector<CaloTTOnOffId_P::__t>::const_iterator it_e = p.m_v.end();

    d.clear();
    for ( ;it!=it_e;++it){
        const CaloTTOnOffId_P::__t& t = *it;
        CaloTTOnOffId_t t2 ;

        t2.pn  = t.pn;
        t2.sampling = t.sampling;
        t2.region = t.region;
        t2.eta = t.eta;
        t2.phi = t.phi;
        t2.layer = t.layer;

        t2.crate = t.crate ;
        t2.module = t.module;
        t2.submodule= t.submodule;
        t2.channel = t.channel;

        d.push_back(t2);
    }
}

void CaloTTOnOffIdMap::set (const CaloTTOnOffId_P& p) {

    CaloTTOnOffId d;
    convert_to_D(p,d);
    set(d);
    m_persData.m_version = p.m_version;
    return;
}
