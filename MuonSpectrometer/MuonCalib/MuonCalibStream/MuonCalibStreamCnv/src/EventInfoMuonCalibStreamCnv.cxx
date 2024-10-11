/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCalibStreamCnv/EventInfoMuonCalibStreamCnv.h"

#include <time.h>
#include <iostream>
#include <memory>

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/StatusCode.h"
#include "MuCalDecode/CalibEvent.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamCnvSvc.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamDataProviderSvc.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamInputSvc.h"
#include "AthenaKernel/StorableConversions.h"

// Update the EventInfo to xAOD::EventInfo 071120223
// Instantiation of a static factory class used by clients to create
// instances of this service
// static CnvFactory<EventInfoMuonCalibStreamCnv> s_factory;
// const  ICnvFactory& EventInfoMuonCalibStreamCnvFactory = s_factory;

EventInfoMuonCalibStreamCnv::EventInfoMuonCalibStreamCnv(ISvcLocator *svcloc) :
    Converter(storageType(), classID(), svcloc),
    AthMessaging(msgSvc(), "EventInfoMuonCalibStreamCnv"),
    m_MuonCalibStreamCnvSvc("MuonCalibStreamCnvSvc", "EventInfoMuonCalibStreamCnv"),
    m_dataProvider("MuonCalibStreamDataProviderSvc", "EventInfoMuonCalibStreamCnv")
{}

const CLID &EventInfoMuonCalibStreamCnv::classID() { return ClassID_traits<xAOD::EventInfo>::ID(); }

StatusCode EventInfoMuonCalibStreamCnv::initialize() {
    ATH_MSG_DEBUG("Initialize EventInfoMuonCalibStreamCnv");

    ATH_CHECK(Converter::initialize());

    ATH_CHECK( m_MuonCalibStreamCnvSvc.retrieve() );
    ATH_CHECK( m_dataProvider.retrieve() );

    return StatusCode::SUCCESS;
}

StatusCode EventInfoMuonCalibStreamCnv::createObj(IOpaqueAddress *pAddr, DataObject *&pObj) {

    MuonCalibStreamAddress *pRE_Addr;
    pRE_Addr = dynamic_cast<MuonCalibStreamAddress *>(pAddr);
    if (!pRE_Addr) {
        ATH_MSG_ERROR(" Cannot cast to MuonCalibStreamAddress ");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG(" Creating Objects   ");

    // get CalibEvent
    const LVL2_MUON_CALIBRATION::CalibEvent *event = m_dataProvider->getEvent();
    if (!event) {
        ATH_MSG_ERROR(" Can not get CalibEvent ");
        return StatusCode::FAILURE;
    }

    // Build EventInfo 
    uint64_t eventNumber = event->lvl1_id();
    int runNumber = event->run_number();
    float pt = event->pt();
    // Time Stamp
    uint64_t timeStamp = event->timestamp();
    int lb_nr(0);

    // create xAOD::EventInfo objects directly
    xAOD::EventInfo* evtInfo = new xAOD::EventInfo();
    xAOD::EventAuxInfo* evtAuxInfo = new xAOD::EventAuxInfo();
    evtInfo->setStore(evtAuxInfo);

    evtInfo->setRunNumber(runNumber);
    evtInfo->setEventNumber(eventNumber);
    evtInfo->setLumiBlock(lb_nr);
    evtInfo->setTimeStamp(timeStamp);
    evtInfo->setBCID(0);
    evtInfo->setTimeStampNSOffset(pt);
    uint32_t eventTypeBitmask = 0;
    eventTypeBitmask |= xAOD::EventInfo::IS_CALIBRATION;
    evtInfo->setEventTypeBitmask( eventTypeBitmask );
    
    pObj = SG::asStorable(evtInfo);

    // PRINT TIMESTAMP INFO
    ATH_MSG_DEBUG("New EventInfo made, run/event/timestamp/pt/LB = "<<runNumber<<" "<<eventNumber<<" "<<timeStamp<<" "<<pt<<" "<<lb_nr);

    return StatusCode::SUCCESS;
}  // EventInfoMuonCalibStreamCnv::createObj()

// Fill RawEvent object from DataObject
// (but really just configure the message service)
StatusCode EventInfoMuonCalibStreamCnv::createRep(DataObject * /* pObj */, IOpaqueAddress *& /* pAddr */) {
    ATH_MSG_DEBUG(" Nothing to be done for EventInfo createRep ");
    return StatusCode::SUCCESS;
}

