#include <iostream>
#include <memory>
#include <grpcpp/grpcpp.h>

#include "EFTrackingDataTransfer/Message.pb.h"
#include "EFTrackingDataTransfer/Message.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class TestServiceImpl final : public UniversalOffloadService::Service {
public:
    int increment=0;
    Status doComputation(ServerContext* context,
                   const OffloadMessage* request,
                   OffloadMessage* response) override {

        std::cout << "... TestServiceImpl::Received request of ID: " << request->identifier() << "\n";
        if ( request->identifier() == "Enterprise to Starfleet Command.") {
            response->set_identifier("Starfleet Command here. Go ahead, Enterprise.");
            return Status::OK;
        }


        // Copy context
        *response->mutable_event() = request->event();

        // Copy ID
        response->set_identifier("TestServiceImpl back to "+request->identifier());

        // Process data
        auto& x = request->float_branches().at("x").values();
        for (int v : x) {
            response->mutable_float_branches()->at("x").add_values(v+1);
        }
        increment++;
        std::cout << "... TestServiceImpl::Returing response with: " << response->identifier() << "\n";

        return Status::OK;
    }
};

int main() {
    std::string address("0.0.0.0:50051");

    TestServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cout << "Server listening on " << address << "\n";
    server->Wait();
}