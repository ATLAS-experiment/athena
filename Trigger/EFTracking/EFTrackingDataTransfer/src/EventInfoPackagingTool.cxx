/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "EventInfoPackagingTool.h"

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
  const ::EventInfoMessage& ei = msg.event();

  EventIDBase eventId(ei.runnumber(),
                      ei.eventnumber(),
                      ei.timestamp(),
                      ei.timestampnsoffset(),
                      ei.bcid(),
                      ei.lumiblock());

  context.setEventID(eventId);
  context.setValid(true);
  const EventIDBase& restoredEventId = context.eventID();
  ATH_MSG_INFO("Restored EventID: run=" << restoredEventId.run_number()
               << ", event=" << restoredEventId.event_number()
               << ", lumi=" << restoredEventId.lumi_block()
               << ", timestamp=" << restoredEventId.time_stamp()
               << ", timestampNsOffset=" << restoredEventId.time_stamp_ns_offset()
               << ", bcid=" << restoredEventId.bunch_crossing_id());
  return StatusCode::SUCCESS;
}
