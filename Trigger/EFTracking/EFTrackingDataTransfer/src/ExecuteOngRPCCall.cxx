/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ExecuteOngRPCCall.h"
#include <grpcpp/grpcpp.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include "EFTrackingDataTransfer/Message.grpc.pb.h"
#include "EFTrackingDataTransfer/Message.pb.h"
#include "tbb/concurrent_queue.h"
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

// queue of requests, it is filled by doComputation, and drained by executeEvent
static tbb::concurrent_queue<std::shared_ptr<ReqResp>> s_pendingRequests;


struct gRPCServerHelper final : public UniversalOffloadService::Service {
  explicit gRPCServerHelper() {}

  grpc::Status doComputation(ServerContext*, const OffloadMessage* request,
                             OffloadMessage* response) override {
    auto pending = std::make_shared<ReqResp>();
    pending->request = request;
    pending->response = response;

    s_pendingRequests.push(pending);

    pending->waitForCompletion();
    return grpc::Status::OK;
  }

 private:
  // ExecuteOngRPCCall* m_owner = nullptr;
};

ExecuteOngRPCCall::ExecuteOngRPCCall(const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode ExecuteOngRPCCall::executeEvent(MinimalEventLoopMgr* el,
                                           EventContext&& ctx) {
  
  std::shared_ptr<ReqResp> r;
  while(! s_pendingRequests.try_pop(r) ) {
    // ATH_MSG_ALWAYS("Trying to pop");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }  
  ATH_MSG_ALWAYS("Got input, upacking ...");
  for ( auto& tool: m_packingTools) {
    ATH_CHECK(tool->unpack(*(r->request), ctx));
  }
  ATH_MSG_ALWAYS("Upacking done, executing algorithms ...");
  StatusCode sc = el->executeEvent(std::move(ctx));
  ATH_MSG_ALWAYS("Processed, harvesting result ...");
  r->response->set_identifier("done");
  // TODO the OffloadMessage needs a field for execution status
  r->complete();

  return StatusCode::SUCCESS;
}


static tbb::task_group s_serverTask;
static std::unique_ptr<gRPCServerHelper> s_serverHelper;
static std::unique_ptr<Server> s_server;

StatusCode ExecuteOngRPCCall::initialize() {

  s_serverHelper = std::make_unique<gRPCServerHelper>();

  ServerBuilder builder;
  builder.AddListeningPort(m_address.value(), grpc::InsecureServerCredentials());
  builder.RegisterService(s_serverHelper.get());

  s_server = builder.BuildAndStart();
  if (!s_server) {
    ATH_MSG_ERROR("gRPC Server (doComputation recieving end point) cration failed");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Server ready, putting it to waiting state");
  s_serverTask.run(  [] () { s_server->Wait(); });
  ATH_MSG_INFO("Server waiting");

  ATH_CHECK(m_packingTools.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode ExecuteOngRPCCall::finalize() {
  // some sort of close-connection action would need to be implemented here
  return StatusCode::SUCCESS;
}
