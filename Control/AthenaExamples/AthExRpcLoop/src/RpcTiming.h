/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCTIMING_H
#define ATHEXRPCLOOP_RPCTIMING_H

/**
 * @file RpcTiming.h
 * @brief Where a request's time goes inside the process.
 *
 * A client measures a round trip, which bundles together three quite different
 * things: the socket, gRPC's own protobuf (de)serialisation, and everything
 * this package does. Only the third is a property of the design being
 * evaluated -- whether payload bytes are worth moving over a wire at all is a
 * system-architecture question that does not depend on how Athena dispatches
 * them.
 *
 * So the server timestamps each request at the boundaries it can see, and
 * reports the phases. The two ends are taken inside the gRPC handler, so
 * `handlerEntry -> handlerExit` is the whole in-process cost *except* gRPC's
 * own parse and serialise, which happen outside the handler and are therefore
 * charged to neither side here. That is a deliberate omission rather than an
 * oversight: what this table is for is finding which part of *this* code a
 * request is spending its time in.
 *
 * Plain C++ on purpose: RpcServer must stay free of Gaudi, and this is shared
 * between it and the loop manager.
 */

#include <chrono>
#include <cstddef>
#include <mutex>
#include <string>
#include <vector>

namespace AthExRpc {

/**
 * @brief Timestamps taken as one request passes through the server.
 *
 * Held by shared_ptr because the two ends are written by a gRPC handler thread
 * and the middle by the event loop, and the Job that carries the middle is
 * destroyed before the handler thread wakes up.
 */
struct RequestTiming {
  using Clock = std::chrono::steady_clock;

  Clock::time_point handlerEntry;  ///< gRPC handed us a parsed request
  Clock::time_point converted;     ///< protobuf -> plain done
  Clock::time_point taken;         ///< the event loop picked it off the queue
  Clock::time_point recorded;      ///< slot allocated, payload in StoreGate
  Clock::time_point pushed;        ///< scheduler accepted the event
  Clock::time_point finished;      ///< scheduler handed the event back
  Clock::time_point replied;       ///< reply read back, encoded, promise set
  Clock::time_point handlerExit;   ///< plain -> protobuf done

  /// True once every phase has been stamped; a request that failed validation
  /// or timed out skips some, and averaging those in would flatter the result.
  bool complete() const;
};

/**
 * @brief Thread-safe accumulator over RequestTiming, producing a phase table.
 *
 * Samples are kept so percentiles are exact; a server left running would grow
 * without bound, so collection stops at @c capacity and says so in the report.
 */
class TimingAccumulator {
public:
  explicit TimingAccumulator( size_t capacity = 200000 );

  void add( const RequestTiming& timing );

  size_t count() const;
  size_t dropped() const;

  /// Multi-line phase table, or an empty string if nothing was collected.
  std::string report() const;

private:
  mutable std::mutex m_mutex;
  std::vector<RequestTiming> m_samples;
  size_t m_capacity;
  size_t m_dropped = 0;
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCTIMING_H
