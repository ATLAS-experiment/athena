/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "EventInfoPackagingTool.h"

#include <xAODEventInfo/EventAuxInfo.h>
#include <xAODEventInfo/EventInfo.h>

#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "EventInfo/EventID.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventType.h"
#include "GaudiKernel/EventIDBase.h"
#include "PersistentDataModel/DataHeader.h"

EventInfoPackagingTool::EventInfoPackagingTool(const std::string& type,
                                               const std::string& name,
                                               const IInterface* parent)
    : base_class(type, name, parent) {}

EventInfoPackagingTool::~EventInfoPackagingTool() {}

StatusCode EventInfoPackagingTool::pack(OffloadMessage& msg,
                                        const EventContext& context) const {
  ::EventInfoMessage* ei = msg.mutable_event();
  ei->set_runnumber(context.eventID().run_number());
  ei->set_eventnumber(context.eventID().event_number());
  ei->set_lumiblock(context.eventID().lumi_block());
  ei->set_timestamp(context.eventID().time_stamp());
  ei->set_timestampnsoffset(context.eventID().time_stamp_ns_offset());
  ei->set_bcid(context.eventID().bunch_crossing_id());

  const DataHeader* dataHeader = nullptr;
  ATH_CHECK(evtStore()->retrieve(dataHeader, "ByteStreamDataHeader"));

  ei->mutable_meta()->set_key("StreamRAW");
  ei->mutable_meta()->set_file_guid(
      dataHeader->elements().at(0).getToken()->dbID().toString());
  ei->mutable_meta()->set_event_offset(
      dataHeader->elements().at(0).getToken()->oid().second);

  return StatusCode::SUCCESS;
}

StatusCode EventInfoPackagingTool::unpack(const OffloadMessage& msg,
                                          EventContext& context) const {
  ATH_MSG_INFO("Asked to unpack event information...");
  const ::EventInfoMessage& mei = msg.event();

  EventIDBase eventId(mei.runnumber(), mei.eventnumber(), mei.timestamp(),
                      mei.timestampnsoffset(), mei.bcid(), mei.lumiblock());

  ATH_MSG_INFO("Run = " << mei.runnumber() << " evt = " << mei.eventnumber());
  context.setEventID(eventId);
  context.setValid(true);
  // const EventIDBase& restoredEventId = context.eventID();

  {
    ATH_MSG_INFO("Creating xAOD::EventInfo...");
    auto outputEvent = std::make_unique<xAOD::EventInfo>();
    auto outputEventAux = std::make_unique<xAOD::EventAuxInfo>();
    outputEvent->setStore(outputEventAux.get());
    outputEvent->setRunNumber(context.eventID().run_number());
    outputEvent->setEventNumber(context.eventID().event_number());
    outputEvent->setTimeStamp(context.eventID().time_stamp());
    outputEvent->setTimeStampNSOffset(context.eventID().time_stamp_ns_offset());
    outputEvent->setBCID(context.eventID().bunch_crossing_id());
    outputEvent->setLumiBlock(context.eventID().lumi_block());

    ATH_CHECK(evtStore()->record(std::move(outputEvent), "EventInfo"));
    ATH_CHECK(evtStore()->record(std::move(outputEventAux), "EventInfoAux."));
  }

  {
    ATH_MSG_INFO("Creating legacy EventInfo...");
    // produce as well the legacy EventInfo, should not be used anymore but it
    // still is
    auto eid = std::make_unique<EventID>(
        context.eventID().run_number(), context.eventID().event_number(),
        context.eventID().time_stamp(),
        context.eventID().time_stamp_ns_offset(),
        context.eventID().lumi_block(), context.eventID().bunch_crossing_id());

    auto ei = std::make_unique<EventInfo>(std::move(eid),
                                          std::make_unique<EventType>());
    ATH_CHECK(evtStore()->record(std::move(ei), "EventInfo"));
  }

  {
    ATH_MSG_INFO("Creating DataHeader...");
    // This is exactly what ByteStreamEventStorageInputSvc::generateDataHeader
    // is doing, but we use correct file GUID and event offsets
    auto makeBSProvenance = [&mei]() -> std::unique_ptr<DataHeaderElement> {
      Token token;
      token.setDb(mei.meta().key());
      token.setTechnology(0x00001000);
      token.setOid(Token::OID_t(0LL, mei.meta().event_offset()));

      return std::make_unique<DataHeaderElement>(
          ClassID_traits<DataHeader>::ID(), mei.meta().key(), std::move(token));
    };

    // Created data header element with BS provenance information
    std::unique_ptr<DataHeaderElement> dataHeaderElement = makeBSProvenance();
    // Create data header itself
    std::unique_ptr<DataHeader> dataHeader = std::make_unique<DataHeader>();
    // Declare header primary
    dataHeader->setStatus(DataHeader::Input);
    // Set processTag
    dataHeader->setProcessTag(dataHeaderElement->getKey());
    // add the data header element self reference to the object vector
    dataHeader->insert(*std::move(dataHeaderElement));

    // Now add ref to xAOD::EventInfo
    auto bsaddr = std::make_unique<ByteStreamAddress>(
        ClassID_traits<xAOD::EventInfo>::ID(), "EventInfo", "");
    bsaddr->setEventContext(context);

    ATH_CHECK(evtStore()->recordAddress("EventInfo", std::move(bsaddr)));
    const SG::DataProxy* ptmpx = evtStore()->transientProxy(
        ClassID_traits<xAOD::EventInfo>::ID(), "EventInfo");
    if (ptmpx != nullptr) {
      DataHeaderElement element(ptmpx, 0, "EventInfo");
      dataHeader->insert(element);
    }

    // Now add ref to xAOD::EventAuxInfo
    bsaddr = std::make_unique<ByteStreamAddress>(
        ClassID_traits<xAOD::EventAuxInfo>::ID(), "EventInfoAux.", "");
    bsaddr->setEventContext(context);

    ATH_CHECK(evtStore()->recordAddress("EventInfoAux.", std::move(bsaddr)));
    const SG::DataProxy* ptmpaux = evtStore()->transientProxy(
        ClassID_traits<xAOD::EventAuxInfo>::ID(), "EventInfoAux.");
    if (ptmpaux != 0) {
      DataHeaderElement element(ptmpaux, 0, "EventInfoAux.");
      dataHeader->insert(element);
    }

    // Record new data header.Boolean flags will allow it's deletion in case
    // of skipped events.
    ATH_CHECK(evtStore()->record<DataHeader>(
        dataHeader.release(), "ByteStreamDataHeader", true, false, true));

    ATH_MSG_INFO("SG after EventInfoPackagingTool::unpack: " << (void*)evtStore().get() << "\n" << evtStore()->dump());
  }

  return StatusCode::SUCCESS;
}
