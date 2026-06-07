/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TestSender.h"

#include <GaudiKernel/StatusCode.h>

#include "GaudiKernel/EventIDBase.h"
#include "TestMessages.pb.h"

TestSender::TestSender(const std::string& name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

TestSender::~TestSender() {}

StatusCode TestSender::initialize() {
  ATH_MSG_DEBUG("Setting up gRPC channel");
  // this should realy be a service
  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());

  channel->WaitForConnected(std::chrono::system_clock::now() +
                            std::chrono::seconds(5));

  auto state = channel->GetState(true);

  if (state == GRPC_CHANNEL_READY) {
    ATH_MSG_INFO("gRPC channel connected");
  } else {
    ATH_MSG_ERROR("gRPC channel is not ready after 5 seconds, exiting ...");
    return StatusCode::FAILURE;
  }

  m_stub = std::make_unique<TestService::Stub>(channel);

  ATH_CHECK(m_outputKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode TestSender::finalize() {
  return StatusCode::SUCCESS;
}

void fillEventInfo(const EventIDBase& input, ::TestEventInfo* ei) {
  ei->set_runnumber(input.run_number());
  ei->set_lumiblock(input.lumi_block());
  ei->set_eventnumber(input.event_number());
  ei->set_bcid(input.bunch_crossing_id());
  ei->set_timestamp(input.time_stamp());
  ei->set_timestampnsoffset(input.time_stamp_ns_offset());
}

StatusCode TestSender::execute(const EventContext& context) const {
  // build the context
  auto ot = std::make_unique<OffloadToken>();
  auto* ei = ot->request()->mutable_eventinfo();
  fillEventInfo(context.eventID(), ei);
  // build message
  auto* data = ot->request()->mutable_data();
  for (int32_t i = 0; i < m_sizeToSend; ++i)
    data->Add(m_valueToSend.value());

  ot->request()->mutable_id()->assign(name());

  ATH_MSG_DEBUG("Prepared input data, event number " << context.eventID().event_number());
  ot->sendRequest(m_stub.get());
  ATH_MSG_DEBUG("Computation request is sent");

  auto handle = SG::makeHandle(m_outputKey, context);
  ATH_CHECK(handle.record(std::move(ot)));
  ATH_MSG_DEBUG("Token in StoreGate");

  return StatusCode::SUCCESS;
}
