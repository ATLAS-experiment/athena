/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECWIRE_REMOTEEXECTIMING_H
#define ATHREMOTEEXECWIRE_REMOTEEXECTIMING_H

/**
 * @file Timing.h
 * @brief Track timing of the request throughout its execution on the server
 */

#include <chrono>
#include <cstddef>
#include <mutex>
#include <string>
#include <vector>

namespace AthRemoteExec {

/// Timestamps of a single request
struct RequestTiming {
  using Clock = std::chrono::steady_clock;
  static constexpr Clock::time_point invalid = Clock::time_point::max();

  Clock::time_point handlerEntry;  ///< RPC framework handed us a parsed request
  Clock::time_point converted;     ///< proto -> plain done
  Clock::time_point taken;         ///< the event loop picked it off the queue
  Clock::time_point recorded;      ///< slot allocated, payload in StoreGate
  Clock::time_point pushed;        ///< scheduler accepted the event
  Clock::time_point finished;      ///< scheduler handed the event back
  Clock::time_point replied;       ///< reply read back, encoded, promise set
  Clock::time_point handlerExit;   ///< plain -> proto done

  /// Check if each timing step is completed
  bool complete() const;
};

/**
 * @brief Thread-safe accumulator over RequestTiming, producing a phase table.
 *
 * All samples are collected up to @c capacity (to keep percentiles exact).
 */
class TimingAccumulator {
 public:
  explicit TimingAccumulator(size_t capacity = 10000);

  void add(const RequestTiming& timing);

  /// Multi-line phase table, or an empty string if nothing was collected.
  std::string report() const;

 private:
  mutable std::mutex m_mutex;
  std::vector<RequestTiming> m_samples;
  size_t m_capacity;
  size_t m_dropped = 0;
};

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECWIRE_REMOTEEXECTIMING_H
