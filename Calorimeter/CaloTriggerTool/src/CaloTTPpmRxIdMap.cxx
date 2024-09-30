/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloTriggerTool/CaloTTPpmRxIdMap.h"
#include "CaloIdentifier/CaloID_Exception.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IMessageSvc.h"

#include "AthenaKernel/errorcheck.h"
#include "StoreGate/StoreGateSvc.h"

#include <iostream>

CaloTTPpmRxIdMap::CaloTTPpmRxIdMap() {
}


CaloTTPpmRxIdMap::~CaloTTPpmRxIdMap() {
}

//------------------------------------------//
void CaloTTPpmRxIdMap::set( const CaloTTPpmRxId& m ) {

  convert_to_P(m);

  SmartIF<IMessageSvc> msgSvc{Gaudi::svcLocator()->service("MessageSvc")};
  if(!msgSvc){
      throw std::runtime_error("Cannot locate MessageSvc");
  }
  MsgStream log( msgSvc, "CaloTTPpmRxIdMap");

  log<<MSG::DEBUG<<" CaloTTPpmRxId size = "<<m.size() <<endmsg;

  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
  if(!detStore){
     log << MSG::ERROR <<  "Cannot locate DetectorStore" << endmsg;
  }

  CaloTTPpmRxId::const_iterator it  = m.begin();
  CaloTTPpmRxId::const_iterator it_e  = m.end();

    try {
        for (; it!=it_e; ++it) {
            const CaloTTPpmRxId_t& t = *it;

			L1CaloCoolChannelId ppmChannelId(t.ppm_crate, L1CaloModuleType::Ppm, t.ppm_module, t.ppm_subModule, t.ppm_channel);
			L1CaloRxCoolChannelId rxChannelId(t.rx_crate, t.rx_module, t.rx_inputConn, t.rx_inputPair, t.rx_outputConn, t.rx_outputPair);

            if (log.level()<=MSG::VERBOSE) {
              log << MSG::VERBOSE << " ppm id = " << ppmChannelId.id() << " rx id =" << rxChannelId.id() <<endmsg;
            }

			m_mPpmIdToRx[ppmChannelId].push_back(rxChannelId);
			m_mRxIdToPpm[rxChannelId] = ppmChannelId;

        }
        if (log.level()<=MSG::DEBUG) {
          log<<MSG::DEBUG<<" CaloTTPpmRxIdMap::set : number of Ids="<<m_mPpmIdToRx.size()<<std::endl;
        }
    } catch (CaloID_Exception& except) {
        log<<MSG::ERROR<<" Failed in CaloTTPpmRxIdMap::set " << endmsg;
        log<<MSG::ERROR<< (std::string) except  << endmsg ;
    }
  return;
}

//--------------------------------------------------------------------------//
std::vector<L1CaloRxCoolChannelId> CaloTTPpmRxIdMap::ppmToRxId(const L1CaloCoolChannelId& ppmChannelId) const {

    std::map<L1CaloCoolChannelId, std::vector<L1CaloRxCoolChannelId> >::const_iterator it = m_mPpmIdToRx.find(ppmChannelId);

    if(it!=m_mPpmIdToRx.end()){
        return it->second;
    }

    REPORT_MESSAGE_WITH_CONTEXT(MSG::ERROR, "CaloTTPpmRxIdMap") <<
      "Ppm channel ID not found, id= "<< ppmChannelId.id()<<" in Ppm to Rx map." <<endmsg;

    return std::vector<L1CaloRxCoolChannelId>();
}


//---------------------------------------------------------------------//
L1CaloCoolChannelId CaloTTPpmRxIdMap::rxToPpmId(const L1CaloRxCoolChannelId& rxChannelId) const {

    std::map<L1CaloRxCoolChannelId, L1CaloCoolChannelId >::const_iterator it=m_mRxIdToPpm.find(rxChannelId);

    if(it!=m_mRxIdToPpm.end()){
        return it->second;
    }

    // ERROR, can not find the id.
    REPORT_MESSAGE_WITH_CONTEXT(MSG::ERROR, "CaloTTPpmRxIdMap") <<
      "Rx channel ID not found, id = " <<rxChannelId.id()<<" in Rx to Ppm map."<< endmsg;

    return L1CaloCoolChannelId(0) ;
}

CaloTTPpmRxId_P* CaloTTPpmRxIdMap::getP() {
        return &m_persData;
}

void CaloTTPpmRxIdMap::convert_to_P( const CaloTTPpmRxId& d ) {

    CaloTTPpmRxId::const_iterator it = d.begin();
    CaloTTPpmRxId::const_iterator it_e = d.end();

    for ( ;it!=it_e;++it){
        const CaloTTPpmRxId_t& t = *it;
        CaloTTPpmRxId_P::__t t2 ;

        t2.ppm_crate = t.ppm_crate;
        t2.ppm_module = t.ppm_module;
        t2.ppm_subModule = t.ppm_subModule;
        t2.ppm_channel = t.ppm_channel;

        t2.rx_crate = t.rx_crate ;
        t2.rx_module = t.rx_module;
        t2.rx_inputConn = t.rx_inputConn;
        t2.rx_inputPair = t.rx_inputPair;
        t2.rx_outputConn = t.rx_outputConn;
        t2.rx_outputPair = t.rx_outputPair;

        m_persData.m_v.push_back(t2);
    }
}


void CaloTTPpmRxIdMap::convert_to_D( const CaloTTPpmRxId_P& p,  CaloTTPpmRxId& d ) {

    std::vector<CaloTTPpmRxId_P::__t>::const_iterator it = p.m_v.begin();
    std::vector<CaloTTPpmRxId_P::__t>::const_iterator it_e = p.m_v.end();

    d.clear();
    for ( ;it!=it_e;++it){
        const CaloTTPpmRxId_P::__t& t = *it;
        CaloTTPpmRxId_t t2 ;

        t2.ppm_crate = t.ppm_crate;
        t2.ppm_module = t.ppm_module;
        t2.ppm_subModule = t.ppm_subModule;
        t2.ppm_channel = t.ppm_channel;

        t2.rx_crate = t.rx_crate ;
        t2.rx_module = t.rx_module;
        t2.rx_inputConn = t.rx_inputConn;
        t2.rx_inputPair = t.rx_inputPair;
        t2.rx_outputConn = t.rx_outputConn;
        t2.rx_outputPair = t.rx_outputPair;

        d.push_back(t2);
    }
}

void CaloTTPpmRxIdMap::set (const CaloTTPpmRxId_P& p) {

    CaloTTPpmRxId d;
    convert_to_D(p,d);
    set(d);
    m_persData.m_version = p.m_version;
    return;
}
