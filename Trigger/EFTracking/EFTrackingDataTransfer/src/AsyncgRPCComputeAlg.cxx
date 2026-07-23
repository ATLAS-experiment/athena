/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AsyncgRPCComputeAlg.h"

StatusCode AsyncgRPCComputeAlg::initialize() {

  ATH_MSG_DEBUG("Setting up gRPC channel");
  // this should realy be a service
  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());

  channel->WaitForConnected(std::chrono::system_clock::now() +
                            std::chrono::seconds(100));

  auto state = channel->GetState(true);
  if (state == GRPC_CHANNEL_READY) {
    ATH_MSG_INFO("gRPC channel connected");
  } else {
    ATH_MSG_ERROR("gRPC channel is not ready after 5 seconds, exiting ...");
    return StatusCode::FAILURE;
  }
  m_stub = std::make_unique<UniversalOffloadService::Stub>(channel);

  ATH_CHECK(m_packingTools.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode AsyncgRPCComputeAlg::finalize() {
  return StatusCode::SUCCESS;
}

void fillEventInfo(const EventIDBase& input, ::EventInfoMessage* ei) {
  ei->set_runnumber(input.run_number());
  ei->set_eventnumber(input.event_number());
  ei->set_lumiblock(input.lumi_block());
  ei->set_timestamp(input.time_stamp());
  ei->set_timestampnsoffset(input.time_stamp_ns_offset());
  ei->set_bcid(input.bunch_crossing_id());
}

StatusCode AsyncgRPCComputeAlg::execute(const EventContext& context) const {
  ATH_MSG_ALWAYS("Invoking");

  // OffloadMessage requestMsg;
  google::protobuf::Arena arena;
  OffloadMessage* requestMsg =
      google::protobuf::Arena::Create<OffloadMessage>(&arena);
  for ( auto& tool : m_packingTools)
    ATH_CHECK(tool->pack(*requestMsg, context));

  // OffloadMessage responseMsg;
  OffloadMessage* responseMsg =
      google::protobuf::Arena::Create<OffloadMessage>(&arena);

  ATH_MSG_DEBUG("Prepared input data, event number "
                << context.eventID().event_number());

  auto gRPCClientContext = std::make_unique<grpc::ClientContext>();
  auto status =
      m_stub->doComputation(gRPCClientContext.get(), *requestMsg, responseMsg);
  ATH_MSG_INFO("Service responded with: " << responseMsg->identifier());



  // using Promise_t = boost::fibers::promise<OffloadMessage*>;
  // using Future_t = boost::fibers::future<OffloadMessage*>;
  // Promise_t promise{};
  // Future_t future = promise.get_future();

  // auto callback = [this, &promise, responseMsg](grpc::Status status) {
  //   if (status.ok()) {
  //     // ATH_MSG_ALWAYS("OK Response received for request "
  //     //                << responseMsg->identifier());
  //     promise.set_value(responseMsg);
  //   } else {
  //     // responseMsg->set_identifier("failed");
  //   }
  // };
  // m_stub->async()->doComputation(gRPCClientContext.get(), requestMsg,
  //                                responseMsg, callback);
  // ATH_MSG_DEBUG("Computation request is sent");
  // future.get(); // this is waiting
  // ATH_CHECK(restoreAfterSuspend());

  return StatusCode::SUCCESS;
}

StatusCode AsyncgRPCComputeAlg::restoreAfterSuspend() const {
  ATH_MSG_ALWAYS("Restored after suspend");
  return StatusCode::SUCCESS;
}