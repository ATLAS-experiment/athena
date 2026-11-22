/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file remote_exec_menu_client.cxx
 * @brief Excersise server with various types of valid and invalid requests
 *
 * Usage: remote_exec_menu_client <port-file|host:port> [delay-ms]
 *
 * This client is bulid on plain gRPC code, that is shared with a server, to
 * have a finer control on the possible error paths that need to be excercised.
 */

#include <CxxUtils/StringUtils.h>
#include <grpcpp/grpcpp.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "athremoteexec.grpc.pb.h"

namespace {

static constexpr std::string kInts = "test_ints";
static constexpr std::string kFloats = "test_floats";

void setEventId(athremoteexec::v1::ExecuteRequest& request,
                uint64_t eventNumber, uint32_t runNumber = 1) {
  athremoteexec::v1::EventId* id = request.mutable_event_id();
  id->set_run_number(runNumber);
  id->set_event_number(eventNumber);
  id->set_lumi_block(1);
  id->set_time_stamp(
      1735689600);  // Wednesday, 1 January 2025 at 01:00:00 UTC+1
}

std::atomic<int> s_failures{0};

void check(bool condition, const std::string& what) {
  if (!condition) {
    std::printf("FAIL: %s\n", what.c_str());
    ++s_failures;
  } else {
    std::printf("ok: %s\n", what.c_str());
  }
}

/// Wait for the server to publish its port, then build the target string.
std::string resolveTarget(const std::string& argument, double timeoutSeconds) {
  if (argument.find(':') != std::string::npos) {
    return argument;
  }
  const auto deadline =
      std::chrono::steady_clock::now() +
      std::chrono::milliseconds(static_cast<long>(timeoutSeconds * 1000));
  while (std::chrono::steady_clock::now() < deadline) {
    std::ifstream in(argument);
    int port = 0;
    if (in >> port && port > 0) {
      return "localhost:" + std::to_string(port);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  return {};
}

/// Build payload
athremoteexec::v1::Payload* addPayload(
    athremoteexec::v1::ExecuteRequest& request, const std::string& key,
    const std::string& schema, const std::string& bytes) {
  athremoteexec::v1::Payload* payload = request.add_inputs();
  payload->set_key(key);
  payload->set_encoding("protobuf");
  payload->set_schema(schema);
  payload->set_data(bytes);
  return payload;
}

/// Parse ints into bytes that test_IntsCodec expects
void addInts(athremoteexec::v1::ExecuteRequest& request, const std::string& key,
             const std::vector<std::pair<std::string, int64_t>>& values) {
  std::string data;
  for (const auto& [name, value] : values) {
    data += std::format("{}:{}#", name, value);
  }

  if (values.size() > 0) {
    data.pop_back();
  }

  addPayload(request, key, kInts, data);
}

/// Parse ints into bytes into something that test_IntsCodec doesn't expect
void addFloats(athremoteexec::v1::ExecuteRequest& request,
               const std::string& key, const std::string& /*name*/,
               const std::vector<float>& values) {
  std::string data;
  for (float value : values) {
    data += std::format("{}#", value);
  }

  if (values.size() > 0) {
    data.pop_back();
  }

  addPayload(request, key, kFloats, data);
}

/// Parse bytes into given named integer
std::optional<int64_t> intOf(const athremoteexec::v1::ExecuteReply& reply,
                             const std::string& key, const std::string& name) {
  for (const athremoteexec::v1::Payload& output : reply.outputs()) {
    if (output.key() != key || output.schema() != kInts) {
      continue;
    }

    const std::vector<std::string> tokens =
        CxxUtils::tokenize(output.data(), '#');
    std::vector<std::pair<std::string, int32_t>> pairs;
    std::transform(
        tokens.begin(), tokens.end(), std::back_inserter(pairs),
        [](std::string_view token) -> std::pair<std::string, int32_t> {
          const std::vector<std::string> split = CxxUtils::tokenize(token, ':');
          return {split[0], CxxUtils::atoi(split[1])};
        });

    const auto found =
        std::find_if(pairs.begin(), pairs.end(),
                     [name](const std::pair<std::string, int32_t>& candidate) {
                       return candidate.first == name;
                     });
    if (found != pairs.end()) {
      return found->second;
    }
  }

  return std::nullopt;
}

athremoteexec::v1::ExecuteRequest sumRequest(int64_t a, int64_t b,
                                             uint64_t id) {
  athremoteexec::v1::ExecuteRequest request;
  request.set_sequence("RemoteExecSeqSum");
  request.set_request_id(id);
  setEventId(request, id);
  addInts(request, "addends", {{"a", a}, {"b", b}});
  return request;
}

athremoteexec::v1::ExecuteRequest chainRequest(int64_t p, int64_t q, int64_t r,
                                               uint64_t id) {
  athremoteexec::v1::ExecuteRequest request;
  request.set_sequence("RemoteExecSeqChain");
  request.set_request_id(id);
  setEventId(request, id);
  addInts(request, "terms", {{"p", p}, {"q", q}, {"r", r}});
  return request;
}

/// One blocking Execute call. Transport failures come back as an unset status.
athremoteexec::v1::ExecuteReply call(
    athremoteexec::v1::SequenceExecutor::Stub& stub,
    const athremoteexec::v1::ExecuteRequest& request) {
  grpc::ClientContext context;
  athremoteexec::v1::ExecuteReply reply;
  const grpc::Status status = stub.Execute(&context, request, &reply);
  if (!status.ok()) {
    std::printf("transport error on request %llu: %s\n",
                static_cast<unsigned long long>(request.request_id()),
                status.error_message().c_str());
  }
  return reply;
}

bool waitUntilReady(athremoteexec::v1::SequenceExecutor::Stub& stub,
                    double timeoutSeconds) {
  const auto deadline =
      std::chrono::steady_clock::now() +
      std::chrono::milliseconds(static_cast<long>(timeoutSeconds * 1000));
  while (std::chrono::steady_clock::now() < deadline) {
    grpc::ClientContext context;
    athremoteexec::v1::ListSequencesRequest request;
    athremoteexec::v1::ListSequencesReply reply;
    if (stub.ListSequences(&context, request, &reply).ok()) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  return false;
}

}  // namespace

int main(int argc, const char* argv[]) {
  if (argc < 2 || argc > 3) {
    std::printf("usage: %s <port-file|host:port> [delay-ms]\n", argv[0]);
    return EXIT_FAILURE;
  }
  const long delayMs = argc == 3 ? std::strtol(argv[2], nullptr, 10) : 0;

  const std::string target = resolveTarget(argv[1], 120.0);
  if (target.empty()) {
    std::printf("FAIL: server never published a port\n");
    return EXIT_FAILURE;
  }
  std::printf("connecting to %s\n", target.c_str());

  grpc::ChannelArguments channelArgs;
  channelArgs.SetMaxReceiveMessageSize(-1);
  channelArgs.SetMaxSendMessageSize(-1);
  const std::shared_ptr<grpc::Channel> channel = grpc::CreateCustomChannel(
      target, grpc::InsecureChannelCredentials(), channelArgs);
  const std::unique_ptr<athremoteexec::v1::SequenceExecutor::Stub> stub =
      athremoteexec::v1::SequenceExecutor::NewStub(channel);

  if (!waitUntilReady(*stub, 120.0)) {
    std::printf("FAIL: server did not become ready\n");
    return EXIT_FAILURE;
  }

  // The menu is discoverable with no out-of-band knowledge beyond the schema.
  {
    grpc::ClientContext context;
    athremoteexec::v1::ListSequencesRequest request;
    athremoteexec::v1::ListSequencesReply reply;
    check(stub->ListSequences(&context, request, &reply).ok(),
          "ListSequences succeeds");

    std::set<std::string> offered;
    for (const athremoteexec::v1::SequenceInfo& info : reply.sequences()) {
      offered.insert(info.name());
      if (info.name() == "RemoteExecSeqChain") {
        check(info.inputs_size() == 1 && info.inputs(0).key() == "terms" &&
                  info.inputs(0).encoding() == "protobuf" &&
                  info.inputs(0).schema() == kInts,
              "RemoteExecSeqChain advertises its input, with encoding and "
              "schema");
        check(athremoteexec::v1::BoundaryInfo::descriptor()->field_count() == 3,
              "RemoteExecSeqChain advertises the wire contract");
        check(info.outputs_size() == 1 &&
                  info.outputs(0).key() == "chainTotal" &&
                  info.outputs(0).schema() == kInts,
              "RemoteExecSeqChain advertises its output");
      }
    }
    for (const char* expected :
         {"RemoteExecSeqSum", "RemoteExecSeqChain", "RemoteExecSeqFail"}) {
      check(offered.count(expected) == 1,
            std::format("The server offers {}", expected));
    }
  }

  // Mixed requests, fired at once, run concurrently.
  {
    static constexpr int nChain = 4;
    static constexpr int nSum = 4;
    std::vector<athremoteexec::v1::ExecuteReply> replies(nChain + nSum);
    std::vector<std::thread> threads;
    threads.reserve(replies.size());

    const auto started = std::chrono::steady_clock::now();
    for (int i = 0; i < nChain; ++i) {
      threads.emplace_back([&stub, &replies, i] {
        replies[i] = call(
            *stub, chainRequest(i, 10, 100, static_cast<uint64_t>(100 + i)));
      });
    }
    for (int i = 0; i < nSum; ++i) {
      threads.emplace_back([&stub, &replies, i] {
        replies[nChain + i] =
            call(*stub, sumRequest(i, 1000, static_cast<uint64_t>(200 + i)));
      });
    }
    for (std::thread& thread : threads) {
      thread.join();
    }
    const double elapsedMs = std::chrono::duration<double, std::milli>(
                                 std::chrono::steady_clock::now() - started)
                                 .count();
    std::printf(
        "%d concurrent requests took %.0f ms (serial lower bound "
        "would be %ld ms)\n",
        nChain + nSum, elapsedMs, nChain * delayMs);

    bool allOk = true;
    for (int i = 0; i < nChain; ++i) {
      const std::optional<int64_t> chain =
          intOf(replies[i], "chainTotal", "chain");
      allOk &= replies[i].status() == athremoteexec::v1::STATUS_OK &&
               chain.has_value() && *chain == i + 110 &&
               replies[i].request_id() == static_cast<uint64_t>(100 + i);
    }
    for (int i = 0; i < nSum; ++i) {
      const std::optional<int64_t> sum =
          intOf(replies[nChain + i], "total", "sum");
      allOk &=
          replies[nChain + i].status() == athremoteexec::v1::STATUS_OK &&
          sum.has_value() && *sum == i + 1000 &&
          replies[nChain + i].request_id() == static_cast<uint64_t>(200 + i);
    }
    check(allOk, "Every concurrent request got its own correct answer");

    if (delayMs > 0) {
      check(elapsedMs < 0.8 * nChain * delayMs,
            "Concurrent requests overlapped");
    }
  }

  {
    // Requesting non-existent sequence
    athremoteexec::v1::ExecuteRequest request;
    request.set_sequence("NoSuchSequence");
    request.set_request_id(300);
    setEventId(request, 300);
    const athremoteexec::v1::ExecuteReply reply = call(*stub, request);
    check(reply.status() == athremoteexec::v1::STATUS_UNKNOWN_SEQUENCE,
          "Unknown sequence is rejected");
    check(reply.diagnostics() == "no sequence named 'NoSuchSequence'",
          "The rejection message contains the reason");
  }

  {
    // Sending no payload
    athremoteexec::v1::ExecuteRequest request;
    request.set_sequence("RemoteExecSeqSum");
    request.set_request_id(301);
    setEventId(request, 301);  // no payload at all
    const athremoteexec::v1::ExecuteReply reply = call(*stub, request);
    check(reply.status() == athremoteexec::v1::STATUS_INVALID_REQUEST,
          "Message with no payload is rejected");
  }

  {
    // Sending wrong payload
    athremoteexec::v1::ExecuteRequest request;
    request.set_sequence("RemoteExecSeqSum");
    request.set_request_id(302);
    setEventId(request, 302);
    addFloats(request, "addends", "addends", {1.0f, 2.0f});
    const athremoteexec::v1::ExecuteReply reply = call(*stub, request);
    check(reply.status() == athremoteexec::v1::STATUS_INVALID_REQUEST,
          "Payload with wrong schema is rejected");
    check(reply.diagnostics().find(kInts) != std::string::npos,
          "The rejection message contains the reason");
  }

  {
    // Sending wrong key
    athremoteexec::v1::ExecuteRequest request = sumRequest(1, 2, 312);
    addInts(request, "notAnInput", {{"a", 1}});
    const athremoteexec::v1::ExecuteReply reply = call(*stub, request);
    check(reply.status() == athremoteexec::v1::STATUS_INVALID_REQUEST,
          "Payload with wrong key is rejected");
  }

  {
    // Sending duplicate key
    athremoteexec::v1::ExecuteRequest request = sumRequest(1, 2, 313);
    addInts(request, "addends", {{"a", 9}, {"a", 9}});
    const athremoteexec::v1::ExecuteReply reply = call(*stub, request);
    check(reply.status() == athremoteexec::v1::STATUS_INVALID_REQUEST,
          "Payload with duplicate keys is rejected");
  }

  {
    // Sending corrupted message
    athremoteexec::v1::ExecuteRequest request;
    request.set_sequence("RemoteExecSeqSum");
    request.set_request_id(314);
    setEventId(request, 314);
    addPayload(request, "addends", kInts, std::string(16, '\xff'));
    const athremoteexec::v1::ExecuteReply reply = call(*stub, request);
    check(reply.status() == athremoteexec::v1::STATUS_ALG_FAILURE,
          "Decoding algorithm fails on corrupted message");
    check(reply.diagnostics().find("RemoteExecSeqSumInputCnvAlg") !=
              std::string::npos,
          "The rejection message contains algorithm name that failed");
  }

  {
    // Calling failing sequence
    athremoteexec::v1::ExecuteRequest request;
    request.set_sequence("RemoteExecSeqFail");
    request.set_request_id(303);
    setEventId(request, 303);
    const athremoteexec::v1::ExecuteReply reply = call(*stub, request);
    check(reply.status() == athremoteexec::v1::STATUS_ALG_FAILURE,
          "Paylaod algorithm fails");
    check(reply.diagnostics().find("RemoteExecSeqFailAlg") != std::string::npos,
          "The rejection message contains algorithm name that failed");
  }

  {
    // Sending request without event information
    athremoteexec::v1::ExecuteRequest request;
    request.set_sequence("RemoteExecSeqSum");
    request.set_request_id(306);
    addInts(request, "addends", {{"a", 1}, {"b", 2}});
    const athremoteexec::v1::ExecuteReply reply = call(*stub, request);
    check(reply.status() == athremoteexec::v1::STATUS_INVALID_REQUEST,
          "Message with no event information is rejected");
    check(reply.diagnostics().find("run_number") != std::string::npos &&
              reply.diagnostics().find("event_number") != std::string::npos,
          "The rejection message contians what's missing");
  }

  {
    // Sending request with partial event information
    athremoteexec::v1::ExecuteRequest request;
    request.set_sequence("RemoteExecSeqSum");
    request.set_request_id(307);
    request.mutable_event_id()->set_run_number(1);
    addInts(request, "addends", {{"a", 1}, {"b", 2}});
    const athremoteexec::v1::ExecuteReply reply = call(*stub, request);
    check(reply.status() == athremoteexec::v1::STATUS_INVALID_REQUEST,
          "Message with no event information is rejected");
    check(reply.diagnostics().find("event_number") != std::string::npos,
          "The rejection message contians what's missing");
  }

  // The server survived all of that and still works.
  {
    const athremoteexec::v1::ExecuteReply reply =
        call(*stub, sumRequest(1, 1, 308));
    check(reply.status() == athremoteexec::v1::STATUS_OK &&
              intOf(reply, "total", "sum") == 2,
          "The server keeps serving after failures");
  }

  if (s_failures != 0) {
    std::printf("remote_exec_menu_client: %d check(s) failed\n",
                s_failures.load());
    return EXIT_FAILURE;
  }

  std::printf("remote_exec_menu_client: all checks passed\n");
  return EXIT_SUCCESS;
}
