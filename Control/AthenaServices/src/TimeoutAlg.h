/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */

/**
 * @brief Algorithm to monitor event timeouts
 * @author Frank Winklmeier
 * @date Sep, 2025
 */

#ifndef ATHENASERVICES_TIMEOUTALG_H
#define ATHENASERVICES_TIMEOUTALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "AthenaKernel/Timeout.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/IIncidentListener.h"

#include <chrono>
#include <future>
#include <thread>


/**
 * @brief Algorithm providing a watchdog thread for event timeouts.
 *
 * This algorithm should run early on (ideally first) in the event sequence.
 * It records the event start time and launches a watchdog thread that checks
 * periodically if an event has timed out.
 *
 * See the algorithm properties for possible actions on an event timeout.
 *
 */
class TimeoutAlg : public extends<AthReentrantAlgorithm, IIncidentListener>,
                   public Athena::TimeoutMaster
{
public:
  /// Use base class ctor
  using base_class::base_class;

  virtual StatusCode initialize() override;
  virtual StatusCode execute (const EventContext& ctx) const override;
  virtual StatusCode stop() override;
  virtual void handle(const Incident& inc) override;

private:
  using clock_t = std::chrono::steady_clock;

  /// Watchdog thread
  void timeoutThread();

  /// Handle timeout
  void handleTimeout(EventContext::ContextID_t slot);

  ///@name Properties
  ///@{
  Gaudi::Property<unsigned long long> m_timeoutProp{
    this, "Timeout", 0, "Timeout in nanoseconds (0 means disabled)"
  };
  Gaudi::Property<unsigned long long> m_checkInterval{
    this, "MaxCheckInterval", 10*1e9, "Maximum time (ns) between timeout checks"
  };
  Gaudi::Property<bool> m_dumpState{
    this, "DumpSchedulerState", false, "Print scheduler state on timeout"
  };
  Gaudi::Property<bool> m_abort{
    this, "AbortJob", false, "Abort job on timeout"
  };
  ///@}

  /// Timeout property as duration
  std::chrono::nanoseconds m_timeout;

  /// Start time of each event per slot
  mutable SG::SlotSpecificObj<clock_t::time_point> m_eventStartTime ATLAS_THREAD_SAFE;

  /// Watchdog thread
  mutable std::thread m_thread ATLAS_THREAD_SAFE;

  /// Signal to stop watchdog thread
  std::promise<void> m_stop_thread;

  /// Has watchdog thread already been stopped? (to avoid setting future twice)
  std::atomic<bool> m_stopped{false};

  /// Mutex for handleTimeout
  std::mutex m_handleMutex;
};

#endif
