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
                            std::chrono::seconds(5));

  auto state = channel->GetState(true);

  if (state == GRPC_CHANNEL_READY) {
    ATH_MSG_INFO("gRPC channel connected");
  } else {
    ATH_MSG_ERROR("gRPC channel is not ready after 5 seconds, exiting ...");
    return StatusCode::FAILURE;
  }

  m_stub = std::make_unique<UniversalOffloadService::Stub>(channel);
  OffloadMessage requestMsg;
  OffloadMessage responseMsg;
  requestMsg.set_identifier("Enterprise to Starfleet Command.");
  auto gRPCClientContext = std::make_unique<grpc::ClientContext>();
  auto status = m_stub->doComputation(gRPCClientContext.get(), requestMsg, &responseMsg);
  ATH_MSG_INFO("Service responded with: " << responseMsg.identifier());

  return StatusCode::SUCCESS;
}

StatusCode AsyncgRPCComputeAlg::finalize() {
  return StatusCode::SUCCESS;
}

void fillEventInfo(const EventIDBase& input, ::EventInfo* ei) {
  ei->set_runnumber(input.run_number());
  ei->set_lumiblock(input.lumi_block());
  ei->set_eventnumber(input.event_number());
  ei->set_bcid(input.bunch_crossing_id());
  ei->set_timestamp(input.time_stamp());
  ei->set_timestampnsoffset(input.time_stamp_ns_offset());
}

StatusCode AsyncgRPCComputeAlg::execute(const EventContext& context) const {
  ATH_MSG_ALWAYS("Invoking");

  OffloadMessage requestMsg;
  OffloadMessage responseMsg;

  requestMsg.mutable_identifier()->assign("do_increment");

  auto* ei = requestMsg.mutable_event();
  fillEventInfo(context.eventID(), ei);
  ATH_MSG_DEBUG("Prepared input data, event number "
                << context.eventID().event_number());
  this->restoreAfterSuspend().ignore();
  auto gRPCClientContext = std::make_unique<grpc::ClientContext>();
  m_stub->async()->doComputation(
      gRPCClientContext.get(), &requestMsg, &responseMsg, [this, &requestMsg, &responseMsg](grpc::Status status) {
        if (status.ok()) {
          ATH_MSG_ALWAYS("Response received for request "
                         << requestMsg.identifier() << " id of response "
                         << responseMsg.identifier());
        }
        this->restoreAfterSuspend().ignore();
      });

  ATH_MSG_DEBUG("Computation request is sent");

  return StatusCode::SUCCESS;
}

StatusCode AsyncgRPCComputeAlg::restoreAfterSuspend() const {
  ATH_MSG_ALWAYS("Restored after suspend");
  return StatusCode::SUCCESS;
}

void AsyncgRPCComputeAlg::encodeMessage(OffloadMessage& requestMsg) const {
  // a test message
  requestMsg.mutable_float_branches()->at("x").add_values(0.0);
  requestMsg.mutable_float_branches()->at("x").add_values(1.0);
  requestMsg.mutable_float_branches()->at("y").add_values(2.0);
  requestMsg.mutable_float_branches()->at("y").add_values(5.0);
  requestMsg.mutable_int_branches()->at("n").add_values(4);
  requestMsg.mutable_int_branches()->at("n").add_values(5);
}

void AsyncgRPCComputeAlg::decodeMessage(const OffloadMessage& responseMsg) const {
  auto x = responseMsg.float_branches().at("x");
  // for ( auto el: responseMsg.float_branches()["x"]) {
  //   ATH_MSG_INFO("float x: " << el);
  // }

  // for ( auto el: responseMsg.float_branches()["y"]) {
  //   ATH_MSG_INFO("float y: " << el);
  // }

  // for ( auto el: responseMsg.int_branches()["y"]) {
  //   ATH_MSG_INFO("int n: " << el);
  // }
}
