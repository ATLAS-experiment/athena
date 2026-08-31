#include "AthenaRemoteEventLoopMgr.h"

#include <AthenaBaseComps/AthCheckMacros.h>
#include <AthenaKernel/ExtendedEventContext.h>
#include <EventInfo/EventID.h>
#include <EventInfo/EventInfo.h>

AthenaRemoteEventLoopMgr::AthenaRemoteEventLoopMgr(const std::string& nam,
                                                   ISvcLocator* svcLoc)
    : base_class(nam, svcLoc),
      AthMessaging(nam),
      m_incidentSvc("IncidentSvc", nam),
      // m_eventStore("StoreGateSvc", nam),
      m_tools(this),
      m_firstRun(true),
      m_nevt(0),
      m_useTools(false) {
  // declareProperty("EventStore", m_eventStore);
  declareProperty("PreSelectTools", m_tools, "AlgTools for event pre-selection")
      ->declareUpdateHandler(&AthenaRemoteEventLoopMgr::setupPreSelectTools,
                             this);
}

StatusCode AthenaRemoteEventLoopMgr::initialize() {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::initialize()");

  StatusCode sc = MinimalEventLoopMgr::initialize();
  if (!sc.isSuccess()) {
    ATH_MSG_ERROR("Failed to initialize base class MinimalEventLoopMgr");
    return sc;
  }

  // sc = m_eventStore.retrieve();
  // if (!sc.isSuccess()) {
  //   ATH_MSG_FATAL("Error retrieving pointer to StoreGateSvc");
  //   return sc;
  // }

  // Listen to the BeforeFork and EndAlgorithms incidents
  m_incidentSvc->addListener(this, "BeforeFork", 0);
  m_incidentSvc->addListener(this, "EndAlgorithms", 0);

  ATH_CHECK(m_scheduler.retrieve());
  ATH_CHECK(m_algResourcePool.retrieve());
  ATH_CHECK(m_algExecState.retrieve());
  ATH_CHECK(m_eventStore.retrieve());
  ATH_CHECK(m_whiteBoard.retrieve());
  ATH_CHECK(m_eventStore.retrieve());

  ATH_MSG_INFO("Leaving AthenaRemoteEventLoopMgr::initialize()");
  return StatusCode::SUCCESS;
}

// StatusCode AthenaRemoteEventLoopMgr::start() {
//   ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::start()");
//
//   return StatusCode::SUCCESS;
// }

StatusCode AthenaRemoteEventLoopMgr::stop() {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::stop()");

  ATH_MSG_INFO("Leaving AthenaRemoteEventLoopMgr::stop()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::finalize() {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::finalize()");

  ATH_MSG_INFO("Leaving AthenaRemoteEventLoopMgr::finalize()");
  return StatusCode::SUCCESS;
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
//   ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::reinitialize()");
//
//   return StatusCode::SUCCESS;
// }

// StatusCode AthenaRemoteEventLoopMgr::restart() {
//   ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::restart()");
//
//   return StatusCode::SUCCESS;
// }

EventContext AthenaRemoteEventLoopMgr::createEventContext() {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::createEventContext()");

  return EventContext{m_nevt++, 0};
}

StatusCode AthenaRemoteEventLoopMgr::nextEvent([[maybe_unused]] int maxevt) {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::nextEvent()");

  StatusCode sc(StatusCode::SUCCESS);

  sc = initializeAlgorithms();
  if (!sc.isSuccess()) {
    return StatusCode::FAILURE;
  }

  // sc = m_eventStore->clearStore();
  // if (!sc.isSuccess()) {
  //   return StatusCode::FAILURE;
  // }

  ATH_MSG_INFO("Waiting for events from clients...");
  while (sc.isSuccess()) {
    auto ctx = createEventContext();

    if (!ctx.valid()) {
      sc = StatusCode::FAILURE;
    } else {
      // FIXME: Extra one here, so logs show correct slot/event numbers
      Gaudi::Hive::setCurrentContext(ctx);

      sc = m_eventStore->clearStore();
      if (!sc.isSuccess()) {
        ATH_MSG_ERROR("Unable to clear event store. Terminating loop.");
        break;
      }

      m_incidentSvc->fireIncident(
          Incident("BeginEvent", IncidentType::BeginEvent));

      ATH_MSG_INFO("Entering m_eventExecutionTool::executeEvent()...");
      sc = m_eventExecutionTool->executeEvent(this, std::move(ctx));

      m_incidentSvc->fireIncident(Incident("EndEvent", IncidentType::EndEvent));
    }

    if (!sc.isSuccess()) {
      ATH_MSG_ERROR(
          "Terminating event processing loop due to errors in executeEvent");
      break;
    }

    ATH_MSG_INFO("Processesed " << m_nevt << " event(s) remotely");
    sc = m_eventExecutionTool->completeEvent(this, std::move(ctx));

    if (!sc.isSuccess()) {
      ATH_MSG_ERROR(
          "Terminating event processing loop due to errors in completeEvent");
      break;
    }
  }

  ATH_MSG_INFO("Leaving AthenaRemoteEventLoopMgr::nextEvent()");
  return sc;
}

StatusCode AthenaRemoteEventLoopMgr::initializeAlgorithms() {
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::executeEvent(EventContext&& ctx) {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::executeEvent()");

  Gaudi::Hive::setCurrentContext(ctx);

  if (m_eventStore->loadEventProxies().isFailure()) {
    ATH_MSG_ERROR("Error loading Event proxies");
    return StatusCode::FAILURE;
  }

  EventID::event_number_t evtNumber = ctx.eventID().event_number();
  unsigned int conditionsRun = ctx.eventID().run_number();

  if (m_firstRun) {
    m_firstRun = false;
    ATH_MSG_INFO("  ===>>>  start processing events from run " << conditionsRun
                                                               << "  <<<===");
    // FIXME: Crashes on this, but is this needed at all?
    // m_incidentSvc->fireIncident(Incident(name(), IncidentType::BeginRun,
    // ctx));
  }

  if (m_useTools) {
    bool toolsPassed = true;
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

  ATH_MSG_INFO("  ===>>>  start processing event #"
               << evtNumber << ", run #" << conditionsRun << " on slot "
               << ctx.slot() << ",  " << m_nevt
               << " events processed so far  <<<===");

  m_incidentSvc->fireIncident(
      Incident(name(), IncidentType::BeginProcessing, ctx));
  StatusCode addEventStatus =
      m_scheduler->pushNewEvent(new EventContext{std::move(ctx)});

  // If this fails, we need to wait for something to complete
  if (!addEventStatus.isSuccess()) {
    ATH_MSG_FATAL(
        "An event processing slot should be now free in the scheduler, but "
        "it appears not to be the case.");
        return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Leaving AthenaRemoteEventLoopMgr::executeEvent()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::executeRun(int maxevt) {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::executeRun()");

  if (nextEvent(maxevt).isFailure()) {
    return StatusCode::FAILURE;
  }

  m_incidentSvc->fireIncident(Incident(name(), "EndEvtLoop"));

  ATH_MSG_INFO("Leaving AthenaRemoteEventLoopMgr::executeRun()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::stopRun() {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::stopRun()");

  ATH_MSG_INFO("Leaving AthenaRemoteEventLoopMgr::stopRun()");
  return StatusCode::SUCCESS;
}

StatusCode AthenaRemoteEventLoopMgr::seek([[maybe_unused]] int evt) {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::seek()");

  ATH_MSG_INFO("Leaving AthenaRemoteEventLoopMgr::seek()");
  return StatusCode::SUCCESS;
}

int AthenaRemoteEventLoopMgr::curEvent() const {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::curEvent()");

  return m_nevt;
}

int AthenaRemoteEventLoopMgr::size() {
  ATH_MSG_INFO("In AthenaRemoteEventLoopMgr::size()");

  return 0;
}

void AthenaRemoteEventLoopMgr::handle(const Incident& inc) {
  ATH_MSG_INFO(
      "In AthenaRemoteEventLoopMgr::handle() for incident: " << inc.type());

  ATH_MSG_INFO("Leaving AthenaRemoteEventLoopMgr::handle() for incident: "
               << inc.type());
}
