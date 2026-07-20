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
  return StatusCode::SUCCESS;
}

StatusCode BSPackagingTool::pack(OffloadMessage& msg,
                                 const EventContext& context) const {

  m_robsSvc->collectCompleteEventData(context);
  const RawEvent* fullEvent = m_robsSvc->getEvent(context);
  ATH_MSG_DEBUG(" event " << fullEvent->global_id() << " children "
                          << fullEvent->nchildren());

  // pack event header
  auto& data = (*msg.mutable_uint_branches())["header"];
  auto values = data.mutable_values();
  values->Reserve(fullEvent->header_size_word());
  values->Assign(fullEvent->payload(), fullEvent->payload() + fullEvent->header_size_word());


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
  return StatusCode::SUCCESS;
}

StatusCode BSPackagingTool::unpack(const OffloadMessage& msg, const EventContext& context) const {
  // I do not know yet how to place the data in the event
  for ( auto [strROBId, contant]:  msg.uint_branches() ) {

  }
  return StatusCode::SUCCESS;
}


bool BSPackagingTool::isROBToBeSent(
    eformat::helper::SourceIdentifier id) const {
  for (auto& el : m_det)
    if (id.human_group() == el)
      return true;
  return false;
}
