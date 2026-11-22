/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecWire/Server.h>
#include <athremoteexec.grpc.pb.h>
#include <grpcpp/grpcpp.h>

#include <condition_variable>
#include <deque>
#include <mutex>

#include "ProtocolConvert.h"

namespace AthRemoteExec {

class Server::Impl final : public athremoteexec::v1::SequenceExecutor::Service {
 public:
  ~Impl() override = default;

  grpc::Status Execute(grpc::ServerContext* /*context*/,
                       const athremoteexec::v1::ExecuteRequest* request,
                       athremoteexec::v1::ExecuteReply* response) override {
    auto timing = std::make_shared<RequestTiming>();
    timing->handlerEntry = RequestTiming::Clock::now();

    auto job = std::make_unique<Job>();
    fromProto(*request, job->request);
    job->timing = timing;
    timing->converted = RequestTiming::Clock::now();
    std::future<ExecuteReply> future = job->reply.get_future();

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_stopping) {
        return refuse(Status::ShuttingDown, "server is shutting down");
      }
      if (m_queue.size() >= m_queueLimit) {
        // Backpressure: refuse rather than grow the queue without bound.
        log(Level::Warning, "request queue full (" +
                                std::to_string(m_queue.size()) +
                                "), rejecting request");
        return refuse(Status::Overloaded, "request queue is full");
      }
      m_queue.push_back(std::move(job));
    }
    m_jobAvailable.notify_one();

    ExecuteReply reply = future.get();
    if (isRefusal(reply.status)) {
      return refuse(reply.status, reply.diagnostics);
    }
    toProto(std::move(reply), *response);
    timing->handlerExit = RequestTiming::Clock::now();
    m_timing.add(*timing);
    return grpc::Status::OK;
  }

  grpc::Status ListSequences(
      grpc::ServerContext* /*context*/,
      const athremoteexec::v1::ListSequencesRequest* /*request*/,
      athremoteexec::v1::ListSequencesReply* response) override {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const SequenceInfo& info : m_menu) {
      toProto(info, *response->add_sequences());
    }
    return grpc::Status::OK;
  }

  void setMenu(std::vector<SequenceInfo> menu) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_menu = std::move(menu);
  }

  int start(const std::string& address, int port, size_t queueLimit,
            int maxMessageBytes, MessageFn message) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_queueLimit = queueLimit > 0 ? queueLimit : 1;
      m_message = std::move(message);
    }

    int boundPort = 0;
    grpc::ServerBuilder builder;
    // gRPC caps messages at 4 MB by defalut and a large payload could fail as a
    // transport error with no useful diagnosis on either side.
    builder.SetMaxReceiveMessageSize(maxMessageBytes);
    builder.SetMaxSendMessageSize(maxMessageBytes);
    builder.AddListeningPort(address + ":" + std::to_string(port),
                             grpc::InsecureServerCredentials(), &boundPort);
    builder.RegisterService(this);
    m_server = builder.BuildAndStart();
    if (!m_server) {
      log(Level::Error,
          "could not bind " + address + ":" + std::to_string(port));
      return -1;
    }

    log(Level::Info,
        "listening on " + address + ":" + std::to_string(boundPort));
    return boundPort;
  }

  bool pop(std::unique_ptr<Job>& job) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_jobAvailable.wait(lock,
                        [this] { return m_stopping || !m_queue.empty(); });
    if (m_queue.empty()) {
      return false;  // stopping
    }
    job = std::move(m_queue.front());
    m_queue.pop_front();
    return true;
  }

  PopResult pop(std::unique_ptr<Job>& job, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_jobAvailable.wait_for(lock, timeout,
                            [this] { return m_stopping || !m_queue.empty(); });
    if (!m_queue.empty()) {
      job = std::move(m_queue.front());
      m_queue.pop_front();
      return PopResult::Job;
    }
    return m_stopping ? PopResult::Stopped : PopResult::Timeout;
  }

  void stopAccepting() {
    std::deque<std::unique_ptr<Job>> pending;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_stopping) {
        return;
      }
      m_stopping = true;
      pending.swap(m_queue);
    }
    m_jobAvailable.notify_all();

    // Release the handler threads still blocked on their futures before asking
    // gRPC to shut down.
    for (std::unique_ptr<Job>& job : pending) {
      ExecuteReply reply;
      reply.status = Status::ShuttingDown;
      reply.diagnostics = "server is shutting down";
      reply.requestId = job->request.requestId;
      job->reply.set_value(std::move(reply));
    }
  }

  void shutdown() {
    // Separate from stopAccepting() because the caller may have jobs of its own
    // that need to be answered. Shutdown() blocks until every handler has
    // returned, and a handler waiting on a promise nobody has fulfilled never
    // returns.
    stopAccepting();
    if (m_server) {
      m_server->Shutdown();
      m_server->Wait();
      m_server.reset();
    }
  }

  size_t queueSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
  }

  std::string timingReport() const { return m_timing.report(); }

 private:
  static grpc::Status refuse(Status status, const std::string& detail) {
    return grpc::Status(status == Status::Overloaded
                            ? grpc::StatusCode::RESOURCE_EXHAUSTED
                            : grpc::StatusCode::UNAVAILABLE,
                        detail);
  }

  void log(Level level, const std::string& text) const {
    if (m_message) {
      m_message(level, text);
    }
  }

  mutable std::mutex m_mutex;
  std::condition_variable m_jobAvailable;
  std::deque<std::unique_ptr<Job>> m_queue;
  std::vector<SequenceInfo> m_menu;
  size_t m_queueLimit = 1;
  bool m_stopping = false;
  MessageFn m_message;
  std::unique_ptr<grpc::Server> m_server;
  TimingAccumulator m_timing;
};

Server::Server() : m_impl(std::make_unique<Impl>()) {}

Server::~Server() {
  m_impl->shutdown();
}

void Server::setMenu(std::vector<SequenceInfo> menu) {
  m_impl->setMenu(std::move(menu));
}

int Server::start(const std::string& address, int port, size_t queueLimit,
                  int maxMessageBytes, MessageFn message) {
  return m_impl->start(address, port, queueLimit, maxMessageBytes,
                       std::move(message));
}

bool Server::pop(std::unique_ptr<Job>& job) {
  return m_impl->pop(job);
}

Server::PopResult Server::pop(std::unique_ptr<Job>& job,
                              std::chrono::milliseconds timeout) {
  return m_impl->pop(job, timeout);
}

void Server::stopAccepting() {
  m_impl->stopAccepting();
}

void Server::shutdown() {
  m_impl->shutdown();
}

size_t Server::queueSize() const {
  return m_impl->queueSize();
}

std::string Server::timingReport() const {
  return m_impl->timingReport();
}

}  // namespace AthRemoteExec
