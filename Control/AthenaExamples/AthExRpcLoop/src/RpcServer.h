/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCSERVER_H
#define ATHEXRPCLOOP_RPCSERVER_H

/**
 * @file RpcServer.h
 * @brief The gRPC server, with no Gaudi and no generated protobuf code on the
 *        interface.
 *
 * Handler threads do as little as possible: validate the request, push a job
 * onto a bounded queue, and block on its future. They never touch StoreGate,
 * the scheduler, or a MsgStream -- all logging goes through the message funnel
 * (the lesson from graphics/JiveXML/src/ONCRPCServerSvc.cxx:142-149).
 */

#include "RpcTiming.h"
#include "RpcWire.h"

#include <chrono>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

namespace AthExRpc {

class RpcServer {
public:
  /// One unit of work handed from a handler thread to the event loop.
  struct Job {
    ExecuteRequest request;
    std::promise<ExecuteReply> reply;
    /// Shared rather than owned: the handler thread stamps the two ends and
    /// the event loop the middle, and this Job is destroyed by the loop before
    /// the handler thread wakes from its future. Never null.
    std::shared_ptr<RequestTiming> timing;
  };

  /// Severity levels, matching MSG::Level so the funnel can forward them.
  enum class Level : int { Debug = 2, Info = 3, Warning = 4, Error = 6 };

  /// Log sink, called from arbitrary threads; the implementation must be
  /// thread-safe.
  using MessageFn = std::function<void( Level, const std::string& )>;

  /// Outcome of a bounded wait for work.
  enum class PopResult {
    Job,      ///< a job was handed over
    Timeout,  ///< nothing arrived within the timeout
    Stopped   ///< the server is shutting down and no more jobs will come
  };

  RpcServer();
  ~RpcServer();

  RpcServer( const RpcServer& ) = delete;
  RpcServer& operator=( const RpcServer& ) = delete;

  /// Sequences advertised by the ListSequences RPC. Call before start().
  void setMenu( std::vector<SequenceInfo> menu );

  /**
   * @brief Start listening.
   * @param address    interface to bind, e.g. "0.0.0.0"
   * @param port       TCP port; 0 lets the OS choose
   * @param queueLimit maximum queued jobs before requests are rejected with
   *                   Overloaded (backpressure instead of unbounded memory)
   * @param maxMessageBytes largest request or reply to accept. gRPC's own
   *                   default is 4 MB, which a realistic payload passes
   *                   without trying
   * @param message    log sink, may be empty
   * @return the bound port, or -1 if the server could not start
   */
  int start( const std::string& address, int port, size_t queueLimit,
             int maxMessageBytes, MessageFn message );

  /**
   * @brief Take the next job, blocking until one arrives or the server stops.
   * @return false if the server is stopping and no job was returned.
   */
  bool pop( std::unique_ptr<Job>& job );

  /**
   * @brief Take the next job, waiting at most @c timeout for one.
   *
   * The MT loop manager needs this: while events are in flight it must come
   * back regularly to drain the scheduler and to check request deadlines, so it
   * cannot block here indefinitely.
   */
  PopResult pop( std::unique_ptr<Job>& job, std::chrono::milliseconds timeout );

  /// Stop accepting requests and fail every queued job with ShuttingDown,
  /// without waiting for gRPC to wind down. Idempotent.
  ///
  /// Split out from shutdown() because a caller that holds jobs of its own
  /// must answer them *before* gRPC is told to stop: Shutdown() blocks until
  /// every handler has returned, and a handler waiting on a promise nobody has
  /// fulfilled never returns.
  void stopAccepting();

  /// stopAccepting(), then wind gRPC down and join the server thread.
  /// Idempotent; also called by the destructor.
  void shutdown();

  /// Number of jobs currently waiting to be picked up.
  size_t queueSize() const;

  /// Phase table for the requests served so far; empty if none completed.
  /// See RpcTiming.h for what it does and does not include.
  std::string timingReport() const;

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCSERVER_H
