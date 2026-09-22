/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RemoteExecServer.h"

#include "RemoteExecProtocolConvert.h"

#include <grpcpp/grpcpp.h>

#include "athremoteexec.grpc.pb.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace AthExRemoteExec {

/// Everything that must not appear in the header lives here.
class RemoteExecServer::Impl final : public athremoteexec::v1::SequenceExecutor::Service {
public:
  ~Impl() override = default;

  grpc::Status Execute( grpc::ServerContext* /*context*/,
                        const athremoteexec::v1::ExecuteRequest* request,
                        athremoteexec::v1::ExecuteReply* response ) override
  {
    // First and last statements of the handler, so the pair brackets
    // everything this process does with the request. gRPC's own parse happened
    // before we were called and its serialise happens after we return; neither
    // is reachable from here, which is why RemoteExecTiming.h says so out loud.
    auto timing = std::make_shared<RequestTiming>();
    timing->handlerEntry = RequestTiming::Clock::now();

    auto job = std::make_unique<Job>();
    fromProto( *request, job->request );
    job->timing = timing;
    timing->converted = RequestTiming::Clock::now();
    std::future<ExecuteReply> future = job->reply.get_future();

    {
      std::lock_guard<std::mutex> lock( m_mutex );
      if ( m_stopping ) {
        return refuse( Status::ShuttingDown, "server is shutting down" );
      }
      if ( m_queue.size() >= m_queueLimit ) {
        // Backpressure: refuse rather than grow the queue without bound.
        log( Level::Warning, "request queue full (" +
                                 std::to_string( m_queue.size() ) +
                                 "), rejecting request" );
        return refuse( Status::Overloaded, "request queue is full" );
      }
      m_queue.push_back( std::move( job ) );
    }
    m_jobAvailable.notify_one();

    // Every request gets an answer: the event loop always sets the promise, and
    // stopAccepting() fails whatever is still queued.
    ExecuteReply reply = future.get();
    if ( isRefusal( reply.status ) ) {
      // A request that was queued and then dropped -- drainPending() answers
      // those. Same refusal, same channel as one refused at the door.
      return refuse( reply.status, reply.detail );
    }
    toProto( std::move( reply ), *response );
    timing->handlerExit = RequestTiming::Clock::now();
    m_timing.add( *timing );
    return grpc::Status::OK;
  }

  grpc::Status ListSequences( grpc::ServerContext* /*context*/,
                              const athremoteexec::v1::ListSequencesRequest* /*request*/,
                              athremoteexec::v1::ListSequencesReply* response ) override
  {
    std::lock_guard<std::mutex> lock( m_mutex );
    for ( const SequenceInfo& info : m_menu ) {
      toProto( info, *response->add_sequences() );
    }
    return grpc::Status::OK;
  }

  void setMenu( std::vector<SequenceInfo> menu )
  {
    std::lock_guard<std::mutex> lock( m_mutex );
    m_menu = std::move( menu );
  }

  int start( const std::string& address, int port, size_t queueLimit,
             int maxMessageBytes, MessageFn message )
  {
    {
      std::lock_guard<std::mutex> lock( m_mutex );
      m_queueLimit = queueLimit > 0 ? queueLimit : 1;
      m_message = std::move( message );
    }

    // No server reflection. grpcurl and friends would be convenient, but
    // reflection publishes the whole descriptor pool to anyone who connects and
    // there is no consumer asking for it yet. It is one line and a link
    // dependency to add back once something needs it, and adding it is a
    // deployment decision rather than a design one.

    int boundPort = 0;
    grpc::ServerBuilder builder;
    // Without this, gRPC caps messages at 4 MB and a large payload fails as a
    // transport error with no useful diagnosis on either side.
    builder.SetMaxReceiveMessageSize( maxMessageBytes );
    builder.SetMaxSendMessageSize( maxMessageBytes );
    builder.AddListeningPort( address + ":" + std::to_string( port ),
                              grpc::InsecureServerCredentials(), &boundPort );
    builder.RegisterService( this );
    m_server = builder.BuildAndStart();
    if ( !m_server ) {
      log( Level::Error, "could not bind " + address + ":" + std::to_string( port ) );
      return -1;
    }

    log( Level::Info, "listening on " + address + ":" + std::to_string( boundPort ) );
    return boundPort;
  }

  bool pop( std::unique_ptr<Job>& job )
  {
    std::unique_lock<std::mutex> lock( m_mutex );
    m_jobAvailable.wait( lock, [this] { return m_stopping || !m_queue.empty(); } );
    if ( m_queue.empty() ) {
      return false;  // stopping
    }
    job = std::move( m_queue.front() );
    m_queue.pop_front();
    return true;
  }

  PopResult pop( std::unique_ptr<Job>& job, std::chrono::milliseconds timeout )
  {
    std::unique_lock<std::mutex> lock( m_mutex );
    m_jobAvailable.wait_for( lock, timeout,
                             [this] { return m_stopping || !m_queue.empty(); } );
    if ( !m_queue.empty() ) {
      job = std::move( m_queue.front() );
      m_queue.pop_front();
      return PopResult::Job;
    }
    return m_stopping ? PopResult::Stopped : PopResult::Timeout;
  }

  void stopAccepting()
  {
    std::deque<std::unique_ptr<Job>> pending;
    {
      std::lock_guard<std::mutex> lock( m_mutex );
      if ( m_stopping ) {
        return;
      }
      m_stopping = true;
      pending.swap( m_queue );
    }
    m_jobAvailable.notify_all();

    // Release the handler threads still blocked on their futures before asking
    // gRPC to shut down, otherwise Shutdown() waits for them until its deadline.
    // Each handler recognises the refusal and answers UNAVAILABLE.
    for ( std::unique_ptr<Job>& job : pending ) {
      ExecuteReply reply;
      reply.status = Status::ShuttingDown;
      reply.detail = "server is shutting down";
      reply.requestId = job->request.requestId;
      job->reply.set_value( std::move( reply ) );
    }
  }

  void shutdown()
  {
    // Separate from stopAccepting because the caller may have jobs of its own
    // still to answer: Shutdown() blocks until every handler has returned, and
    // a handler waiting on a promise nobody has fulfilled never returns. Stop
    // accepting, answer everything, then come back here.
    stopAccepting();
    if ( m_server ) {
      m_server->Shutdown();
      m_server->Wait();
      m_server.reset();
    }
  }

  size_t queueSize() const
  {
    std::lock_guard<std::mutex> lock( m_mutex );
    return m_queue.size();
  }

  std::string timingReport() const { return m_timing.report(); }

private:
  /**
   * @brief Turn a refusal into the gRPC status that carries it.
   *
   * Refusals leave as transport statuses and no reply body, for two reasons.
   * The practical one: gRPC will not send both. A non-OK status suppresses the
   * response message outright -- grpcpp/support/method_handler.h calls
   * SendMessagePtr only `if (status.ok())` -- so filling one in here would be
   * work whose result nobody ever sees.
   *
   * The structural one: a refusal is exactly the answer an intermediary should
   * be able to act on, and an intermediary sees an opaque payload. Envoy will
   * not parse an ExecuteReply to discover a backend is overloaded, but it reads
   * grpc-status from the HTTP/2 trailers as a matter of course, and both codes
   * below are in gRPC's default retryable set.
   *
   * RESOURCE_EXHAUSTED and UNAVAILABLE are the standard spellings; RemoteExecClient
   * maps them back so a caller still sees one Status vocabulary.
   */
  static grpc::Status refuse( Status status, const std::string& detail )
  {
    return grpc::Status( status == Status::Overloaded
                             ? grpc::StatusCode::RESOURCE_EXHAUSTED
                             : grpc::StatusCode::UNAVAILABLE,
                         detail );
  }

  /// Caller must hold m_mutex, or be outside the served phase.
  void log( Level level, const std::string& text ) const
  {
    if ( m_message ) {
      m_message( level, text );
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
  /// Its own lock, deliberately not m_mutex: handler threads add to it after
  /// the reply is serialised, and making that contend with the request queue
  /// would have the measurement perturb the thing being measured.
  TimingAccumulator m_timing;
};

RemoteExecServer::RemoteExecServer() : m_impl( std::make_unique<Impl>() ) {}

RemoteExecServer::~RemoteExecServer()
{
  m_impl->shutdown();
}

void RemoteExecServer::setMenu( std::vector<SequenceInfo> menu )
{
  m_impl->setMenu( std::move( menu ) );
}

int RemoteExecServer::start( const std::string& address, int port, size_t queueLimit,
                      int maxMessageBytes, MessageFn message )
{
  return m_impl->start( address, port, queueLimit, maxMessageBytes,
                        std::move( message ) );
}

bool RemoteExecServer::pop( std::unique_ptr<Job>& job )
{
  return m_impl->pop( job );
}

RemoteExecServer::PopResult RemoteExecServer::pop( std::unique_ptr<Job>& job,
                                     std::chrono::milliseconds timeout )
{
  return m_impl->pop( job, timeout );
}

void RemoteExecServer::stopAccepting()
{
  m_impl->stopAccepting();
}

void RemoteExecServer::shutdown()
{
  m_impl->shutdown();
}

size_t RemoteExecServer::queueSize() const
{
  return m_impl->queueSize();
}

std::string RemoteExecServer::timingReport() const
{
  return m_impl->timingReport();
}

}  // namespace AthExRemoteExec
