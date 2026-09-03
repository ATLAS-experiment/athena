#include "AthenaRemoteEventLoopMgr.h"

#include <AthenaKernel/EventContextClid.h>
#include <AthenaKernel/ExtendedEventContext.h>
#include <EventInfo/EventID.h>
#include <EventInfo/EventInfo.h>
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/ITimelineSvc.h>

AthenaRemoteEventLoopMgr::AthenaRemoteEventLoopMgr(const std::string& nam,
                                                   ISvcLocator* svcLoc)
    : base_class(nam, svcLoc),
      AthMessaging(nam),
      m_incidentSvc("IncidentSvc", nam),
      m_eventStore("StoreGateSvc", nam),
      m_tools(this),
      m_firstRun(true),
      m_nevt(0),
      m_useTools(false) {
  declareProperty("EventStore", m_eventStore);

  declareProperty("PreSelectTools", m_tools, "AlgTools for event pre-selection")
      ->declareUpdateHandler(&AthenaRemoteEventLoopMgr::setupPreSelectTools,
                             this);

  declareProperty("WhiteboardSvc", m_whiteboardName = "EventDataSvc",
                  "Name of the Whiteboard to be used");

  declareProperty("SchedulerSvc", m_schedulerName = "ForwardSchedulerSvc",
                  "Name of the scheduler to be used");
}

StatusCode AthenaRemoteEventLoopMgr::initialize() {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::initialize()");

  StatusCode sc = MinimalEventLoopMgr::initialize();
  if (!sc.isSuccess()) {
    ATH_MSG_ERROR("Failed to initialize base class MinimalEventLoopMgr");
    return sc;
  }

  sc = m_eventStore.retrieve();
  if (!sc.isSuccess()) {
    ATH_MSG_FATAL("Error retrieving pointer to StoreGateSvc");
    return sc;
  }

  sc = m_incidentSvc.retrieve();
  if (!sc.isSuccess()) {
    ATH_MSG_FATAL("Error retrieving IncidentSvc.");
    return sc;
  }

  m_whiteboard = serviceLocator()->service(m_whiteboardName);
  if (!m_whiteboard.isValid()) {
    ATH_MSG_FATAL("Error retrieving WhiteboardSvc interface IHiveWhiteBoard.");
    return StatusCode::FAILURE;
  }

  m_algResourcePool = serviceLocator()->service("AlgResourcePool");
  if (!m_algResourcePool.isValid()) {
    ATH_MSG_FATAL("Error retrieving AlgResourcePool");
    return StatusCode::FAILURE;
  }

  m_aess = serviceLocator()->service("AlgExecStateSvc");
  if (!m_aess.isValid()) {
    ATH_MSG_FATAL("Error retrieving AlgExecStateSvc");
    return StatusCode::FAILURE;
  }

  m_schedulerSvc = serviceLocator()->service(m_schedulerName);
  if (!m_schedulerSvc.isValid()) {
    ATH_MSG_FATAL("Error retrieving SchedulerSvc interface ISchedulerSvc.");
    return StatusCode::FAILURE;
  }

  // Listen to the BeforeFork and EndAlgorithms incidents
  m_incidentSvc->addListener(this, "BeforeFork", 0);
  m_incidentSvc->addListener(this, "EndAlgorithms", 0);

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::initialize()");
  return StatusCode::SUCCESS;
}

// StatusCode AthenaRemoteEventLoopMgr::start() {
//   ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::start()");
//
//   return StatusCode::SUCCESS;
// }

StatusCode AthenaRemoteEventLoopMgr::stop() {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::stop()");

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::stop()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::finalize() {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::finalize()");

  StatusCode sc = MinimalEventLoopMgr::finalize();
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Error in Service base class Finalize");
  }

  m_whiteboard = 0;
  m_algResourcePool = 0;
  m_schedulerSvc = 0;

  m_incidentSvc.release().ignore();

  if (m_useTools) {
    tool_iterator firstTool = m_tools.begin();
    tool_iterator lastTool = m_tools.end();
    unsigned int toolCtr = 0;
    ATH_MSG_INFO(
        "Summary of AthenaEvtLoopPreSelectTool invocation: "
        "(invoked/success/failure)");
    ATH_MSG_INFO("-----------------------------------------------------");

    for (; firstTool != lastTool; ++firstTool) {
      ATH_MSG_INFO(std::setw(2)
                   << std::setiosflags(std::ios_base::right) << toolCtr + 1
                   << ".) " << std::resetiosflags(std::ios_base::right)
                   << std::setw(48) << std::setfill('.')
                   << std::setiosflags(std::ios_base::left)
                   << (*firstTool)->name()
                   << std::resetiosflags(std::ios_base::left)
                   << std::setfill(' ') << " (" << std::setw(6)
                   << std::setiosflags(std::ios_base::right)
                   << m_toolInvoke[toolCtr] << "/" << m_toolAccept[toolCtr]
                   << "/" << m_toolReject[toolCtr] << ")");
      toolCtr++;
    }
  }

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::finalize()");
  return sc;
}

void AthenaRemoteEventLoopMgr::setupPreSelectTools(
    Gaudi::Details::PropertyBase&) {

  m_toolInvoke.clear();
  m_toolReject.clear();
  m_toolAccept.clear();

  m_tools.retrieve().ignore();
  if (m_tools.size() > 0) {
    m_useTools = true;
    m_toolInvoke.resize(m_tools.size());
    m_toolReject.resize(m_tools.size());
    m_toolAccept.resize(m_tools.size());

    tool_iterator firstTool = m_tools.begin();
    tool_iterator lastTool = m_tools.end();
    unsigned int toolCtr = 0;
    for (; firstTool != lastTool; ++firstTool) {
      // reset statistics
      m_toolInvoke[toolCtr] = 0;
      m_toolReject[toolCtr] = 0;
      m_toolAccept[toolCtr] = 0;
      toolCtr++;
    }
  }

  return;
}

// StatusCode AthenaRemoteEventLoopMgr::reinitialize() {
//   ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::reinitialize()");
//
//   return StatusCode::SUCCESS;
// }

// StatusCode AthenaRemoteEventLoopMgr::restart() {
//   ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::restart()");
//
//   return StatusCode::SUCCESS;
// }

EventContext AthenaRemoteEventLoopMgr::createEventContext() {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::createEventContext()");

  EventContext ctx{m_nevt, m_whiteboard->allocateStore(m_nevt)};

  StatusCode sc = m_whiteboard->selectStore(ctx.slot());
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Slot " << ctx.slot()
                          << " could not be selected for the WhiteBoard");
    return EventContext{};  // invalid EventContext
  } else {
    Atlas::setExtendedEventContext(
        ctx, Atlas::ExtendedEventContext(m_eventStore->hiveProxyDict()));

    ATH_MSG_DEBUG("created EventContext, num: " << ctx.evt()
                                                << "  in slot: " << ctx.slot());
  }

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::createEventContext()");
  return ctx;
}

StatusCode AthenaRemoteEventLoopMgr::nextEvent([[maybe_unused]] int maxevt) {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::nextEvent()");

  Gaudi::setAppReturnCode(m_appMgrProperty, Gaudi::ReturnCode::Success, true)
      .ignore();

  StatusCode sc(StatusCode::SUCCESS);

  ATH_MSG_INFO("Waiting for events from clients...");
  while (sc.isSuccess()) {
    ATH_MSG_DEBUG("Checking for any finished events...");
    std::vector<std::unique_ptr<EventContext>> finishedEvtContexts;
    EventContext* finishedEvtContext{nullptr};

    if (m_schedulerSvc->popFinishedEvent(finishedEvtContext).isSuccess()) {
      ATH_MSG_DEBUG("drainScheduler: scheduler not empty: Context "
                    << finishedEvtContext);
      finishedEvtContexts.emplace_back(finishedEvtContext);

      // Let's see if we can pop other event contexts
      while (
          m_schedulerSvc->tryPopFinishedEvent(finishedEvtContext).isSuccess()) {
        finishedEvtContexts.emplace_back(finishedEvtContext);
      }

      for (auto& thisFinishedEvtContext : finishedEvtContexts) {
        if (!thisFinishedEvtContext) {
          ATH_MSG_FATAL("Detected nullptr ctxt while clearing WB!");
          sc = StatusCode::FAILURE;
          continue;
        }

        if (m_aess->eventStatus(*thisFinishedEvtContext) !=
            EventStatus::Success) {
          ATH_MSG_FATAL("Failed event detected on "
                        << thisFinishedEvtContext << " w/ fail mode: "
                        << m_aess->eventStatus(*thisFinishedEvtContext));
          sc = StatusCode::FAILURE;
          continue;
        }

        EventID::number_type n_run(0);
        EventID::event_number_t n_evt(0);

        if (m_whiteboard->selectStore(thisFinishedEvtContext->slot())
                .isSuccess()) {
          n_run = thisFinishedEvtContext->eventID().run_number();
          n_evt = thisFinishedEvtContext->eventID().event_number();
        } else {
          ATH_MSG_ERROR("DrainSched: unable to select store "
                        << thisFinishedEvtContext->slot());
          sc = StatusCode::FAILURE;
          continue;
        }

        // Some code still needs global context in addition to that passed in
        // the incident
        Gaudi::Hive::setCurrentContext(*thisFinishedEvtContext);
        m_incidentSvc->fireIncident(Incident(
            name(), IncidentType::EndProcessing, *thisFinishedEvtContext));

        ATH_MSG_DEBUG("Clearing slot " << thisFinishedEvtContext->slot()
                                       << " (event "
                                       << thisFinishedEvtContext->evt()
                                       << ") of the whiteboard");
        m_eventExecutionTool->completeEvent(this, *thisFinishedEvtContext)
            .ignore();
        if (!m_whiteboard->freeStore(thisFinishedEvtContext->slot())
                 .isSuccess()) {
          ATH_MSG_ERROR("Whiteboard slot " << thisFinishedEvtContext->slot()
                                           << " could not be properly cleared");
          sc = StatusCode::FAILURE;
          continue;
        }

        m_processed++;
        ATH_MSG_INFO("  ===>>>  done processing event #"
                     << n_evt << ", run #" << n_run << " on slot "
                     << thisFinishedEvtContext->slot() << ",  " << m_processed
                     << " events processed so far  <<<===");

        ATH_MSG_DEBUG("drainScheduler thisFinishedEvtContext: "
                      << thisFinishedEvtContext);
      }
    } else {
      // no more events left in scheduler to be drained
      ATH_MSG_DEBUG("drainScheduler: scheduler empty");
    }

    ATH_MSG_DEBUG("Free slots: " << m_schedulerSvc->freeSlots());
    while (m_schedulerSvc->freeSlots() > 0) {
      ATH_MSG_DEBUG("Got free slots, adding events to scheduler");

      auto ctx = createEventContext();

      if (!ctx.valid()) {
        sc = StatusCode::FAILURE;
      } else {
        int slot = ctx.slot();
        m_whiteboard->selectStore(ctx.slot()).ignore();

        // CHECK: Needed?
        // m_incidentSvc->fireIncident(
        //     Incident("BeginEvent", IncidentType::BeginEvent));

        // CHECK: Put this also here, not only in ::executeEvent so unpacking
        // tools called by m_eventExecutionTool->executeEvent() put everything
        // in the correct context?
        Gaudi::Hive::setCurrentContext(ctx);

        ATH_MSG_DEBUG("Entering m_eventExecutionTool::executeEvent()...");
        StatusCode scExecuteEvent =
            m_eventExecutionTool->executeEvent(this, std::move(ctx));
        sc = scExecuteEvent.isSuccess() or scExecuteEvent.isRecoverable()
                 ? StatusCode::SUCCESS
                 : StatusCode::FAILURE;

        // Timeout from gRPC
        if (scExecuteEvent.isRecoverable()) {
          if (!m_whiteboard->freeStore(slot).isSuccess()) {
            ATH_MSG_ERROR("Whiteboard slot "
                          << slot << " could not be properly cleared");
            sc = StatusCode::FAILURE;
          }
          break;
        }

        // CHECK: Needed?
        // m_incidentSvc->fireIncident(
        //     Incident("EndEvent", IncidentType::EndEvent));
      }

      if (!sc.isSuccess()) {
        ATH_MSG_ERROR(
            "Terminating event processing loop due to errors in "
            "executeEvent");
        break;
      }
    }
  }

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::nextEvent()");
  return sc;
}

StatusCode AthenaRemoteEventLoopMgr::initializeAlgorithms() {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::initializeAlgorithms()");

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::initializeAlgorithms()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::executeAlgorithms() {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::executeAlgorithms()");

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::executeAlgorithms()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::executeEvent(EventContext&& ctx) {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::executeEvent()");

  m_aess->reset(ctx);

  Gaudi::Hive::setCurrentContext(ctx);

  // From AthenaHiveEventLoopMgr::declareEventRootAddress()
  // CHECK: Is this needed?
  if (m_eventStore->loadEventProxies().isFailure()) {
    ATH_MSG_ERROR("Error loading Event proxies");
    return StatusCode::FAILURE;
  }

  if (m_eventStore->record(std::make_unique<EventContext>(ctx), "EventContext")
          .isFailure()) {
    ATH_MSG_ERROR("Error recording event context object");
    return StatusCode::FAILURE;
  }

  EventID::event_number_t evtNumber = ctx.eventID().event_number();
  unsigned int runNumber = ctx.eventID().run_number();

  if (m_firstRun) {
    m_firstRun = false;
    ATH_MSG_INFO("  ===>>>  start processing events from run " << runNumber
                                                               << "  <<<===");
    m_incidentSvc->fireIncident(Incident(name(), IncidentType::BeginRun, ctx));
  }

  bool toolsPassed = true;
  if (m_useTools) {
    std::size_t toolCtr = 0;
    tool_store::iterator theTool = m_tools.begin();
    tool_store::iterator lastTool = m_tools.end();
    while (toolsPassed && theTool != lastTool) {
      toolsPassed = (*theTool)->passEvent(ctx.eventID());
      m_toolInvoke[toolCtr]++;
      toolsPassed ? m_toolAccept[toolCtr]++ : m_toolReject[toolCtr]++;
      ++toolCtr;
      ++theTool;
    }
  }

  if (toolsPassed) {
    ATH_MSG_DEBUG("Adding event " << ctx.evt() << ", nr "
                                  << ctx.eventID().event_number() << ", slot "
                                  << ctx.slot() << " to the scheduler");

    m_incidentSvc->fireIncident(
        Incident(name(), IncidentType::BeginProcessing, ctx));
    if (!m_schedulerSvc->pushNewEvent(new EventContext{std::move(ctx)})
             .isSuccess()) {
      ATH_MSG_ERROR("Error while pushing event to scheduler");
      return StatusCode::FAILURE;
    }
  }

  ATH_MSG_INFO("  ===>>>  start processing event #"
               << evtNumber << ", run #" << runNumber << " on slot "
               << ctx.slot() << ",  " << m_processed
               << " events processed so far  <<<===");

  ++m_nevt;

  // invalidate thread local context once outside of event execute loop
  Gaudi::Hive::setCurrentContext(EventContext());

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::executeEvent()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::executeRun(int maxevt) {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::executeRun()");

  if (nextEvent(maxevt).isFailure()) {
    return StatusCode::FAILURE;
  }

  m_incidentSvc->fireIncident(Incident(name(), "EndEvtLoop"));

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::executeRun()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::stopRun() {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::stopRun()");

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::stopRun()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::seek([[maybe_unused]] int evt) {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::seek()");

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::seek()");
  return StatusCode::SUCCESS;
}

int AthenaRemoteEventLoopMgr::curEvent() const {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::curEvent()");

  return m_nevt;
}

int AthenaRemoteEventLoopMgr::size() {
  ATH_MSG_VERBOSE("In AthenaRemoteEventLoopMgr::size()");

  return 0;
}

void AthenaRemoteEventLoopMgr::handle(const Incident& inc) {
  ATH_MSG_VERBOSE(
      "In AthenaRemoteEventLoopMgr::handle() for incident: " << inc.type());

  if (inc.type() == "EndAlgorithms") {
    ATH_MSG_DEBUG("Clearing event storage for slot " << inc.context().slot());
    if (!m_whiteboard->clearStore(inc.context().slot()).isSuccess()) {
      ATH_MSG_WARNING("Clear of Event data store failed");
    }
  } else {
    ATH_MSG_ERROR("Unhandled incident type! " << inc.type());
  }

  ATH_MSG_VERBOSE("Leaving AthenaRemoteEventLoopMgr::handle() for incident: "
                  << inc.type());
}
