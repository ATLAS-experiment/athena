#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
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

  TestServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cerr << "Server listening on " << address << "\n";
  server->Wait();
}