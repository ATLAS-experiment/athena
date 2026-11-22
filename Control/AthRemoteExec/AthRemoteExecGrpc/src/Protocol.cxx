/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <grpcpp/grpcpp.h>

#include "ProtocolConvert.h"

namespace AthRemoteExec {

athremoteexec::v1::Status toProto(Status status) {
  // A refusal has no proto value -> Set status to STATUS_UNSPECIFIED rather
  // number that schema doesn't know.
  if (isRefusal(status)) {
    return athremoteexec::v1::STATUS_UNSPECIFIED;
  }
  return static_cast<athremoteexec::v1::Status>(static_cast<int>(status));
}

Status fromProto(athremoteexec::v1::Status status) {
  return static_cast<Status>(static_cast<int>(status));
}

void toProto(const Payload& payload, athremoteexec::v1::Payload& out) {
  out.set_key(std::move(payload.key));
  out.set_encoding(std::move(payload.encoding));
  out.set_schema(std::move(payload.schema));
  out.set_data(std::move(payload.data));
}

void fromProto(const athremoteexec::v1::Payload& in, Payload& payload) {
  payload.key = std::move(in.key());
  payload.encoding = std::move(in.encoding());
  payload.schema = std::move(in.schema());
  payload.data = std::move(in.data());
}

void toProto(const ExecuteRequest& request,
             athremoteexec::v1::ExecuteRequest& out) {
  out.set_sequence(request.sequence);
  out.set_request_id(request.requestId);
  for (const Payload& payload : request.inputs) {
    toProto(payload, *out.add_inputs());
  }

  // Only the fields that were actually set are written, so an incomplete
  // identity stays incomplete on the wire and the server can say which parts
  // are missing rather than "something was wrong".
  athremoteexec::v1::EventId& id = *out.mutable_event_id();
  const EventId& from = request.eventId;
  if (from.runNumber) {
    id.set_run_number(*from.runNumber);
  }
  if (from.eventNumber) {
    id.set_event_number(*from.eventNumber);
  }
  if (from.lumiBlock) {
    id.set_lumi_block(*from.lumiBlock);
  }
  if (from.timeStamp) {
    id.set_time_stamp(*from.timeStamp);
  }
  id.set_time_stamp_ns_offset(from.timeStampNsOffset);
  id.set_bunch_crossing_id(from.bunchCrossingId);
}

void fromProto(const athremoteexec::v1::ExecuteRequest& in,
               ExecuteRequest& request) {
  request.sequence = in.sequence();
  request.requestId = in.request_id();

  request.eventId = EventId{};
  if (in.has_event_id()) {
    const athremoteexec::v1::EventId& id = in.event_id();
    EventId& to = request.eventId;
    if (id.has_run_number()) {
      to.runNumber = id.run_number();
    }
    if (id.has_event_number()) {
      to.eventNumber = id.event_number();
    }
    if (id.has_lumi_block()) {
      to.lumiBlock = id.lumi_block();
    }
    if (id.has_time_stamp()) {
      to.timeStamp = id.time_stamp();
    }
    to.timeStampNsOffset = id.time_stamp_ns_offset();
    to.bunchCrossingId = id.bunch_crossing_id();
  }
  request.inputs.clear();
  request.inputs.reserve(in.inputs_size());
  for (const athremoteexec::v1::Payload& payload : in.inputs()) {
    Payload converted;
    fromProto(payload, converted);
    request.inputs.push_back(std::move(converted));
  }
}

void toProto(const ExecuteReply& reply, athremoteexec::v1::ExecuteReply& out) {
  out.set_status(toProto(reply.status));
  out.set_diagnostics(reply.diagnostics);
  out.set_request_id(reply.requestId);
  for (const Payload& payload : reply.outputs) {
    toProto(payload, *out.add_outputs());
  }
}

void toProto(ExecuteReply&& reply, athremoteexec::v1::ExecuteReply& out) {
  out.set_status(toProto(reply.status));
  out.set_diagnostics(std::move(reply.diagnostics));
  out.set_request_id(reply.requestId);
  for (Payload& payload : reply.outputs) {
    athremoteexec::v1::Payload& target = *out.add_outputs();
    target.set_key(std::move(payload.key));
    target.set_encoding(std::move(payload.encoding));
    target.set_schema(std::move(payload.schema));
    target.set_data(std::move(payload.data));
  }
}

void fromProto(const athremoteexec::v1::ExecuteReply& in, ExecuteReply& reply) {
  reply.status = fromProto(in.status());
  reply.diagnostics = in.diagnostics();
  reply.requestId = in.request_id();
  reply.outputs.clear();
  reply.outputs.reserve(in.outputs_size());
  for (const athremoteexec::v1::Payload& payload : in.outputs()) {
    Payload converted;
    fromProto(payload, converted);
    reply.outputs.push_back(std::move(converted));
  }
}

void toProto(const SequenceInfo& info, athremoteexec::v1::SequenceInfo& out) {
  out.set_name(info.name);
  const auto fill = [](const BoundaryInfo& boundary,
                       athremoteexec::v1::BoundaryInfo& target) {
    target.set_key(boundary.key);
    target.set_encoding(boundary.encoding);
    target.set_schema(boundary.schema);
  };
  for (const BoundaryInfo& boundary : info.inputs) {
    fill(boundary, *out.add_inputs());
  }
  for (const BoundaryInfo& boundary : info.outputs) {
    fill(boundary, *out.add_outputs());
  }
}

bool encode(const ExecuteRequest& request, std::string& wire) {
  athremoteexec::v1::ExecuteRequest message;
  toProto(request, message);
  return message.SerializeToString(&wire);
}

bool decode(std::string_view wire, ExecuteRequest& request) {
  athremoteexec::v1::ExecuteRequest message;
  if (!message.ParseFromArray(wire.data(), static_cast<int>(wire.size()))) {
    return false;
  }
  fromProto(message, request);
  return true;
}

bool encode(const ExecuteReply& reply, std::string& wire) {
  athremoteexec::v1::ExecuteReply message;
  toProto(reply, message);
  return message.SerializeToString(&wire);
}

bool decode(std::string_view wire, ExecuteReply& reply) {
  athremoteexec::v1::ExecuteReply message;
  if (!message.ParseFromArray(wire.data(), static_cast<int>(wire.size()))) {
    return false;
  }
  fromProto(message, reply);
  return true;
}

std::string rpcFrameworkVersion() {
  return "gRPC " + grpc::Version();
}

}  // namespace AthRemoteExec
