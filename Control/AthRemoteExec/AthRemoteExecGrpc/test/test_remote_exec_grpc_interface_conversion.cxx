// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#include <AthRemoteExecWire/Protocol.h>
#include <athremoteexec.pb.h>
#include <gtest/gtest.h>

#include "../src/ProtocolConvert.h"

namespace AthRemoteExec {

// Test toProto(Status)
TEST(ProtocolConvertTest, StatusToProto) {
  // Test normal statuses
  EXPECT_EQ(athremoteexec::v1::STATUS_OK, toProto(Status::Ok));
  EXPECT_EQ(athremoteexec::v1::STATUS_UNKNOWN_SEQUENCE,
            toProto(Status::UnknownSequence));
  EXPECT_EQ(athremoteexec::v1::STATUS_INVALID_REQUEST,
            toProto(Status::InvalidRequest));
  EXPECT_EQ(athremoteexec::v1::STATUS_ALG_FAILURE, toProto(Status::AlgFailure));
  EXPECT_EQ(athremoteexec::v1::STATUS_TIMEOUT, toProto(Status::Timeout));

  // Test refusal statuses should map to STATUS_UNSPECIFIED
  EXPECT_EQ(athremoteexec::v1::STATUS_UNSPECIFIED, toProto(Status::Overloaded));
  EXPECT_EQ(athremoteexec::v1::STATUS_UNSPECIFIED,
            toProto(Status::ShuttingDown));

  // Test fromProto
  EXPECT_EQ(Status::Ok, fromProto(athremoteexec::v1::STATUS_OK));
  EXPECT_EQ(Status::Unspecified,
            fromProto(athremoteexec::v1::STATUS_UNSPECIFIED));
}

// Test Payload conversion
TEST(ProtocolConvertTest, PayloadConversion) {
  Payload payload;
  payload.key = "testKey";
  payload.encoding = "testEncoding";
  payload.schema = "testSchema";
  payload.data = "testData";

  athremoteexec::v1::Payload protoPayload;
  toProto(payload, protoPayload);

  EXPECT_EQ(payload.key, protoPayload.key());
  EXPECT_EQ(payload.encoding, protoPayload.encoding());
  EXPECT_EQ(payload.schema, protoPayload.schema());
  EXPECT_EQ(payload.data, protoPayload.data());

  // Test reverse conversion
  Payload payload2;
  fromProto(protoPayload, payload2);
  EXPECT_EQ(payload.key, payload2.key);
  EXPECT_EQ(payload.encoding, payload2.encoding);
  EXPECT_EQ(payload.schema, payload2.schema);
  EXPECT_EQ(payload.data, payload2.data);
}

// Test ExecuteRequest conversion
TEST(ProtocolConvertTest, ExecuteRequestConversion) {
  ExecuteRequest request;
  request.sequence = "testSequence";
  request.requestId = 12345;

  // Add inputs
  Payload inputPayload;
  inputPayload.key = "inputKey";
  inputPayload.encoding = "inputEncoding";
  inputPayload.schema = "inputSchema";
  inputPayload.data = "inputData";
  request.inputs.push_back(inputPayload);

  // Set event id
  request.eventId.runNumber = 123;
  request.eventId.eventNumber = 456;
  request.eventId.lumiBlock = 789;
  request.eventId.timeStamp = 1000;
  request.eventId.timeStampNsOffset = 10;
  request.eventId.bunchCrossingId = 20;

  athremoteexec::v1::ExecuteRequest protoRequest;
  toProto(request, protoRequest);

  EXPECT_EQ(request.sequence, protoRequest.sequence());
  EXPECT_EQ(request.requestId, protoRequest.request_id());
  EXPECT_EQ(1, protoRequest.inputs_size());
  EXPECT_TRUE(protoRequest.has_event_id());
  EXPECT_EQ(123, protoRequest.event_id().run_number());
  EXPECT_EQ(456, protoRequest.event_id().event_number());
  EXPECT_EQ(789, protoRequest.event_id().lumi_block());
  EXPECT_EQ(1000, protoRequest.event_id().time_stamp());
  EXPECT_EQ(10, protoRequest.event_id().time_stamp_ns_offset());
  EXPECT_EQ(20, protoRequest.event_id().bunch_crossing_id());

  // Reverse conversion
  ExecuteRequest request2;
  fromProto(protoRequest, request2);

  EXPECT_EQ(request.sequence, request2.sequence);
  EXPECT_EQ(request.requestId, request2.requestId);
  EXPECT_EQ(request.inputs.size(), request2.inputs.size());
  EXPECT_EQ(request.inputs[0].key, request2.inputs[0].key);
  EXPECT_TRUE(request2.eventId.runNumber.has_value());
  EXPECT_EQ(*request2.eventId.runNumber, 123);
  EXPECT_TRUE(request2.eventId.eventNumber.has_value());
  EXPECT_EQ(*request2.eventId.eventNumber, 456);
  EXPECT_TRUE(request2.eventId.lumiBlock.has_value());
  EXPECT_EQ(*request2.eventId.lumiBlock, 789);
  EXPECT_TRUE(request2.eventId.timeStamp.has_value());
  EXPECT_EQ(*request2.eventId.timeStamp, 1000);
  EXPECT_EQ(request2.eventId.timeStampNsOffset, 10);
  EXPECT_EQ(request2.eventId.bunchCrossingId, 20);
}

// Test ExecuteReply conversion
TEST(ProtocolConvertTest, ExecuteReplyConversion) {
  ExecuteReply reply;
  reply.status = Status::AlgFailure;
  reply.diagnostics = "test diagnostics";
  reply.requestId = 54321;

  // Add outputs
  Payload outputPayload;
  outputPayload.key = "outputKey";
  outputPayload.encoding = "outputEncoding";
  outputPayload.schema = "outputSchema";
  outputPayload.data = "outputData";
  reply.outputs.push_back(outputPayload);

  athremoteexec::v1::ExecuteReply protoReply;
  toProto(reply, protoReply);

  EXPECT_EQ(athremoteexec::v1::STATUS_ALG_FAILURE, protoReply.status());
  EXPECT_EQ(reply.diagnostics, protoReply.diagnostics());
  EXPECT_EQ(reply.requestId, protoReply.request_id());
  EXPECT_EQ(1, protoReply.outputs_size());

  // Reverse conversion
  ExecuteReply reply2;
  fromProto(protoReply, reply2);

  EXPECT_EQ(reply.status, reply2.status);
  EXPECT_EQ(reply.diagnostics, reply2.diagnostics);
  EXPECT_EQ(reply.requestId, reply2.requestId);
  EXPECT_EQ(reply.outputs.size(), reply2.outputs.size());
  EXPECT_EQ(reply.outputs[0].key, reply2.outputs[0].key);
}

// Test SequenceInfo conversion
TEST(ProtocolConvertTest, SequenceInfoConversion) {
  SequenceInfo info;
  info.name = "testSequence";

  BoundaryInfo inputBoundary;
  inputBoundary.key = "inputKey";
  inputBoundary.encoding = "inputEncoding";
  inputBoundary.schema = "inputSchema";
  info.inputs.push_back(inputBoundary);

  BoundaryInfo outputBoundary;
  outputBoundary.key = "outputKey";
  outputBoundary.encoding = "outputEncoding";
  outputBoundary.schema = "outputSchema";
  info.outputs.push_back(outputBoundary);

  athremoteexec::v1::SequenceInfo protoInfo;
  toProto(info, protoInfo);

  EXPECT_EQ(info.name, protoInfo.name());
  EXPECT_EQ(1, protoInfo.inputs_size());
  EXPECT_EQ("inputKey", protoInfo.inputs(0).key());
  EXPECT_EQ("inputEncoding", protoInfo.inputs(0).encoding());
  EXPECT_EQ("inputSchema", protoInfo.inputs(0).schema());
  EXPECT_EQ(1, protoInfo.outputs_size());
  EXPECT_EQ("outputKey", protoInfo.outputs(0).key());
  EXPECT_EQ("outputEncoding", protoInfo.outputs(0).encoding());
  EXPECT_EQ("outputSchema", protoInfo.outputs(0).schema());
}

// Test the overload of toProto for ExecuteReply&&
TEST(ProtocolConvertTest, ExecuteReplyMoveConversion) {
  ExecuteReply reply;
  reply.status = Status::Ok;
  reply.diagnostics = "diagnostics";
  reply.requestId = 111;
  reply.outputs.push_back(Payload{"key", "encoding", "schema", "data"});

  athremoteexec::v1::ExecuteReply protoReply;
  toProto(std::move(reply), protoReply);

  EXPECT_EQ(athremoteexec::v1::STATUS_OK, protoReply.status());
  EXPECT_EQ("diagnostics", protoReply.diagnostics());
  EXPECT_EQ(111, protoReply.request_id());
  EXPECT_EQ(1, protoReply.outputs_size());
  EXPECT_EQ("key", protoReply.outputs(0).key());
  EXPECT_EQ("encoding", protoReply.outputs(0).encoding());
  EXPECT_EQ("schema", protoReply.outputs(0).schema());
  EXPECT_EQ("data", protoReply.outputs(0).data());
}
}  // namespace AthRemoteExec

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
