/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "EventInfoPackagingTool.h"

#include <xAODEventInfo/EventAuxInfo.h>
#include <xAODEventInfo/EventInfo.h>

#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "EventInfo/EventType.h"

#include "GaudiKernel/EventIDBase.h"

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

  return StatusCode::SUCCESS;
}

StatusCode EventInfoPackagingTool::unpack(const OffloadMessage& msg,
                                          EventContext& context) const {
  const ::EventInfoMessage& mei = msg.event();

  EventIDBase eventId(mei.runnumber(), mei.eventnumber(), mei.timestamp(),
                      mei.timestampnsoffset(), mei.bcid(), mei.lumiblock());

  context.setEventID(eventId);
  context.setValid(true);
  const EventIDBase& restoredEventId = context.eventID();
  {
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
    // produce as well the legacy EventInfo, should not be used anymore but it still is
    auto eid = std::make_unique<EventID>(
        context.eventID().run_number(), context.eventID().event_number(),
        context.eventID().time_stamp(),
        context.eventID().time_stamp_ns_offset(),
        context.eventID().lumi_block(), context.eventID().bunch_crossing_id());

    auto ei = std::make_unique<EventInfo>(std::move(eid),
                                          std::make_unique<EventType>());
    ATH_CHECK(evtStore()->record(std::move(ei), "EventInfo"));
  }
  return StatusCode::SUCCESS;
}
