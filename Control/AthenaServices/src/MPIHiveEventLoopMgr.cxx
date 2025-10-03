/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MPIHiveEventLoopMgr.h"

// Gaudi includes
#include "GaudiKernel/AppReturnCode.h"

// Utilities
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaKernel/ClusterMessage.h"

// Standard Library
#include <chrono>
#include <fstream>
#include <string>

using Clock = std::chrono::high_resolution_clock;

// Standard Constructor
MPIHiveEventLoopMgr::MPIHiveEventLoopMgr(const std::string& name,
                                         ISvcLocator* svcLoc)
    : AthenaHiveEventLoopMgr(name, svcLoc) {}

// Standard Destructor
MPIHiveEventLoopMgr::~MPIHiveEventLoopMgr() = default;

/// Initialize
/// Initializes AthenaHiveEventLoop and MPIClusterSvc
StatusCode MPIHiveEventLoopMgr::initialize() {
  // Initialize cluster svc
  ATH_CHECK(m_clusterSvc.retrieve());
  return AthenaHiveEventLoopMgr::initialize();
}

/// Finalize
/// Finalizes AthenaHiveEventLoopMgr
StatusCode MPIHiveEventLoopMgr::finalize() {
  return AthenaHiveEventLoopMgr::finalize();
}

/// nextEvent
/// Dispatches to either masterEventLoop or workerEventLoop
StatusCode MPIHiveEventLoopMgr::nextEvent(int maxevt) {
  // make nextEvent(0) a dummy call
  if (0 == maxevt) {
    return StatusCode::SUCCESS;
  }

  // Reset the application return code.
  Gaudi::setAppReturnCode(m_appMgrProperty, Gaudi::ReturnCode::Success, true)
      .ignore();
  ATH_MSG_INFO("Starting loop on events");

  if (m_clusterSvc->rank() == 0) {
    return masterEventLoop(maxevt);
  }
  return workerEventLoop();
}

/// Master event loop
StatusCode MPIHiveEventLoopMgr::masterEventLoop(int maxEvt) {
  ATH_MSG_INFO("Running with " << m_clusterSvc->numRanks() << " ranks");
  // Determine number of events to process
  int skipEvts = int(m_firstEventIndex.value());
  if (m_evtSelector != nullptr) {
    int evt = size();
    if (evt == -1) {
      m_clusterSvc->abort();
      return StatusCode::FAILURE;
    }
    if (maxEvt < 0 || maxEvt > evt) {
      maxEvt = evt;
    }
    ATH_MSG_INFO("Will be processing " << maxEvt << " events");
  }

  // Setup worker status DB (Spare one at start)
  std::vector<bool> workers_done(m_clusterSvc->numRanks(), false);
  workers_done[0] =
      true;  // Set 0 to true because it doesn't correspond to a worker
  int num_workers_done = 1;  // Init to 1 so we can compare to numRanks
  std::vector<ClusterMessage::WorkerStatus> statuses(m_clusterSvc->numRanks());

  // Entering event loop
  m_clusterSvc->barrier();
  // Note: no ++evt. This is because this is really a message loop, and we don't
  // want to increment evt if we haven't provided an event
  auto start = Clock::now();
  for (int evt = skipEvts; evt < skipEvts + maxEvt;) {
    ClusterMessage msg = m_clusterSvc->waitReceiveMessage();
    // Messages we can get are RequestEvent, FinalWorkerStatus, or WorkerError
    if (msg.messageType == ClusterMessageType::RequestEvent) {
      ATH_MSG_DEBUG("Starting event " << evt << " on " << msg.source);
      m_clusterSvc->sendMessage(
          msg.source, ClusterMessage(ClusterMessageType::ProvideEvent, evt));
      ++evt;
      continue;
    }

    if (msg.messageType == ClusterMessageType::WorkerError) {
      ATH_MSG_ERROR("Received WorkerError message from " << msg.source);
      statuses.at(msg.source) = get<ClusterMessage::WorkerStatus>(msg.payload);
      workers_done.at(msg.source) =
          true;  // If a worker hits an error, it's done
      ++num_workers_done;
      for (int i = 1; i < m_clusterSvc->numRanks(); ++i) {
        if (!workers_done.at(i)) {
          // Tell workers that aren't done to emergency stop
          m_clusterSvc->sendMessage(
              i, ClusterMessage(ClusterMessageType::EmergencyStop));
          workers_done[i] = true;
          ++num_workers_done;
        }
      }
      break;
    }

    if (msg.messageType == ClusterMessageType::FinalWorkerStatus) {
      ATH_MSG_INFO("Received FinalWorkerStatus from " << msg.source);
      statuses.at(msg.source) = get<ClusterMessage::WorkerStatus>(msg.payload);
      workers_done.at(msg.source) = true;  // Worker hit end of stream
      ++num_workers_done;
      continue;
    }

    // Other message types are an error
    ATH_MSG_ERROR("Received unexpected message "
                  << std::format("{}", msg.messageType) << " from "
                  << msg.source);
  }
  auto all_provided = Clock::now() - start;
  ATH_MSG_INFO("Provided all events to workers, waiting for them to complete.");
  // Event loop done, tell remaining workers
  while (num_workers_done < m_clusterSvc->numRanks()) {
    ClusterMessage msg = m_clusterSvc->waitReceiveMessage();
    // Messages we can get are RequestEvent, FinalWorkerStatus, or WorkerError
    if (msg.messageType == ClusterMessageType::RequestEvent) {
      m_clusterSvc->sendMessage(msg.source,
                                ClusterMessage(ClusterMessageType::EventsDone));
      continue;
    }

    if (msg.messageType == ClusterMessageType::WorkerError) {
      ATH_MSG_ERROR("Received WorkerError message from " << msg.source);
      statuses.at(msg.source) = get<ClusterMessage::WorkerStatus>(msg.payload);
      workers_done.at(msg.source) =
          true;  // If a worker hits an error, it's done
      ++num_workers_done;
      for (int i = 1; i < m_clusterSvc->numRanks(); ++i) {
        if (!workers_done.at(i)) {
          // Tell workers that aren't done to emergency stop
          m_clusterSvc->sendMessage(
              i, ClusterMessage(ClusterMessageType::EmergencyStop));
          workers_done[i] = true;
          ++num_workers_done;
        }
      }
      break;
    }

    if (msg.messageType == ClusterMessageType::FinalWorkerStatus) {
      ATH_MSG_INFO("Received FinalWorkerStatus from " << msg.source);
      statuses.at(msg.source) = get<ClusterMessage::WorkerStatus>(msg.payload);
      workers_done.at(msg.source) = true;  // Told worker we're done
      ++num_workers_done;
      continue;
    }

    // Other message types are an error
    ATH_MSG_ERROR("Received unexpected message "
                  << std::format("{}", msg.messageType) << " from "
                  << msg.source);
  }
  auto all_done = Clock::now() - start;
  // Collate status
  int n_created = 0;
  int n_skipped = 0;
  int n_finished = 0;

  StatusCode sc = StatusCode::SUCCESS;
  int worker_idx = 0;
  for (const auto& worker_status : statuses) {
    if (worker_status.status.isFailure() &&
        worker_status.status != StatusCode(9999)) {
      sc = worker_status.status;
    }
    n_created += worker_status.createdEvents;
    n_skipped += worker_status.skippedEvents;
    n_finished += worker_status.finishedEvents;

    if ((worker_idx++) != 0) {
      ATH_MSG_INFO("Worker " << worker_idx << ": SC " << worker_status.status
                             << ", created " << worker_status.createdEvents
                             << ", skipped " << worker_status.skippedEvents
                             << ", finished " << worker_status.finishedEvents);
    }
  }

  ATH_MSG_INFO("Overall: SC " << sc << ", created " << n_created << ", skipped "
                              << n_skipped << ", finished " << n_finished);
  ATH_MSG_INFO("MASTER: Took " << std::chrono::hh_mm_ss(all_provided)
                               << " to provide all events.");
  ATH_MSG_INFO("MASTER: Took " << std::chrono::hh_mm_ss(all_done)
                               << " to complete all events.");
  return sc;
}

/// Worker event loop
StatusCode MPIHiveEventLoopMgr::workerEventLoop() {
  bool end_of_stream = false;
  // barrier so all ranks enter message loop together
  m_clusterSvc->barrier();
  auto start = Clock::now();
  m_evtSelectorCurrentPos = int(m_firstEventIndex.value()) - 1;
  while (true) {
    // Drain the scheduler (wait for at least one event to complete, then free
    // up completed slots) in two circumstances
    // 1. Have created exactly one event, so the first event runs to completion
    // before any more are scheduled
    // 2. There are no free slots left
    bool haveFreeSlots =
        m_schedulerSvc->freeSlots() > 0 && m_whiteboard->freeSlots() > 0;
    if (!haveFreeSlots || m_nLocalCreatedEvts == 1) {
      StatusCode sc = drainLocalScheduler();
      if (sc.isFailure()) {
        ClusterMessage::WorkerStatus status{};
        status.status = sc;
        status.createdEvents = m_nLocalCreatedEvts;
        status.skippedEvents = m_nLocalSkippedEvts;
        status.finishedEvents = m_nLocalFinishedEvts;
        m_clusterSvc->sendMessage(
            0, ClusterMessage(ClusterMessageType::WorkerError, status));
        return sc;
      }
    }

    auto start_time = Clock::now();
    m_clusterSvc->sendMessage(0,
                              ClusterMessage(ClusterMessageType::RequestEvent));
    ClusterMessage msg = m_clusterSvc->waitReceiveMessage();
    auto request_time = Clock::now() - start_time;
    if (msg.messageType == ClusterMessageType::EmergencyStop) {
      // Emergency stop, return FAILURE
      ATH_MSG_ERROR("Received EmergencyStop message!");
      return StatusCode::FAILURE;
    }

    if (msg.messageType == ClusterMessageType::EventsDone) {
      auto loop_time = Clock::now() - start;
      ATH_MSG_INFO("Worker " << m_clusterSvc->rank() << " DONE. Loop took "
                             << std::chrono::hh_mm_ss(loop_time)
                             << " to process " << m_nLocalCreatedEvts
                             << " events.");
      // Been told we've reached end
      // Provide status to master
      ClusterMessage::WorkerStatus status{};
      // At end of stream, we need to *fully* drain the scheduler
      StatusCode sc = StatusCode::SUCCESS;
      std::size_t numSlots = m_whiteboard->getNumberOfStores();
      while (sc.isSuccess() && m_schedulerSvc->freeSlots() < numSlots) {
        sc = drainLocalScheduler();
      }
      status.status = sc;
      status.createdEvents = m_nLocalCreatedEvts;
      status.skippedEvents = m_nLocalSkippedEvts;
      status.finishedEvents = m_nLocalFinishedEvts;
      m_clusterSvc->sendMessage(
          0, ClusterMessage(ClusterMessageType::FinalWorkerStatus, status));
      return sc;
    }

    // Any other message other than ProvideEvent would now be an error
    if (msg.messageType != ClusterMessageType::ProvideEvent ||
        msg.source != 0) {
      ATH_MSG_ERROR("Received unexpected message "
                    << std::format("{}", msg.messageType) << " from "
                    << msg.source);
      return StatusCode::FAILURE;
    }

    int evt = get<int>(msg.payload);
    ATH_MSG_INFO("Starting event " << evt);
    StatusCode sc = insertEvent(
        evt, end_of_stream,
        std::chrono::duration_cast<std::chrono::nanoseconds>(request_time)
            .count());
    if (sc.isFailure() && !sc.isRecoverable()) {
      ClusterMessage::WorkerStatus status{};
      status.status = sc;
      status.createdEvents = m_nLocalCreatedEvts;
      status.skippedEvents = m_nLocalSkippedEvts;
      status.finishedEvents = m_nLocalFinishedEvts;
      m_clusterSvc->sendMessage(
          0, ClusterMessage(ClusterMessageType::WorkerError, status));
      return sc;
    }
    if (end_of_stream || m_terminateLoop) {
      auto loop_time = Clock::now() - start;
      ATH_MSG_INFO("Worker " << m_clusterSvc->rank() << " DONE. Loop took "
                             << std::chrono::hh_mm_ss(loop_time)
                             << " to process " << m_nLocalCreatedEvts
                             << " events.");
      // reached end of stream, drain scheduler
      ClusterMessage::WorkerStatus status{};
      // At end of stream, we need to *fully* drain the scheduler
      StatusCode sc = StatusCode::SUCCESS;
      std::size_t numSlots = m_whiteboard->getNumberOfStores();
      while (sc.isSuccess() && m_schedulerSvc->freeSlots() < numSlots) {
        sc = drainLocalScheduler();
      }
      status.status = sc;
      status.createdEvents = m_nLocalCreatedEvts;
      status.skippedEvents = m_nLocalSkippedEvts;
      status.finishedEvents = m_nLocalFinishedEvts;
      m_clusterSvc->sendMessage(
          0, ClusterMessage(ClusterMessageType::FinalWorkerStatus, status));
      return sc;
    }
  }
}

/// Insert event into local scheduler
StatusCode MPIHiveEventLoopMgr::insertEvent(int eventIdx, bool& endOfStream,
                                            std::int64_t requestTime_ns) {
  // fast-forward to event
  // Create the event context now so next writes into the next slot when
  // skipping, not the one that's being used
  endOfStream = false;
  auto ctx = createEventContext();
  Gaudi::Hive::setCurrentContext(ctx);
  if (!ctx.valid()) {
    endOfStream = true;  // BUG: Doesn't actually mean end of stream. Remove
                         // after making sure!
    return StatusCode::FAILURE;
  }

  ATH_CHECK(seek(eventIdx));
  // execute event
  StatusCode sc = executeEvent(std::move(ctx));
  const auto evtID = m_lastEventContext.eventID();
  m_clusterSvc->log_addEvent(eventIdx, evtID.run_number(), evtID.event_number(),
                             requestTime_ns);

  if (sc.isRecoverable()) {
    ++m_nLocalSkippedEvts;
  } else if (sc.isSuccess()) {
    ++m_nLocalCreatedEvts;
  }
  return sc;
}

/// drainLocalScheduler
/// Drain the local scheduler on this MPI rank
StatusCode MPIHiveEventLoopMgr::drainLocalScheduler() {

  StatusCode sc(StatusCode::SUCCESS);

  // maybe we can do better
  std::vector<std::unique_ptr<EventContext>> finishedEvtContexts;

  EventContext* finishedEvtContext(nullptr);

  // Here we wait not to loose cpu resources
  ATH_MSG_DEBUG("drainLocalScheduler: [" << m_nLocalFinishedEvts
                                         << "] Waiting for a context");
  sc = m_schedulerSvc->popFinishedEvent(finishedEvtContext);

  // We got past it: cache the pointer
  if (sc.isSuccess()) {
    ATH_MSG_DEBUG("drainLocalScheduler: scheduler not empty: Context "
                  << finishedEvtContext);
    finishedEvtContexts.emplace_back(finishedEvtContext);
  } else {
    // no more events left in scheduler to be drained
    ATH_MSG_DEBUG("drainLocalScheduler: scheduler empty");
    return StatusCode::SUCCESS;
  }

  // Let's see if we can pop other event contexts
  while (m_schedulerSvc->tryPopFinishedEvent(finishedEvtContext).isSuccess()) {
    finishedEvtContexts.emplace_back(finishedEvtContext);
  }

  // Now we flush them
  StatusCode fail(StatusCode::SUCCESS);
  for (auto& thisFinishedEvtContext : finishedEvtContexts) {
    if (!thisFinishedEvtContext) {
      ATH_MSG_FATAL("Detected nullptr ctxt while clearing WB!");
      fail = StatusCode::FAILURE;
      continue;
    }

    // Update event log
    m_clusterSvc->log_completeEvent(
        thisFinishedEvtContext->eventID().run_number(),
        thisFinishedEvtContext->eventID().event_number(),
        m_aess->eventStatus(*thisFinishedEvtContext));

    if (m_aess->eventStatus(*thisFinishedEvtContext) != EventStatus::Success) {
      ATH_MSG_FATAL("Failed event detected on "
                    << thisFinishedEvtContext << " w/ fail mode: "
                    << m_aess->eventStatus(*thisFinishedEvtContext));
      thisFinishedEvtContext.reset();
      fail = StatusCode::FAILURE;
      continue;
    }

    EventID::number_type n_run(0);
    EventID::event_number_t n_evt(0);

    if (m_whiteboard->selectStore(thisFinishedEvtContext->slot()).isSuccess()) {
      n_run = thisFinishedEvtContext->eventID().run_number();
      n_evt = thisFinishedEvtContext->eventID().event_number();
    } else {
      ATH_MSG_ERROR("DrainSched: unable to select store "
                    << thisFinishedEvtContext->slot());
      thisFinishedEvtContext.reset();
      fail = StatusCode::FAILURE;
      continue;
    }

    // Some code still needs global context in addition to that passed in the
    // incident
    Gaudi::Hive::setCurrentContext(*thisFinishedEvtContext);
    m_incidentSvc->fireIncident(
        Incident(name(), IncidentType::EndProcessing, *thisFinishedEvtContext));

    ATH_MSG_DEBUG("Clearing slot "
                  << thisFinishedEvtContext->slot() << " (event "
                  << thisFinishedEvtContext->evt() << ") of the whiteboard");

    StatusCode sc = clearWBSlot(thisFinishedEvtContext->slot());
    if (!sc.isSuccess()) {
      ATH_MSG_ERROR("Whiteboard slot " << thisFinishedEvtContext->slot()
                                       << " could not be properly cleared");
      if (fail != StatusCode::FAILURE) {
        fail = sc;
      }
      thisFinishedEvtContext.reset();
      continue;
    }

    ++m_nLocalFinishedEvts;

    writeHistograms().ignore();
    ++m_proc;

    if (m_doEvtHeartbeat) {
      if (!m_useTools) {
        ATH_MSG_INFO("  ===>>>  done processing event #"
                     << n_evt << ", run #" << n_run << " on slot "
                     << thisFinishedEvtContext->slot() << ",  " << m_proc
                     << " events processed so far <<<===");
      } else {
        ATH_MSG_INFO("  ===>>>  done processing event #"
                     << n_evt << ", run #" << n_run << " on slot "
                     << thisFinishedEvtContext->slot() << ",  " << m_nev
                     << " events read and " << m_proc
                     << " events processed so far <<<===");
      }
      std::ofstream outfile("eventLoopHeartBeat.txt");
      if (!outfile) {
        ATH_MSG_ERROR(" unable to open: eventLoopHeartBeat.txt");
        fail = StatusCode::FAILURE;
        thisFinishedEvtContext.reset();
        continue;
      }
      outfile << "  done processing event #" << n_evt << ", run #" << n_run
              << " " << m_nev << " events read so far <<<===" << std::endl;
      outfile.close();
    }

    ATH_MSG_DEBUG("drainLocalScheduler thisFinishedEvtContext: "
                  << thisFinishedEvtContext);

    thisFinishedEvtContext.reset();
  }

  return fail;
}

inline StoreGateSvc* MPIHiveEventLoopMgr::eventStore() const {
  return m_eventStore.get();
}
