/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecCore/RemoteExecHiveEventLoopMgr.h>
#include <AthRemoteExecCore/ReplyStaging.h>
#include <AthRemoteExecCore/RequestDescriptor.h>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <thread>
#include <utility>

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaKernel/EventContextClid.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "StoreGate/StoreGateSvc.h"

namespace AthRemoteExec {

RemoteExecHiveEventLoopMgr::RemoteExecHiveEventLoopMgr(const std::string& name,
                                                       ISvcLocator* svcLoc)
    : AthenaHiveEventLoopMgr(name, svcLoc) {
  // This driver supplies events directly; it never reads a batch selector.
  m_evtsel = "NONE";
  m_schedulerName = "AvalancheSchedulerSvc";
  declareProperty("EvtStore",
                  m_eventStore);  // Preserve the RemoteExec property name.
}

RemoteExecHiveEventLoopMgr::~RemoteExecHiveEventLoopMgr() = default;

StatusCode RemoteExecHiveEventLoopMgr::initialize() {
  ATH_CHECK(AthenaHiveEventLoopMgr::initialize());

  if (m_sequences.empty()) {
    ATH_MSG_ERROR(
        "No sequences configured; the server would have nothing to "
        "offer. Set the Sequences property.");
    return StatusCode::FAILURE;
  }

  const auto known = [this](const std::string& sequence) {
    return std::find(m_sequences.begin(), m_sequences.end(), sequence) !=
           m_sequences.end();
  };
  for (const auto& [sequence, keys] : m_outputKeys) {
    if (!known(sequence)) {
      ATH_MSG_ERROR("OutputKeys names sequence '{}', which is not in Sequences",
                    sequence);
      return StatusCode::FAILURE;
    }
  }
  for (const auto& [sequence, key] : m_replyKeys) {
    if (!known(sequence)) {
      ATH_MSG_ERROR("ReplyKeys names sequence '{}', which is not in Sequences",
                    sequence);
      return StatusCode::FAILURE;
    }
  }
  for (const auto& [sequence, keys] : m_outputKeys) {
    if (!keys.empty() && m_replyKeys.value().count(sequence) == 0) {
      ATH_MSG_ERROR("Sequence '{}' declares outputs but no entry in ReplyKeys",
                    sequence);
      return StatusCode::FAILURE;
    }
  }

  std::ostringstream menu;
  for (const std::string& sequence : m_sequences) {
    menu << (menu.tellp() == 0 ? "" : ", ") << sequence;
  }
  ATH_MSG_INFO("Serving {} sequence(s): {} over {} slot(s)", m_sequences.size(),
               menu.str(), m_whiteboard->getNumberOfStores());

  return StatusCode::SUCCESS;
}

StatusCode RemoteExecHiveEventLoopMgr::finalize() {
  // Normally already done by nextEvent. This covers the paths where the loop
  // never ran or threw, so the threads cannot outlive the services they use.
  stopHelpers();
  if (m_server) {
    m_server->shutdown();
    m_server.reset();
  }
  return AthenaHiveEventLoopMgr::finalize();
}

StatusCode RemoteExecHiveEventLoopMgr::stopRun() {
  ATH_MSG_INFO("Stop requested, draining the server");
  m_stopRequested = true;
  if (m_server) {
    m_server->shutdown();
  }
  m_scheduledStop = true;
  return StatusCode::SUCCESS;
}

StatusCode RemoteExecHiveEventLoopMgr::startServing() {
  // Started here rather than in initialize() so that threads survives any
  // future fork.
  m_server = std::make_unique<Server>();

  std::vector<SequenceInfo> menu;
  menu.reserve(m_sequences.size());
  for (const std::string& sequence : m_sequences) {
    SequenceInfo info;
    info.name = sequence;
    // Advertise key#encoding#schema, so a client can discover what server is
    // capable of receiving and replying. This is the same declaration the gate
    // and pack algorithms parse.
    const auto advertise = [](const std::vector<std::string>& specs,
                              std::vector<BoundaryInfo>& out) {
      for (const std::string& spec : specs) {
        const auto boundary = parseBoundary(spec);
        if (boundary.has_value()) {
          out.push_back(std::move(*boundary));
        }
      }
    };
    if (auto inputs = m_inputKeys.value().find(sequence);
        inputs != m_inputKeys.value().end()) {
      advertise(inputs->second, info.inputs);
    }
    if (auto outputs = m_outputKeys.value().find(sequence);
        outputs != m_outputKeys.value().end()) {
      advertise(outputs->second, info.outputs);
    }
    menu.push_back(std::move(info));
  }
  m_server->setMenu(std::move(menu));

  const int boundPort =
      m_server->start(m_address, m_port, m_queueLimit, m_maxMessageBytes,
                      [this](Server::Level level, const std::string& text) {
                        serverMessage(level, text);
                      });
  if (boundPort < 0) {
    ATH_MSG_ERROR("Could not start the gRPC server");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("gRPC server listening on {}:{}", m_address.value(), boundPort);

  if (!m_portFile.empty()) {
    const std::string temporary = m_portFile + ".tmp";
    {
      std::ofstream out(temporary);
      out << boundPort << '\n';
    }
    if (std::rename(temporary.c_str(), m_portFile.value().c_str()) != 0) {
      ATH_MSG_ERROR("Could not write the port file " << m_portFile.value());
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode RemoteExecHiveEventLoopMgr::nextEvent(int maxevt) {
  if (maxevt == 0) {
    return StatusCode::SUCCESS;
  }
  ATH_CHECK(startServing());

  const size_t limit = maxevt < 0 ? std::numeric_limits<size_t>::max()
                                  : static_cast<size_t>(maxevt);
  const size_t slots = m_whiteboard->getNumberOfStores();
  const std::chrono::milliseconds poll(m_pollInterval);
  size_t accepted = 0;

  ATH_MSG_INFO("Starting the request loop");
  std::optional<Clock::time_point> firstAccepted;
  startHelpers();

  const auto canAccept = [&] {
    return !m_stopRequested && accepted < limit && m_inFlight.size() < slots &&
           m_schedulerSvc->freeSlots() > 0;
  };

  while (true) {
    if (m_inFlight.empty() && (m_stopRequested || accepted >= limit)) {
      break;
    }

    bool progressed = false;

    while (canAccept()) {
      ExecuteRequest request;
      std::unique_ptr<Server::Job> job;
      const Take taken = takeRequest(request, job);
      if (taken == Take::None) {
        break;
      }
      if (taken == Take::Stopped) {
        m_stopRequested = true;
        progressed = true;
        break;
      }
      ++accepted;
      if (!firstAccepted) {
        firstAccepted = Clock::now();
      }
      ATH_CHECK(dispatch(std::move(request), std::move(job)));
      progressed = true;
    }

    // Complete everything the output thread has collected.
    for (const Finished& finished : takeFinished()) {
      ATH_CHECK(complete(finished));
      progressed = true;
    }

    expireDeadlines();

    if (progressed) {
      continue;
    }

    // Wait until we can accept new request or there are finished events OR
    // periodically check if events passed their deadline
    std::unique_lock<std::mutex> lock(m_pendingMutex);
    const auto nothingToDo = [&] {
      return (m_incoming.empty() || !canAccept()) && m_finishedEvents.empty();
    };
    if (nothingToDo()) {
      if (m_requestTimeout > 0.0) {
        m_pending.wait_for(lock, poll);
      } else {
        m_pending.wait(lock, [&] { return !nothingToDo(); });
      }
    }
  }

  stopHelpers();

  ATH_MSG_INFO("Request loop finished: {} request(s) accepted, {} event(s) run",
               accepted, m_completed);

  // Cakculate throughput
  if (firstAccepted && m_completed > 0) {
    const double seconds =
        std::chrono::duration<double>(Clock::now() - *firstAccepted).count();
    if (seconds > 0.0) {
      ATH_MSG_ALWAYS(
          "Throughput: {} request(s) in {} s = {} request/s over {} slot(s)",
          m_completed, seconds, (m_completed / seconds),
          m_whiteboard->getNumberOfStores());
    }
  }

  if (m_server) {
    if (m_reportTiming) {
      const std::string report = m_server->timingReport();
      if (!report.empty()) {
        ATH_MSG_ALWAYS('\n' << report);
      }
    }
    m_server->shutdown();
  }

  return StatusCode::SUCCESS;
}

void RemoteExecHiveEventLoopMgr::startHelpers() {
  {
    const std::lock_guard<std::mutex> lock(m_pendingMutex);
    m_helpersStopping = false;
    // One work spot per slot, so the loop never waits on the input thread
    m_incomingLimit = std::max<size_t>(1, m_whiteboard->getNumberOfStores());
    m_inputDone = (m_server == nullptr);
  }
  if (m_server && !m_inputThread.joinable()) {
    m_inputThread = std::thread([this] { inputLoop(); });
  }
  if (!m_outputThread.joinable()) {
    m_outputThread = std::thread([this] { outputLoop(); });
  }
}

void RemoteExecHiveEventLoopMgr::stopHelpers() {
  {
    const std::lock_guard<std::mutex> lock(m_pendingMutex);
    m_helpersStopping = true;
  }
  m_inFlightCv.notify_all();
  m_pending.notify_all();
  m_incomingRoom.notify_all();

  // Stop accepting new request first...
  if (m_inputThread.joinable()) {
    if (m_server) {
      m_server->stopAccepting();
    }
    m_inputThread.join();
  }

  // ...reply to client about all unfinished jobs...
  drainPending();

  // ... and stop replying
  if (m_server) {
    m_server->shutdown();
  }
  if (m_outputThread.joinable()) {
    m_outputThread.join();
  }
}

void RemoteExecHiveEventLoopMgr::drainPending() {
  std::deque<std::unique_ptr<Server::Job>> leftover;
  std::deque<Finished> uncollected;
  {
    const std::lock_guard<std::mutex> lock(m_pendingMutex);
    leftover.swap(m_incoming);
    uncollected.swap(m_finishedEvents);
  }

  for (const std::unique_ptr<Server::Job>& job : leftover) {
    // Reply to clients so they won't stay stuck waiting
    ATH_MSG_WARNING("Request {} ({}) was still queued when the loop stopped",
                    job->request.requestId, job->request.sequence);
    respondDirect(job->request, job, AthRemoteExec::Status::ShuttingDown,
                  "the server stopped before this request was served");
  }

  // Destroy finished events
  for (const Finished& finished : uncollected) {
    delete finished.ctx;
  }
}

void RemoteExecHiveEventLoopMgr::inputLoop() {
  std::unique_ptr<Server::Job> job;
  while (m_server->pop(job)) {
    {
      std::unique_lock<std::mutex> lock(m_pendingMutex);
      // Wait until there is space for a new job
      m_incomingRoom.wait(lock, [this] {
        return m_incoming.size() < m_incomingLimit || m_helpersStopping;
      });
      m_incoming.push_back(std::move(job));
    }
    m_pending.notify_one();
  }
  {
    const std::lock_guard<std::mutex> lock(m_pendingMutex);
    m_inputDone = true;
  }
  m_pending.notify_one();
}

void RemoteExecHiveEventLoopMgr::outputLoop() {
  while (true) {
    {
      std::unique_lock<std::mutex> lock(m_pendingMutex);
      m_inFlightCv.wait(lock, [this] {
        return m_awaitingCompletion > 0 || m_helpersStopping;
      });
      if (m_awaitingCompletion == 0) {
        return;
      }
    }

    // Wait until there is a finished job to collect
    EventContext* finished = nullptr;
    if (m_schedulerSvc->popFinishedEvent(finished).isFailure()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    const Clock::time_point at = Clock::now();

    {
      const std::lock_guard<std::mutex> lock(m_pendingMutex);
      --m_awaitingCompletion;
      m_finishedEvents.push_back(Finished{finished, at});
    }
    m_pending.notify_one();
  }
}

std::vector<RemoteExecHiveEventLoopMgr::Finished>
RemoteExecHiveEventLoopMgr::takeFinished() {
  std::vector<Finished> finished;
  const std::lock_guard<std::mutex> lock(m_pendingMutex);
  finished.assign(m_finishedEvents.begin(), m_finishedEvents.end());
  m_finishedEvents.clear();
  return finished;
}

RemoteExecHiveEventLoopMgr::Take RemoteExecHiveEventLoopMgr::takeRequest(
    ExecuteRequest& request, std::unique_ptr<Server::Job>& job) {
  std::unique_lock<std::mutex> lock(m_pendingMutex);
  if (m_incoming.empty()) {
    return m_inputDone ? Take::Stopped : Take::None;
  }
  job = std::move(m_incoming.front());
  m_incoming.pop_front();

  request = std::move(job->request);
  if (job->timing) {
    job->timing->taken = Clock::now();
  }
  lock.unlock();

  m_incomingRoom.notify_one();

  return Take::Job;
}

AthRemoteExec::Status RemoteExecHiveEventLoopMgr::validate(
    const ExecuteRequest& request, std::string& diagnostics) const {
  if (std::find(m_sequences.begin(), m_sequences.end(), request.sequence) ==
      m_sequences.end()) {
    diagnostics = "no sequence named '" + request.sequence + "'";
    return AthRemoteExec::Status::UnknownSequence;
  }

  if (const std::vector<std::string> absent = request.eventId.missing();
      !absent.empty()) {
    std::ostringstream text;
    text << "request carries no usable event identity; missing ";
    for (size_t i = 0; i < absent.size(); ++i) {
      text << (i == 0 ? "" : ", ") << absent[i];
    }
    diagnostics = text.str();
    return AthRemoteExec::Status::InvalidRequest;
  }

  const auto declared = m_inputKeys.value().find(request.sequence);
  std::vector<BoundaryInfo> boundaries;
  if (declared != m_inputKeys.value().end()) {
    for (const std::string& spec : declared->second) {
      const auto boundary = parseBoundary(spec);
      if (boundary.has_value()) {
        boundaries.push_back(std::move(*boundary));
      }
    }
  }

  for (auto payload = request.inputs.begin(); payload != request.inputs.end();
       ++payload) {
    if (payload->key.empty()) {
      diagnostics = "payload with an empty StoreGate key";
      return AthRemoteExec::Status::InvalidRequest;
    }

    if (payload->encoding.empty()) {
      diagnostics =
          std::format("payload '{}' carries no encoding", payload->key);
      return AthRemoteExec::Status::InvalidRequest;
    }

    if (std::any_of(request.inputs.begin(), payload,
                    [&payload](const Payload& earlier) {
                      return earlier.key == payload->key;
                    })) {
      diagnostics =
          std::format("payload '{}' is supplied more than once", payload->key);
      return AthRemoteExec::Status::InvalidRequest;
    }

    const auto boundary =
        std::find_if(boundaries.begin(), boundaries.end(),
                     [&payload](const BoundaryInfo& candidate) {
                       return candidate.key == payload->key;
                     });
    if (boundary == boundaries.end()) {
      diagnostics = std::format("sequence '{}' declares no input '{}'",
                                request.sequence, payload->key);
      return AthRemoteExec::Status::InvalidRequest;
    }

    if (payload->encoding != boundary->encoding ||
        payload->schema != boundary->schema) {
      diagnostics = std::format(
          "input '{}' arrived as {} [{}] but sequence '{}' declares {} [{}]",
          payload->key, payload->schema, payload->encoding, request.sequence,
          boundary->schema, boundary->encoding);
      return AthRemoteExec::Status::InvalidRequest;
    }
  }

  for (const BoundaryInfo& boundary : boundaries) {
    const auto supplied = std::find_if(
        request.inputs.begin(), request.inputs.end(),
        [&boundary](const Payload& p) { return p.key == boundary.key; });
    if (supplied == request.inputs.end()) {
      diagnostics = std::format("sequence '{}' requires input '{}'",
                                request.sequence, boundary.key);
      return AthRemoteExec::Status::InvalidRequest;
    }
  }

  return AthRemoteExec::Status::Ok;
}

StatusCode RemoteExecHiveEventLoopMgr::dispatch(
    ExecuteRequest request, std::unique_ptr<Server::Job> job) {
  std::string diagnostics;
  if (const AthRemoteExec::Status early = validate(request, diagnostics);
      early != AthRemoteExec::Status::Ok) {
    respondDirect(request, job, early, diagnostics);
    return StatusCode::SUCCESS;
  }

  EventContext ctx = allocateEventContext(m_nextEvent);
  if (!ctx.valid()) {
    respondDirect(request, job, AthRemoteExec::Status::Overloaded,
                  "could not allocate an event slot");
    return StatusCode::SUCCESS;
  }
  const size_t slot = ctx.slot();

  const EventId& id = request.eventId;
  ctx.setEventID(EventIDBase(
      *id.runNumber, static_cast<EventIDBase::event_number_t>(*id.eventNumber),
      *id.timeStamp, id.timeStampNsOffset, *id.lumiBlock, id.bunchCrossingId));
  Atlas::getExtendedEventContext(ctx).setConditionsRun(*id.runNumber);
  Gaudi::Hive::setCurrentContext(ctx);
  m_aess->reset(ctx);

  if (m_eventStore->record(std::make_unique<EventContext>(ctx), "EventContext")
          .isFailure() ||
      // The payloads are moved into the descriptor: nothing downstream reads
      // request.inputs again (InFlight keeps the request only for its id and
      // sequence name), and copying them here was a second duplicate of the
      // whole payload on the critical path.
      m_eventStore
          ->record(std::make_unique<RequestDescriptor>(
                       request.sequence, request.requestId,
                       std::move(request.inputs)),
                   m_requestKey)
          .isFailure()) {
    ATH_MSG_ERROR("Could not prepare slot {} for request {}", slot,
                  request.requestId);
    respondDirect(request, job, AthRemoteExec::Status::AlgFailure,
                  "could not prepare the event store");
    m_whiteboard->clearStore(slot).ignore();
    m_whiteboard->freeStore(slot).ignore();
    return StatusCode::SUCCESS;
  }

  if (job && job->timing) {
    job->timing->recorded = Clock::now();
  }

  InFlight entry;
  entry.request = std::move(request);
  entry.job = std::move(job);
  if (m_requestTimeout > 0.0) {
    entry.hasDeadline = true;
    entry.deadline =
        Clock::now() +
        std::chrono::milliseconds(static_cast<long>(m_requestTimeout * 1000.0));
  }

  ATH_MSG_DEBUG(
      "RemoteExec request {} ({}) dispatched to slot {} as run {} lb {} event "
      "{}",
      entry.request.requestId, entry.request.sequence, slot, *id.runNumber,
      *id.lumiBlock, *id.eventNumber);

  const uint64_t requestId = entry.request.requestId;
  auto [placed, inserted] = m_inFlight.emplace(slot, std::move(entry));
  if (!inserted) {
    ATH_MSG_FATAL("Slot {} was already in flight", slot);
    return StatusCode::FAILURE;
  }

  if (submitEvent(ctx).isFailure()) {
    ATH_MSG_ERROR("Could not submit request " << requestId);
    respond(placed->second, ExecuteReply{AthRemoteExec::Status::AlgFailure,
                                         "could not submit the event",
                                         {},
                                         requestId});
    m_whiteboard->clearStore(slot).ignore();
    m_whiteboard->freeStore(slot).ignore();
    m_inFlight.erase(placed);
    Gaudi::Hive::setCurrentContext(EventContext());
    return StatusCode::SUCCESS;
  }

  if (placed->second.job && placed->second.job->timing) {
    placed->second.job->timing->pushed = Clock::now();
  }

  {
    const std::lock_guard<std::mutex> lock(m_pendingMutex);
    ++m_awaitingCompletion;
  }
  m_inFlightCv.notify_one();

  ++m_nextEvent;
  Gaudi::Hive::setCurrentContext(EventContext());
  return StatusCode::SUCCESS;
}

StatusCode RemoteExecHiveEventLoopMgr::complete(const Finished& finished) {
  if (finished.ctx == nullptr) {
    ATH_MSG_FATAL("Scheduler returned a null context");
    return StatusCode::FAILURE;
  }
  const std::unique_ptr<EventContext> ctx(finished.ctx);

  const size_t slot = ctx->slot();
  const auto entry = m_inFlight.find(slot);
  if (entry == m_inFlight.end()) {
    ATH_MSG_FATAL("Slot {} finished but no request was in flight on it", slot);
    return StatusCode::FAILURE;
  }

  if (entry->second.job && entry->second.job->timing) {
    entry->second.job->timing->finished = finished.at;
  }

  const StatusCode sc = finishEvent(*ctx);
  if (sc.isFailure()) {
    respond(entry->second, ExecuteReply{AthRemoteExec::Status::AlgFailure,
                                        "could not finish the event",
                                        {},
                                        entry->second.request.requestId});
  }

  m_inFlight.erase(entry);
  ++m_completed;
  Gaudi::Hive::setCurrentContext(EventContext());
  return sc;
}

StatusCode RemoteExecHiveEventLoopMgr::eventFinished(const EventContext& ctx) {
  auto& entry = m_inFlight.at(ctx.slot());
  respond(entry, buildReply(ctx, entry));

  // An algorithm failure is a failed request, not a reason to stop serving!
  return StatusCode::SUCCESS;
}

ExecuteReply RemoteExecHiveEventLoopMgr::buildReply(const EventContext& ctx,
                                                    const InFlight& entry) {
  ExecuteReply reply;
  reply.requestId = entry.request.requestId;

  if (m_aess->eventStatus(ctx) != EventStatus::Success) {
    std::ostringstream diagnostics;
    diagnostics << "sequence '" << entry.request.sequence << "' failed ("
                << m_aess->eventStatus(ctx) << ")";
    const std::string failed = failedAlgorithms(ctx);
    if (!failed.empty()) {
      diagnostics << ": " << failed;
    }
    reply.status = AthRemoteExec::Status::AlgFailure;
    reply.diagnostics = diagnostics.str();
    return reply;
  }

  const std::string replyKey = replyKeyFor(entry.request.sequence);
  if (replyKey.empty()) {
    // A fragment may legitimately return nothing at all.
    reply.status = AthRemoteExec::Status::Ok;
    return reply;
  }

  if (m_whiteboard->selectStore(ctx.slot()).isFailure()) {
    reply.status = AthRemoteExec::Status::AlgFailure;
    reply.diagnostics =
        "could not select the event slot to read the reply from";
    return reply;
  }

  ReplyStaging* staged = nullptr;
  if (m_eventStore->retrieve(staged, replyKey).isFailure() ||
      staged == nullptr) {
    reply.status = AthRemoteExec::Status::AlgFailure;
    reply.diagnostics = std::format(
        "sequence '{}' staged no reply under '{}'; did its gate select it?",
        entry.request.sequence, replyKey);
    return reply;
  }

  reply = staged->takeReply();
  reply.requestId = entry.request.requestId;
  return reply;
}

void RemoteExecHiveEventLoopMgr::expireDeadlines() {
  if (m_requestTimeout <= 0.0) {
    return;
  }

  const Clock::time_point now = Clock::now();
  for (auto& [slot, entry] : m_inFlight) {
    if (entry.answered || !entry.hasDeadline || now < entry.deadline) {
      continue;
    }

    ATH_MSG_WARNING(
        "Request {} ({}) on slot {} exceeded {} s; answering the client now "
        "and releasing the slot when the event finishes",
        entry.request.requestId, entry.request.sequence, slot,
        m_requestTimeout.value());
    std::ostringstream diagnostics;
    diagnostics << "the server did not finish within "
                << m_requestTimeout.value() << " s";
    ExecuteReply reply;
    reply.status = AthRemoteExec::Status::Timeout;
    reply.diagnostics = diagnostics.str();
    respond(entry, std::move(reply));
  }
}

void RemoteExecHiveEventLoopMgr::respond(InFlight& entry, ExecuteReply reply) {
  if (entry.answered) {
    return;
  }
  entry.answered = true;
  reply.requestId = entry.request.requestId;

  if (reply.status != AthRemoteExec::Status::Ok) {
    ATH_MSG_WARNING(
        "RemoteExec request {} ({}) -> {}{}", reply.requestId,
        entry.request.sequence, toString(reply.status),
        !reply.diagnostics.empty() ? " : " + reply.diagnostics : "");
  } else {
    ATH_MSG_DEBUG("RemoteExec request {} ({}) -> {}{}", reply.requestId,
                  entry.request.sequence, toString(reply.status),
                  !reply.diagnostics.empty() ? " : " + reply.diagnostics : "");
    for (const Payload& output : reply.outputs) {
      ATH_MSG_DEBUG("  output " << output.key << " = " << describe(output));
    }
  }

  if (entry.job) {
    if (entry.job->timing) {
      entry.job->timing->replied = Clock::now();
    }
    entry.job->reply.set_value(std::move(reply));
  }
}

void RemoteExecHiveEventLoopMgr::respondDirect(
    const ExecuteRequest& request, const std::unique_ptr<Server::Job>& job,
    AthRemoteExec::Status status, const std::string& diagnostics) {
  ATH_MSG_INFO("RemoteExec request {} ({}) -> {}{}", request.requestId,
               request.sequence, toString(status),
               !diagnostics.empty() ? " : " + diagnostics : "");
  if (job) {
    ExecuteReply reply;
    reply.status = status;
    reply.diagnostics = diagnostics;
    reply.requestId = request.requestId;
    job->reply.set_value(std::move(reply));
  }
}

std::string RemoteExecHiveEventLoopMgr::failedAlgorithms(
    const EventContext& ctx) {
  SmartIF<IAlgManager> algMgr = serviceLocator()->as<IAlgManager>();
  if (!algMgr) {
    return {};
  }

  std::ostringstream names;
  for (IAlgorithm* alg : algMgr->getAlgorithms()) {
    if (alg == nullptr) {
      continue;
    }

    const AlgExecStateRef state = m_aess->algExecState(alg, ctx);
    if (state.state() == AlgExecState::State::Done &&
        state.execStatus().isFailure()) {
      names << (names.tellp() == 0 ? "" : ", ") << alg->name();
    }
  }

  return names.str();
}

std::string RemoteExecHiveEventLoopMgr::replyKeyFor(
    const std::string& sequence) const {
  const auto found = m_replyKeys.value().find(sequence);
  return found == m_replyKeys.value().end() ? std::string{} : found->second;
}

void RemoteExecHiveEventLoopMgr::serverMessage(Server::Level level,
                                               const std::string& text) {
  std::lock_guard<std::mutex> lock(m_messageMutex);
  msg(static_cast<MSG::Level>(level)) << "[gRPC] " << text << endmsg;
}

}  // namespace AthRemoteExec
