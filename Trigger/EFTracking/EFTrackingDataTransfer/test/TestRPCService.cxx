#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <functional>
#include <thread>
#include <chrono>

#include "EFTrackingDataTransfer/Message.grpc.pb.h"
#include "EFTrackingDataTransfer/Message.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

void printEventInfoMessage(const EventInfoMessage& ei) {
      std::cout << "EventInfoMessage:";
      std::cout << "  runnumber: " << ei.runnumber();
      std::cout << "  eventnumber: " << ei.eventnumber();
      std::cout << "  lumiblock: " << ei.lumiblock();
      std::cout << "  timestamp: " << ei.timestamp();
      std::cout << "  timestampnsoffset: " << ei.timestampnsoffset();
      std::cout << "  bcid: " << ei.bcid() << std::endl;
}

// Minimal EventLoop manager that implements the gRPC service interface but
// delegates the actual computation to an external "tool" (handler).
class EventLoopMgr final : public UniversalOffloadService::Service {
 public:
  using Handler = std::function<Status(ServerContext*, const OffloadMessage*, OffloadMessage*)>;

  explicit EventLoopMgr(Handler handler) : handler_(std::move(handler)) {}

  Status doComputation(ServerContext* ctx, const OffloadMessage* request,
                       OffloadMessage* response) override {
    if (handler_) return handler_(ctx, request, response);
    return Status(grpc::StatusCode::UNIMPLEMENTED, "No offload handler configured");
  }

 private:
  Handler handler_;
};

// Placeholder that represents the separate tool implementing the computation.
// Replace or implement this function in the real tool integration.
Status offloadToolHandler(ServerContext* ctx, const OffloadMessage* request,
                          OffloadMessage* response) {
  (void)ctx;
  (void)request;
  (void)response;
  return Status(grpc::StatusCode::UNIMPLEMENTED, "offloadToolHandler not implemented");
}

class TestServiceImpl final : public UniversalOffloadService::Service {
 public:
  int increment = 0;
  Status doComputation(ServerContext*, const OffloadMessage* request,
                       OffloadMessage* response) override {

    std::cerr << "... TestServiceImpl::Received request of ID: "
              << request->identifier() << "\n";
  printEventInfoMessage(request->event());

    if (request->identifier() == "Enterprise to Starfleet Command.") {
      response->set_identifier("Starfleet Command here. Go ahead, Enterprise.");
      std::cerr << ".... init message received and respondeed\n";
      return Status::OK;
    }

    for (auto& k : request->uint_branches()) {
      std::cerr << ".....  " << k.first << " " << k.second.values_size()  << "\n";
    }

    // Copy context
    *response->mutable_event() = request->event();
    // Copy ID
    response->set_identifier("TestServiceImpl back to " +
                             request->identifier());
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cerr << "... TestServiceImpl::Returing response with: "
              << response->identifier() << "\n";

    return Status::OK;
  }
};

int main() {
  std::string address("0.0.0.0:50051");

  std::cerr << "protobuf version " << int(PROTOBUF_VERSION) << std::endl;

  EventLoopMgr service(offloadToolHandler);

  ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cerr << "Server listening on " << address << "\n";
  server->Wait();
}