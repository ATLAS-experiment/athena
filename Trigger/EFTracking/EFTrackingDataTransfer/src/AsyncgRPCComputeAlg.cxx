/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AsyncgRPCComputeAlg.h"

AsyncgRPCComputeAlg::AsyncgRPCComputeAlg(const std::string& name,
                                       ISvcLocator* pSvcLocator)
    : AthAsynchronousAlgorithm(name, pSvcLocator) {}

AsyncgRPCComputeAlg::~AsyncgRPCComputeAlg() {}

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
  OffloadMessage outMsg;
  OffloadMessage inMsg;

  outMsg.mutable_identifier()->assign("do_increment");

  auto* ei = outMsg.mutable_event();
  fillEventInfo(context.eventID(), ei);
  ATH_MSG_DEBUG("Prepared input data, event number "
                << context.eventID().event_number());

  auto gRPCClientContext = std::make_unique<grpc::ClientContext>();
  m_stub->async()->doComputation(
      gRPCClientContext.get(), &outMsg, &inMsg, [this, &outMsg, &inMsg](grpc::Status status) {
        if (status.ok()) {
          ATH_MSG_ALWAYS("Response received for request "
                         << outMsg.identifier() << " id of response "
                         << inMsg.identifier());
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

void AsyncgRPCComputeAlg::encodeMessage(OffloadMessage& outMsg) const {
  // a test message
  outMsg.mutable_float_branches()->at("x").add_values(0.0);
  outMsg.mutable_float_branches()->at("x").add_values(1.0);
  outMsg.mutable_float_branches()->at("y").add_values(2.0);
  outMsg.mutable_float_branches()->at("y").add_values(5.0);
  outMsg.mutable_int_branches()->at("n").add_values(4);
  outMsg.mutable_int_branches()->at("n").add_values(5);
}

void AsyncgRPCComputeAlg::decodeMessage(const OffloadMessage& inMsg) const {
  auto x = inMsg.float_branches().at("x");
  // for ( auto el: inMsg.float_branches()["x"]) {
  //   ATH_MSG_INFO("float x: " << el);
  // }

  // for ( auto el: inMsg.float_branches()["y"]) {
  //   ATH_MSG_INFO("float y: " << el);
  // }

  // for ( auto el: inMsg.int_branches()["y"]) {
  //   ATH_MSG_INFO("int n: " << el);
  // }
}
