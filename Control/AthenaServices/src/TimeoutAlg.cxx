/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/**
 * @brief Algorithm to monitor event timeouts
 * @author Frank Winklmeier
 * @date Sep, 2025
 */

#include "TimeoutAlg.h"

#include "AthenaKernel/ICoreDumpSvc.h"
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/ServiceHandle.h"

#include <format>


StatusCode TimeoutAlg::initialize()
{
  m_timeout = std::chrono::nanoseconds(m_timeoutProp);

  // Subscribe to EndAlgorithms (includes output sequence)
  ServiceHandle<IIncidentSvc> incSvc("IncidentSvc/IncidentSvc", name());
  ATH_CHECK(incSvc.retrieve());
  incSvc->addListener(this, "EndAlgorithms", /*priority*/ 0);

  return StatusCode::SUCCESS;
}


StatusCode TimeoutAlg::execute (const EventContext& ctx) const
{
  // Timeout thread is started on first event to make sure this also works
  // in athenaMP (threads usually don't survive forking).
  [[maybe_unused]] static const bool initThread = [&](){
    if (m_timeoutProp > 0) {
      const auto nc_this ATLAS_THREAD_SAFE = const_cast<TimeoutAlg*>(this);
      m_thread = std::thread(&TimeoutAlg::timeoutThread, nc_this);
    }
    return true;
  }();

  // Set event start time for current slot
  *m_eventStartTime.get(ctx) = clock_t::now();

  return StatusCode::SUCCESS;
}


void TimeoutAlg::handle(const Incident& inc)
{
  if (inc.type() == "EndAlgorithms") {
    ATH_MSG_DEBUG("Resetting event timeout for slot " << inc.context().slot());
    // Reset start time for slot to zero
    *m_eventStartTime.get(inc.context()) = {};
  }
}


StatusCode TimeoutAlg::stop()
{
  if (m_thread.joinable() && !m_stopped.exchange(true)) {
    // Signal timeout thread to stop
    ATH_MSG_DEBUG("Stopping timeout thread");
    m_stop_thread.set_value();
  }

  return StatusCode::SUCCESS;
}


void TimeoutAlg::timeoutThread()
{
  ATH_MSG_INFO(std::format("Setting per-event timeout of {}",
                           std::chrono::duration<double>(m_timeout)));

  // Wakeup at regular intervals (with a minimum frequency, useful for long timeouts)
  const std::chrono::nanoseconds wakeup_interval =
    std::min(m_timeout, std::chrono::nanoseconds(m_checkInterval));

  // Loop until we have received stop signal
  auto stop_signal = m_stop_thread.get_future();
  while ( stop_signal.wait_for(wakeup_interval) == std::future_status::timeout ) {

    // Loop over all slots and check if event has reached timeout
    const auto now = clock_t::now();
    for (EventContext::ContextID_t slot = 0;
         const auto& startTime : m_eventStartTime) {

      if (startTime.time_since_epoch().count() > 0 &&  now > startTime + m_timeout) {
        handleTimeout(slot);
      }

      ++slot;
    }
  }
}


void TimeoutAlg::handleTimeout(EventContext::ContextID_t slot)
{
  // To avoid getting another timeout while handling this one
  std::scoped_lock lock(m_handleMutex);

  // Create minimal context with slot number
  const EventContext ctx(0, slot);

  // Don't duplicate the actions if the timeout was already reached for this slot
  if (Athena::Timeout::instance(ctx).reached()) return;

  // Print ERROR message
  const std::string msg = std::format("Event timeout ({}) in slot {} reached",
                                      std::chrono::duration<double>(m_timeout), slot);
  ATH_MSG_ERROR(msg);

  // Set timeout flag
  setTimeout(Athena::Timeout::instance(ctx));

  // Dump scheduler state if requested
  if (m_dumpState) {
    ServiceHandle<IScheduler> schedulerSvc("AvalancheSchedulerSvc", name());
    if (schedulerSvc.retrieve().isSuccess()) {
      schedulerSvc->dumpState();
    }
  }

  // Abort job if requested
  if (m_abort) {
    // Stop the timeout thread to avoid additional triggers
    stop().ignore();

    // Tell CoreDumpSvc about the reason for the abort
    ServiceHandle<ICoreDumpSvc> coreDumpSvc("CoreDumpSvc", name());
    if ( coreDumpSvc.retrieve().isSuccess() ) {
      coreDumpSvc->setCoreDumpInfo(ctx, "Reason", msg);
    }
    else {
      std::cerr << msg << std::endl;
    }
    // Abort job (and let CoreDumpSvc handle SIGABRT)
    std::abort();
  }

}
