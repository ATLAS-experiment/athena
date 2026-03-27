/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MMPrepDataContainerCnv.h"

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"

MMPrepDataContainerCnv::MMPrepDataContainerCnv(ISvcLocator* svcloc) :
MMPrepDataContainerCnvBase(svcloc)
{
}

MMPrepDataContainerCnv::~MMPrepDataContainerCnv() = default;

StatusCode MMPrepDataContainerCnv::initialize() {
   // Call base clase initialize
    if( !MMPrepDataContainerCnvBase::initialize().isSuccess() )
       return StatusCode::FAILURE;
    
   // Get the messaging service, print where you are
    MsgStream log(msgSvc(), "MMPrepDataContainerCnv");
    if (log.level() <= MSG::INFO) log << MSG::INFO << "MMPrepDataContainerCnv::initialize()" << endmsg;

    return StatusCode::SUCCESS;
}

MMPrepDataContainer_PERS*    MMPrepDataContainerCnv::createPersistent (Muon::MMPrepDataContainer* transCont) {
    MsgStream log(msgSvc(), "MMPrepDataContainerCnv" );
    if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createPersistent(): main converter"<<endmsg;
    MMPrepDataContainer_PERS *pers= m_converter_p2.createPersistent( transCont, log );
    // COMPRESS MMPrepDataContainer_PERS *pers= m_converter_p1.createPersistent( transCont, log );
    return pers;
}

Muon::MMPrepDataContainer* MMPrepDataContainerCnv::createTransient(const Token* token) {
    MsgStream log(msgSvc(), "MMPrepDataContainerCnv" );
    static const pool::Guid   p1_guid("5E5C97E2-8D7C-11E2-9FCB-001517648C14"); 
    static const pool::Guid   p2_guid("86B42223-304D-4AFA-8E94-A49B23AC8865"); 
    if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createTransient(const Token* token): main converter"<<endmsg;
    Muon::MMPrepDataContainer* p_collection(nullptr);
    if( compareClassGuid(token, p1_guid) ) {
        if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createTransient(const Token* token): T/P version 1 detected"<<endmsg;
        std::unique_ptr< Muon::MMPrepDataContainer_p1 >  p_coll( poolReadObject< Muon::MMPrepDataContainer_p1 >(token) );
        p_collection = m_converter_p1.createTransient( p_coll.get(), log );
    } else if(compareClassGuid(token, p2_guid)) {
        if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createTransient(const Token* token): T/P version 2 detected"<<endmsg;
        std::unique_ptr< Muon::MMPrepDataContainer_p2 >  p_coll( poolReadObject< Muon::MMPrepDataContainer_p2 >(token) );
        p_collection = m_converter_p2.createTransient( p_coll.get(), log );

    }else {
        throw std::runtime_error("Unsupported persistent version of MMPrepDataContainer.");
 
    }
    return p_collection;
}
