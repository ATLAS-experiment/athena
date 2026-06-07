#include <iostream>
#include <memory>
#include <grpcpp/grpcpp.h>

#include "../src/TestMessages.pb.h"
#include "../src/TestMessages.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class TestServiceImpl final : public TestService::Service {
public:
    int increment=0;
    Status doComputation(ServerContext* context,
                   const TestInputData* request,
                   TestOutputData* response) override {

        std::cout << "... TestServiceImpl::Received request of ID: " << request->id() << "\n";
        
        // Copy context
        *response->mutable_eventinfo() = request->eventinfo();

        // Copy ID
        response->set_id("TestServiceImpl back to "+request->id());

        // Process data
        for (int v : request->data()) {
            response->add_data(v * 2+increment);
        }
        increment++;
        std::cout << "... TestServiceImpl::Returing response with: " << response->id() << "\n";

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