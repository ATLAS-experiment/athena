/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "BSPackagingTool.h"

#include "ByteStreamData/RawEvent.h"
#include "eformat/index.h"

BSPackagingTool::BSPackagingTool(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent)
    : base_class(type, name, parent) {}

BSPackagingTool::~BSPackagingTool() {}

StatusCode BSPackagingTool::initialize() {
  ATH_CHECK(m_robsSvc.retrieve());
  m_eventsCache = SG::SlotSpecificObj<RawEvent>( SG::getNSlots() );
  return StatusCode::SUCCESS;
}

StatusCode BSPackagingTool::pack(OffloadMessage& msg,
                                 const EventContext& context) const {

  msg.mutable_identifier()->assign("RawEvent");
  m_robsSvc->collectCompleteEventData(context);
  const RawEvent* fullEvent = m_robsSvc->getEvent(context);
  ATH_MSG_DEBUG(" event " << fullEvent->global_id() << " children "
                          << fullEvent->nchildren());

  // pack event header
  auto& data = (*msg.mutable_uint_branches())["header"];
  auto values = data.mutable_values();
  values->Reserve(fullEvent->header_size_word());
  values->Assign(fullEvent->start(), fullEvent->start() + fullEvent->header_size_word());


  const uint32_t nROBs = fullEvent->nchildren();
  for (uint32_t robIdx = 0; robIdx < nROBs; ++robIdx) {
    const uint32_t* robData = fullEvent->child(robIdx);
    eformat::read::ROBFragment rob(robData);
    const auto sourceId =
        eformat::helper::SourceIdentifier(rob.rob_source_id());
    if (isROBToBeSent(sourceId)) {
      ATH_MSG_DEBUG("ROB id will be packed in the messasge" << rob.rob_source_id() << " "
                              << sourceId.human());

      const std::string key = std::to_string(rob.rob_source_id());
      auto& data = (*msg.mutable_uint_branches())[key];
      auto values = data.mutable_values();
      ATH_MSG_DEBUG("ROB size " << rob.rod_fragment_size_word() << " nch of this rob " << rob.nchildren());
      values->Reserve(rob.rod_fragment_size_word());
      values->Assign(robData, robData + rob.payload_size_word());
    }
  }
  ATH_CHECK(unpack(msg, context)); // TODO, remove, this is done to test packing/unpacking sequence
  return StatusCode::SUCCESS;
}

StatusCode BSPackagingTool::unpack(const OffloadMessage& msg, const EventContext& context) const {
  // not a message for me, I only handle RawEvent
  ATH_MSG_DEBUG("Asked to unpack " << msg.identifier() );
  if (msg.identifier() != "RawEvent") {
    ATH_MSG_DEBUG("Nothing for me here, ... ignoring this fragment" );
    return StatusCode::SUCCESS;
  }

  std::vector<uint32_t> temp;
  auto& header = msg.uint_branches().at("header");
  temp.insert(temp.end(), std::begin(header.values()), std::end(header.values()));

  for ( auto& [strROBId, content]:  msg.uint_branches() ) {
    if ( strROBId != "header" ) { // other framents are just ROBs
      ATH_MSG_DEBUG("ROB is unpacked " << strROBId);
      temp.insert(temp.end(), std::begin(content.values()), std::end(content.values()));
    }
  }
  RawEvent* rawEvent = m_eventsCache.get(context);
  rawEvent->assign(temp.data());
  rawEvent->check();

  ATH_MSG_INFO("Constructed raw event of size " << eventData->size() << " size in event header " << eventData->at(1));
  // (*eventData)[1] = eventData->size(); // TODO here one should probably recacluate full event fragment size
  m_robsSvc->setNextEvent(context, rawEvent);
  ATH_MSG_INFO("Event given to RawEvent");
  return StatusCode::SUCCESS;
}


bool BSPackagingTool::isROBToBeSent(
    eformat::helper::SourceIdentifier id) const {
  for (auto& el : m_det)
    if (id.human_group() == el)
      return true;
  return false;
}
