/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibStreamCnv/EventInfoCalibRawDataProvider.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "MuCalDecode/CalibEvent.h"
#include "MuonCalibStreamCnvSvc/IMuonCalibStreamDataProviderSvc.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"

#include "GaudiKernel/StatusCode.h"

#include <memory>

// Instantiation of a static factory class used by clients to create
// instances of this service
// static CnvFactory<EventInfoMuonCalibStreamCnv> s_factory;
// const  ICnvFactory& EventInfoMuonCalibStreamCnvFactory = s_factory;
using namespace LVL2_MUON_CALIBRATION;

EventInfoCalibRawDataProvider::EventInfoCalibRawDataProvider(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode EventInfoCalibRawDataProvider::initialize() {
    ATH_MSG_INFO("EventInfoCalibRawDataProvider::initialize");

    // retrieve the dataProviderSvc
    ATH_CHECK(m_dataProvider.retrieve());
    //ATH_CHECK(m_lumiBlockCoolSvc.retrieve());
    // init output EventInfo key
    ATH_CHECK(m_eventInfoKey.initialize());   

    return StatusCode::SUCCESS;
}

// Execute
StatusCode EventInfoCalibRawDataProvider::execute(const EventContext& ctx) const {

    ATH_MSG_INFO("EventInfoCalibRawDataProvider::execute");

    const CalibEvent *event = m_dataProvider->getEvent();

    // Build EventInfo 
    uint64_t eventNumber = event->lvl1_id();
    int runNumber = event->run_number();
    float pt = event->pt();
    // Time Stamp
    uint64_t timeStamp = event->timestamp();
    int lb_nr(0);

    ATH_MSG_DEBUG("New EventInfo made, run/event/timestamp/pt/LB = "<<runNumber<<" "<<eventNumber<<" "<<timeStamp<<" "<<pt<<" "<<lb_nr);

    auto evtInfo = std::make_unique<xAOD::EventInfo>();
    auto evtAuxInfo = std::make_unique<xAOD::EventAuxInfo>();
    evtInfo->setStore(evtAuxInfo.get());
 
    evtInfo->setRunNumber(runNumber);
    evtInfo->setEventNumber(eventNumber);
    evtInfo->setLumiBlock(lb_nr);
    evtInfo->setTimeStamp(timeStamp);
    evtInfo->setBCID(0);
    evtInfo->setTimeStampNSOffset(pt);
    uint32_t eventTypeBitmask = 0;
    eventTypeBitmask |= xAOD::EventInfo::IS_CALIBRATION;
    evtInfo->setEventTypeBitmask( eventTypeBitmask );

    SG::WriteHandle<xAOD::EventInfo> handle{m_eventInfoKey, ctx};
    ATH_CHECK(handle.record(std::move(evtInfo),std::move(evtAuxInfo)));

    ATH_MSG_DEBUG("Created EventInfo " << m_eventInfoKey.key());    
    ATH_MSG_DEBUG("EventInfo core decode processed in MT decode (calibration stream event)");
    
    return StatusCode::SUCCESS;
}
