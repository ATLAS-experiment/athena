/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ExecuteOngRPCCall.h"

#include <grpcpp/grpcpp.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "EFTrackingDataTransfer/Message.grpc.pb.h"
#include "EFTrackingDataTransfer/Message.pb.h"
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/task_group.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

/**
 * @brief Helper that implements the gRPC server API.
 * In doComputation it will notify the owning tool about the incoming data and
 * then wait until the computation completes before returning.
 */
struct ReqResp {
  const OffloadMessage* request = nullptr;
  OffloadMessage* response = nullptr;
  bool done = false;
  std::mutex mutex;
  std::condition_variable cv;

  void waitForCompletion() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return done; });
  }

  void complete() {
    {
      std::lock_guard<std::mutex> lock(mutex);
      done = true;
    }
    cv.notify_one();
  }
};
typedef uint64_t eventID_t;
eventID_t getId(const OffloadMessage* m) {
  return m->event().eventnumber();
}

typedef uint64_t eventID_t;
eventID_t getId(const EventContext& context) {
  return context.eventID().event_number();
}

// queue of requests, it is filled by doComputation, and drained by executeEvent
static tbb::concurrent_queue<std::shared_ptr<ReqResp>> s_incommingRequests;

static tbb::concurrent_unordered_map<uint64_t, std::shared_ptr<ReqResp>>
    s_ongoingRequests;

static tbb::concurrent_unordered_map<uint64_t, std::shared_ptr<ReqResp>>
    s_completedRequests;

struct gRPCServerHelper final : public UniversalOffloadService::Service {
  explicit gRPCServerHelper() {}

  grpc::Status doComputation(ServerContext*, const OffloadMessage* request,
                             OffloadMessage* response) override {
    auto incomming = std::make_shared<ReqResp>();
    incomming->request = request;
    incomming->response = response;
    auto id = getId(incomming->request);
    std::cout << "Request from the client " << id << "\n";
    s_incommingRequests.push(incomming);

    while (!s_completedRequests.contains(id)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    auto r = s_completedRequests[id];
    r->response->set_identifier(std::format("done for event={}", id));
    std::cout << "Returning to the client " << id << "\n";
    // TODO we should fill event info in the return message so that client can
    // crosscheck this needs protection
    // s_completedRequests.unsafe_erase(id);
    return grpc::Status::OK;
  }

 private:
};

ExecuteOngRPCCall::ExecuteOngRPCCall(const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode ExecuteOngRPCCall::executeEvent(MinimalEventLoopMgr* el,
                                           EventContext&& context) {

  std::shared_ptr<ReqResp> r;
  while (!s_incommingRequests.try_pop(r)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  ATH_MSG_INFO("Got input, upacking with " << m_unpackingTools.size()
                                           << " tools...");
  for (auto& tool : m_unpackingTools) {
    ATH_MSG_INFO("Unpacking with " << tool.name() << "...");
    ATH_CHECK(tool->unpack(*(r->request), context));
  }
  ATH_MSG_ALWAYS("After decoding done, executing algorithms ...");
  StatusCode sc = el->executeEvent(std::move(context));
  const auto id = getId(r->request);
  ATH_MSG_ALWAYS("Event send for execution ... id " << id);
  r->response->set_identifier("ongoing");
  s_ongoingRequests[id] = std::move(r);
  return StatusCode::SUCCESS;
}

StatusCode ExecuteOngRPCCall::completeEvent(MinimalEventLoopMgr* el,
                                            const EventContext& context) {
  // copy necessary data from the store to output message
  // there should be a set of tools that would fetch and pack the results
  ATH_MSG_INFO("eventStore content after execution\n" << evtStore()->dump());
  auto eventId = getId(context);
  ATH_MSG_INFO("will look for matching request " << eventId);
  std::shared_ptr<ReqResp> r = s_ongoingRequests[eventId];
  ATH_MSG_INFO("found one " << (void*)(r->request) << " "
                            << (void*)(r->response));

  for (auto& tool : m_packingTools)
    ATH_CHECK(tool->pack(*(r->response), context));

  r->response->set_identifier("completed");
  s_completedRequests[eventId] = std::move(r);
  // s_ongoingRequests.unsafe_erase(eventId);
  ATH_MSG_INFO("Request completed, will be now handed by doComputation");

  return StatusCode::SUCCESS;
}

// static tbb::task_group s_serverTask;
static std::unique_ptr<gRPCServerHelper> s_serverHelper;
static std::unique_ptr<Server> s_server;

StatusCode ExecuteOngRPCCall::initialize() {

  s_serverHelper = std::make_unique<gRPCServerHelper>();

  ServerBuilder builder;
  builder.AddListeningPort(m_address.value(),
                           grpc::InsecureServerCredentials());
  builder.RegisterService(s_serverHelper.get());

  s_server = builder.BuildAndStart();
  if (!s_server) {
    ATH_MSG_ERROR(
        "gRPC Server (doComputation recieving end point) cration failed");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Server ready, putting it to waiting state");
  // FIXME: This clashes with tbb inside Gaudi causing one of the slots to hang
  // s_serverTask.run([]() { s_server->Wait(); });
  ATH_MSG_INFO("Server waiting");

  ATH_CHECK(m_unpackingTools.retrieve());
  ATH_CHECK(m_packingTools.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode ExecuteOngRPCCall::finalize() {
  // some sort of close-connection action would need to be implemented here
  return StatusCode::SUCCESS;
}
