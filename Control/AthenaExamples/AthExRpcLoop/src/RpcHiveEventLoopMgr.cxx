/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcHiveEventLoopMgr.h"

#include "RpcBoundary.h"
#include "RpcReplyStaging.h"
#include "RpcRequestDescriptor.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaKernel/EventContextClid.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "StoreGate/StoreGateSvc.h"

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <thread>
#include <utility>

namespace {

/// Whiteboard slot value meaning "could not allocate".
constexpr size_t s_noSlot = std::string::npos;

}  // anonymous namespace

namespace AthExRpc {

RpcHiveEventLoopMgr::RpcHiveEventLoopMgr( const std::string& name,
                                          ISvcLocator* svcLoc )
    : base_class( name, svcLoc ), AthMessaging( name )
{
}

RpcHiveEventLoopMgr::~RpcHiveEventLoopMgr() = default;

StatusCode RpcHiveEventLoopMgr::initialize()
{
  ATH_CHECK( MinimalEventLoopMgr::initialize() );

  m_whiteboard = serviceLocator()->service( m_whiteboardName );
  if ( !m_whiteboard.isValid() ) {
    ATH_MSG_FATAL( "Could not retrieve " << m_whiteboardName.value()
                                         << " as an IHiveWhiteBoard" );
    return StatusCode::FAILURE;
  }
  m_scheduler = serviceLocator()->service( m_schedulerName );
  if ( !m_scheduler.isValid() ) {
    ATH_MSG_FATAL( "Could not retrieve " << m_schedulerName.value()
                                         << " as an IScheduler" );
    return StatusCode::FAILURE;
  }
  m_aess = serviceLocator()->service( "AlgExecStateSvc" );
  if ( !m_aess.isValid() ) {
    ATH_MSG_FATAL( "Could not retrieve AlgExecStateSvc" );
    return StatusCode::FAILURE;
  }
  ATH_CHECK( m_evtStore.retrieve() );
  ATH_CHECK( m_incidentSvc.retrieve() );
  ATH_CHECK( m_conditionsCleaner.retrieve() );

  if ( m_sequences.empty() ) {
    ATH_MSG_ERROR( "No sequences configured; the server would have nothing to "
                   "offer. Set the Sequences property." );
    return StatusCode::FAILURE;
  }

  const auto known = [this]( const std::string& sequence ) {
    return std::find( m_sequences.begin(), m_sequences.end(), sequence ) !=
           m_sequences.end();
  };
  for ( const auto& [sequence, keys] : m_outputKeys ) {
    if ( !known( sequence ) ) {
      ATH_MSG_ERROR( "OutputKeys names sequence '"
                     << sequence << "', which is not in Sequences" );
      return StatusCode::FAILURE;
    }
  }
  for ( const auto& [sequence, key] : m_replyKeys ) {
    if ( !known( sequence ) ) {
      ATH_MSG_ERROR( "ReplyKeys names sequence '"
                     << sequence << "', which is not in Sequences" );
      return StatusCode::FAILURE;
    }
  }
  // A sequence that returns outputs needs somewhere for its pack algorithm to
  // stage them; catching that here beats every request failing at run time.
  for ( const auto& [sequence, keys] : m_outputKeys ) {
    if ( !keys.empty() && m_replyKeys.value().count( sequence ) == 0 ) {
      ATH_MSG_ERROR( "Sequence '" << sequence << "' declares outputs but no "
                                                 "entry in ReplyKeys" );
      return StatusCode::FAILURE;
    }
  }

  std::ostringstream menu;
  for ( const std::string& sequence : m_sequences ) {
    menu << ( menu.tellp() == 0 ? "" : ", " ) << sequence;
  }
  ATH_MSG_INFO( "Serving " << m_sequences.size() << " sequence(s): "
                           << menu.str() << " over "
                           << m_whiteboard->getNumberOfStores() << " slot(s)" );
  return StatusCode::SUCCESS;
}

StatusCode RpcHiveEventLoopMgr::stop()
{
  StatusCode sc = MinimalEventLoopMgr::stop();

  // If the loop exited early, some slots may still hold data. Clearing them
  // here avoids a crash later, when DetectorStore is finalised before the
  // whiteboard (same reasoning as AthenaHiveEventLoopMgr::stop). The guard is
  // for the case where initialize() failed before the whiteboard was found.
  if ( m_whiteboard ) {
    const size_t slots = m_whiteboard->getNumberOfStores();
    for ( size_t slot = 0; slot < slots; ++slot ) {
      sc &= m_whiteboard->freeStore( slot );
    }
  }
  Gaudi::Hive::setCurrentContext( EventContext() );
  return sc;
}

StatusCode RpcHiveEventLoopMgr::finalize()
{
  // Normally already done by nextEvent; this covers the paths where the loop
  // never ran or threw, so the threads cannot outlive the services they use.
  stopHelpers();
  if ( m_server ) {
    m_server->shutdown();
    m_server.reset();
  }
  m_whiteboard = nullptr;
  m_scheduler = nullptr;
  m_aess = nullptr;
  return MinimalEventLoopMgr::finalize();
}

StatusCode RpcHiveEventLoopMgr::executeRun( int maxevt )
{
  ATH_CHECK( nextEvent( maxevt ) );
  m_incidentSvc->fireIncident( Incident( name(), "EndEvtLoop" ) );
  return StatusCode::SUCCESS;
}

StatusCode RpcHiveEventLoopMgr::stopRun()
{
  ATH_MSG_INFO( "Stop requested, draining the server" );
  m_stopRequested = true;
  if ( m_server ) {
    m_server->shutdown();
  }
  m_scheduledStop = true;
  return StatusCode::SUCCESS;
}

StatusCode RpcHiveEventLoopMgr::startServing()
{
  if ( cannedMode() ) {
    ATH_CHECK( cannedRequests() );
    ATH_MSG_INFO( "Test mode: serving " << m_canned.size()
                                        << " canned request(s), no socket" );
    return StatusCode::SUCCESS;
  }

  // Started here rather than in initialize() so that it survives any future
  // fork story: threads do not survive fork, so they must be created after it
  // (Control/AthenaServices/src/TimeoutAlg.cxx:41-48).
  m_server = std::make_unique<RpcServer>();

  std::vector<SequenceInfo> menu;
  menu.reserve( m_sequences.size() );
  for ( const std::string& sequence : m_sequences ) {
    SequenceInfo info;
    info.name = sequence;
    // Advertise encoding and schema alongside the key, so a client can
    // discover what to build rather than being told out of band. This is the
    // same declaration the gate and pack algorithms parse, so what is
    // advertised is what the fragment will actually accept.
    const auto advertise = []( const std::vector<std::string>& specs,
                               std::vector<BoundaryInfo>& out ) {
      for ( const std::string& spec : specs ) {
        Boundary boundary;
        if ( parseBoundary( spec, boundary ) ) {
          out.push_back(
              {boundary.key, boundary.encoding, boundary.schema} );
        }
      }
    };
    if ( auto inputs = m_inputKeys.value().find( sequence );
         inputs != m_inputKeys.value().end() ) {
      advertise( inputs->second, info.inputs );
    }
    if ( auto outputs = m_outputKeys.value().find( sequence );
         outputs != m_outputKeys.value().end() ) {
      advertise( outputs->second, info.outputs );
    }
    menu.push_back( std::move( info ) );
  }
  m_server->setMenu( std::move( menu ) );

  const int boundPort = m_server->start(
      m_address, m_port, m_queueLimit, m_maxMessageBytes,
      [this]( RpcServer::Level level, const std::string& text ) {
        serverMessage( level, text );
      } );
  if ( boundPort < 0 ) {
    ATH_MSG_ERROR( "Could not start the gRPC server" );
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO( "gRPC server listening on " << m_address.value() << ":"
                                            << boundPort );

  if ( !m_portFile.empty() ) {
    // Written only after the port is known, so a client that waits for the
    // file to appear cannot read a half-written or stale value.
    const std::string temporary = m_portFile + ".tmp";
    {
      std::ofstream out( temporary );
      out << boundPort << '\n';
    }
    if ( std::rename( temporary.c_str(), m_portFile.value().c_str() ) != 0 ) {
      ATH_MSG_ERROR( "Could not write the port file " << m_portFile.value() );
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode RpcHiveEventLoopMgr::nextEvent( int maxevt )
{
  if ( maxevt == 0 ) {
    return StatusCode::SUCCESS;
  }
  ATH_CHECK( startServing() );

  const size_t limit =
      maxevt < 0 ? std::numeric_limits<size_t>::max()
                 : static_cast<size_t>( maxevt );
  const size_t slots = m_whiteboard->getNumberOfStores();
  const std::chrono::milliseconds poll( m_pollInterval );
  size_t accepted = 0;

  ATH_MSG_INFO( "Starting the request loop" );
  // Stamped when the *first* request is accepted rather than here, so the rate
  // below excludes however long the loop sat idle waiting for a client to
  // connect. In canned mode the queue is already full and the two coincide.
  std::optional<Clock::time_point> firstAccepted;
  startHelpers();

  const auto canAccept = [&] {
    return !m_stopRequested && accepted < limit && m_inFlight.size() < slots &&
           m_scheduler->freeSlots() > 0;
  };

  while ( true ) {
    if ( m_inFlight.empty() && ( m_stopRequested || accepted >= limit ) ) {
      break;
    }

    bool progressed = false;

    // Accept everything that is already queued. Nothing here ever blocks: the
    // input thread is the one that waits for requests, so the loop is free to
    // go and look at completions instead.
    while ( canAccept() ) {
      ExecuteRequest request;
      std::unique_ptr<RpcServer::Job> job;
      const Take taken = takeRequest( request, job );
      if ( taken == Take::None ) {
        break;
      }
      if ( taken == Take::Stopped ) {
        m_stopRequested = true;
        // Counts as progress: the loop's exit condition has just become
        // satisfiable, so it must go round and re-test it rather than fall
        // through to the wait below. In canned mode that wait is
        // unbounded and nothing can ever end it -- there is no input thread,
        // and the output thread has nothing left in flight to report -- so
        // without this the job hangs after serving every request correctly.
        //
        // It only shows up with a single slot. With two or more, the last
        // completions and this transition tend to land in the same iteration,
        // and their `progressed` covers for it; the deadlock is real either
        // way and was simply never reached, because every canned test until
        // the throughput sweep ran with --concurrent-events=2.
        progressed = true;
        break;
      }
      ++accepted;
      if ( !firstAccepted ) {
        firstAccepted = Clock::now();
      }
      ATH_CHECK( dispatch( std::move( request ), std::move( job ) ) );
      progressed = true;
    }

    // Complete everything the output thread has collected.
    for ( const Finished& finished : takeFinished() ) {
      ATH_CHECK( complete( finished ) );
      progressed = true;
    }

    expireDeadlines();

    if ( progressed ) {
      continue;
    }

    // Nothing to do. Wait for either helper to bring something, which is the
    // whole point of having two of them: a request arriving while an event is
    // still running now wakes the loop immediately instead of waiting out the
    // event. Deadlines still need a periodic wake, so RequestTimeout turns the
    // wait into a bounded one.
    std::unique_lock<std::mutex> lock( m_pendingMutex );
    const auto nothingToDo = [&] {
      return ( m_incoming.empty() || !canAccept() ) && m_finishedEvents.empty();
    };
    if ( nothingToDo() ) {
      if ( m_requestTimeout > 0.0 ) {
        m_pending.wait_for( lock, poll );
      } else {
        // Deliberately not "|| m_inputDone": once the input thread has
        // finished, that would be true on every check and the wait would spin
        // a core for as long as the last events took to drain. Nothing is lost
        // by omitting it -- if input is done and nothing is in flight, the top
        // of the loop has already broken out.
        m_pending.wait( lock, [&] { return !nothingToDo(); } );
      }
    }
  }

  stopHelpers();

  ATH_MSG_INFO( "Request loop finished: " << accepted << " request(s) accepted, "
                                          << m_completed << " event(s) run" );

  // The throughput line. It measures the loop, so it excludes startup: a
  // server that loads a detector description pays that once, and charging it
  // to the requests would say nothing about how fast the server answers.
  if ( firstAccepted && m_completed > 0 ) {
    const double seconds =
        std::chrono::duration<double>( Clock::now() - *firstAccepted ).count();
    if ( seconds > 0.0 ) {
      ATH_MSG_ALWAYS( "Throughput: " << m_completed << " request(s) in "
                                     << seconds << " s = "
                                     << ( m_completed / seconds )
                                     << " request/s over "
                                     << m_whiteboard->getNumberOfStores()
                                     << " slot(s)" );
    }
  }
  if ( m_server ) {
    if ( m_reportTiming ) {
      const std::string report = m_server->timingReport();
      if ( !report.empty() ) {
        // ALWAYS, not INFO: with LogEveryRequest on, a busy server hits the
        // MessageSvc INFO limit long before it gets here, and the one message
        // worth keeping would be the one suppressed.
        ATH_MSG_ALWAYS( '\n' << report );
      }
    }
    m_server->shutdown();
  }
  return StatusCode::SUCCESS;
}

void RpcHiveEventLoopMgr::startHelpers()
{
  {
    const std::lock_guard<std::mutex> lock( m_pendingMutex );
    m_helpersStopping = false;
    // One per slot: enough that the loop never waits on the input thread for
    // work it could be dispatching, and no more, so everything past that
    // queues where QueueLimit governs it.
    m_incomingLimit = std::max<size_t>( 1, m_whiteboard->getNumberOfStores() );
    // With no server there is nothing to block on for input; the canned list
    // is drained by takeRequest directly.
    m_inputDone = ( m_server == nullptr );
  }
  if ( m_server && !m_inputThread.joinable() ) {
    m_inputThread = std::thread( [this] { inputLoop(); } );
  }
  if ( !m_outputThread.joinable() ) {
    m_outputThread = std::thread( [this] { outputLoop(); } );
  }
}

void RpcHiveEventLoopMgr::stopHelpers()
{
  {
    const std::lock_guard<std::mutex> lock( m_pendingMutex );
    m_helpersStopping = true;
  }
  m_inFlightCv.notify_all();
  m_pending.notify_all();
  m_incomingRoom.notify_all();

  // Order matters here, and getting it wrong deadlocks rather than misbehaves.
  //
  // The input thread is parked in RpcServer::pop, which only returns once the
  // server stops accepting, so it has to be told before it can be joined. But
  // only *stopAccepting*, not the full shutdown: that blocks until every gRPC
  // handler has returned, and handlers whose requests are still sitting in
  // m_incoming are waiting on promises that only drainPending will fulfil.
  // Shutting gRPC down before draining is a deadlock, and was one.
  if ( m_inputThread.joinable() ) {
    if ( m_server ) {
      m_server->stopAccepting();
    }
    m_inputThread.join();
  }

  // Safe now: the input thread has stopped, so nothing can add to m_incoming
  // while it is being emptied.
  drainPending();

  if ( m_server ) {
    m_server->shutdown();
  }
  if ( m_outputThread.joinable() ) {
    m_outputThread.join();
  }
}

void RpcHiveEventLoopMgr::drainPending()
{
  std::deque<std::unique_ptr<RpcServer::Job>> leftover;
  std::deque<Finished> uncollected;
  {
    const std::lock_guard<std::mutex> lock( m_pendingMutex );
    leftover.swap( m_incoming );
    uncollected.swap( m_finishedEvents );
  }

  for ( const std::unique_ptr<RpcServer::Job>& job : leftover ) {
    // Destroying these instead would break their promises, literally: the
    // handler thread waiting on the future gets broken_promise thrown out of a
    // gRPC handler. "Every request gets a reply" has to include the ones the
    // loop stopped before reaching, which is routine whenever a job is given a
    // maxevt smaller than the number of requests a client sends.
    ATH_MSG_WARNING( "Request " << job->request.requestId << " ("
                                << job->request.sequence
                                << ") was still queued when the loop stopped" );
    respondDirect( job->request, job, AthExRpc::Status::ShuttingDown,
                   "the server stopped before this request was served" );
  }

  // Contexts the output thread collected but the loop never completed. Only
  // reachable if the loop exited on an error; freeing them keeps that path from
  // leaking on top of whatever went wrong.
  for ( const Finished& finished : uncollected ) {
    delete finished.ctx;
  }
}

void RpcHiveEventLoopMgr::inputLoop()
{
  std::unique_ptr<RpcServer::Job> job;
  while ( m_server->pop( job ) ) {
    {
      std::unique_lock<std::mutex> lock( m_pendingMutex );
      // Wait for the loop to take something before handing over more. Without
      // this, everything would be pulled out of RpcServer's bounded queue and
      // piled up here instead, so QueueLimit would never reject anything
      // however overloaded the server got -- it would accumulate requests, and
      // their payloads, until it ran out of memory.
      m_incomingRoom.wait( lock, [this] {
        return m_incoming.size() < m_incomingLimit || m_helpersStopping;
      } );
      m_incoming.push_back( std::move( job ) );
    }
    m_pending.notify_one();
  }
  {
    const std::lock_guard<std::mutex> lock( m_pendingMutex );
    m_inputDone = true;
  }
  m_pending.notify_one();
}

void RpcHiveEventLoopMgr::outputLoop()
{
  while ( true ) {
    {
      std::unique_lock<std::mutex> lock( m_pendingMutex );
      m_inFlightCv.wait( lock, [this] {
        return m_awaitingCompletion > 0 || m_helpersStopping;
      } );
      if ( m_awaitingCompletion == 0 ) {
        return;  // stopping, and nothing is left to wait for
      }
    }

    // Safe to block here: m_awaitingCompletion counts events the scheduler has
    // accepted and not yet handed back, so while it is non-zero one is coming.
    EventContext* finished = nullptr;
    if ( m_scheduler->popFinishedEvent( finished ).isFailure() ) {
      // The scheduler reports failure when it believes nothing is in flight,
      // which can briefly disagree with our own count around a push, and
      // permanently once it has been deactivated. Neither is worth spinning on.
      std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
      continue;
    }
    const Clock::time_point at = Clock::now();

    {
      const std::lock_guard<std::mutex> lock( m_pendingMutex );
      --m_awaitingCompletion;
      m_finishedEvents.push_back( Finished{ finished, at } );
    }
    m_pending.notify_one();
  }
}

std::vector<RpcHiveEventLoopMgr::Finished> RpcHiveEventLoopMgr::takeFinished()
{
  std::vector<Finished> finished;
  const std::lock_guard<std::mutex> lock( m_pendingMutex );
  finished.assign( m_finishedEvents.begin(), m_finishedEvents.end() );
  m_finishedEvents.clear();
  return finished;
}

RpcHiveEventLoopMgr::Take RpcHiveEventLoopMgr::takeRequest(
    ExecuteRequest& request, std::unique_ptr<RpcServer::Job>& job )
{
  if ( cannedMode() ) {
    if ( m_canned.empty() ) {
      return Take::Stopped;
    }
    request = std::move( m_canned.front() );
    m_canned.pop_front();
    return Take::Job;
  }

  // Never blocks: the input thread does the waiting. Whatever it has already
  // handed over is here, and if that is nothing the loop has completions to
  // look at before it parks.
  bool makeRoom = false;
  std::unique_lock<std::mutex> lock( m_pendingMutex );
  if ( m_incoming.empty() ) {
    return m_inputDone ? Take::Stopped : Take::None;
  }
  job = std::move( m_incoming.front() );
  m_incoming.pop_front();
  makeRoom = true;
  // Moved, not copied. The server is done with this job's request the moment
  // it leaves the queue -- only the promise is still wanted -- and copying it
  // here duplicated the whole payload, which the phase table showed costing
  // about as much as decoding it.
  request = std::move( job->request );
  if ( job->timing ) {
    job->timing->taken = Clock::now();
  }
  lock.unlock();
  if ( makeRoom ) {
    m_incomingRoom.notify_one();
  }
  return Take::Job;
}

AthExRpc::Status RpcHiveEventLoopMgr::validate( const ExecuteRequest& request,
                                      std::string& detail ) const
{
  if ( std::find( m_sequences.begin(), m_sequences.end(), request.sequence ) ==
       m_sequences.end() ) {
    detail = "no sequence named '" + request.sequence + "'";
    return AthExRpc::Status::UnknownSequence;
  }

  // Before anything else: conditions are resolved against the EventID built
  // from this, so a request without one cannot be served correctly -- and the
  // failure would be a wrong answer rather than an error. Caught here, where it
  // costs no slot and the client is told exactly which fields are absent.
  if ( const std::vector<std::string> absent = request.eventId.missing();
       !absent.empty() ) {
    std::ostringstream text;
    text << "request carries no usable event identity; missing ";
    for ( size_t i = 0; i < absent.size(); ++i ) {
      text << ( i == 0 ? "" : ", " ) << absent[i];
    }
    detail = text.str();
    return AthExRpc::Status::InvalidRequest;
  }

  // The whole declared boundary, checked before a slot is spent on it. The
  // gate would reject each of these too, but only after the request has cost a
  // slot, an event and a scheduler round trip -- and a request that fails
  // inside the scheduler is reported as an algorithm failure, which tells the
  // client nothing about what was wrong with what it sent.
  const auto declared = m_inputKeys.value().find( request.sequence );
  std::vector<Boundary> boundaries;
  if ( declared != m_inputKeys.value().end() ) {
    for ( const std::string& spec : declared->second ) {
      Boundary boundary;
      if ( parseBoundary( spec, boundary ) ) {
        boundaries.push_back( std::move( boundary ) );
      }
    }
  }

  for ( auto payload = request.inputs.begin(); payload != request.inputs.end();
        ++payload ) {
    if ( payload->key.empty() ) {
      detail = "payload with an empty StoreGate key";
      return AthExRpc::Status::InvalidRequest;
    }
    if ( payload->encoding.empty() || payload->schema.empty() ) {
      detail = "payload '" + payload->key + "' carries no encoding or schema";
      return AthExRpc::Status::InvalidRequest;
    }
    // A duplicate would be silently ignored: the gate takes the first match
    // and the second payload -- possibly the one the client meant -- would
    // never be looked at.
    if ( std::any_of( request.inputs.begin(), payload,
                      [&payload]( const Payload& earlier ) {
                        return earlier.key == payload->key;
                      } ) ) {
      detail = "payload '" + payload->key + "' is supplied more than once";
      return AthExRpc::Status::InvalidRequest;
    }
    const auto boundary =
        std::find_if( boundaries.begin(), boundaries.end(),
                      [&payload]( const Boundary& candidate ) {
                        return candidate.key == payload->key;
                      } );
    // An undeclared key is a client that thinks it is talking to a different
    // fragment -- a renamed boundary, a stale configuration. Ignoring it and
    // running anyway would answer that client with a result computed from
    // something else.
    if ( boundary == boundaries.end() ) {
      detail = "sequence '" + request.sequence + "' declares no input '" +
               payload->key + "'";
      return AthExRpc::Status::InvalidRequest;
    }
    if ( payload->encoding != boundary->encoding ||
         payload->schema != boundary->schema ) {
      detail = "input '" + payload->key + "' arrived as " + payload->schema +
               " [" + payload->encoding + "] but sequence '" +
               request.sequence + "' declares " + boundary->schema + " [" +
               boundary->encoding + "]";
      return AthExRpc::Status::InvalidRequest;
    }
  }

  for ( const Boundary& boundary : boundaries ) {
    const auto supplied = std::find_if(
        request.inputs.begin(), request.inputs.end(),
        [&boundary]( const Payload& p ) { return p.key == boundary.key; } );
    if ( supplied == request.inputs.end() ) {
      detail = "sequence '" + request.sequence + "' requires input '" +
               boundary.key + "'";
      return AthExRpc::Status::InvalidRequest;
    }
  }
  return AthExRpc::Status::Ok;
}

StatusCode RpcHiveEventLoopMgr::dispatch( ExecuteRequest request,
                                          std::unique_ptr<RpcServer::Job> job )
{
  std::string detail;
  // Note the qualification: IInterface::Status is inherited into this class and
  // shadows AthExRpc::Status, so the unqualified name is the wrong enum.
  if ( const AthExRpc::Status early = validate( request, detail );
       early != AthExRpc::Status::Ok ) {
    respondDirect( request, job, early, detail );
    return StatusCode::SUCCESS;
  }

  const size_t slot = m_whiteboard->allocateStore( static_cast<int>( m_nextEvent ) );
  if ( slot == s_noSlot ) {
    respondDirect( request, job, AthExRpc::Status::Overloaded,
                   "no free event slot" );
    return StatusCode::SUCCESS;
  }
  if ( m_whiteboard->selectStore( slot ).isFailure() ) {
    ATH_MSG_ERROR( "Could not select whiteboard slot " << slot );
    respondDirect( request, job, AthExRpc::Status::AlgFailure,
                   "could not select an event slot" );
    m_whiteboard->freeStore( slot ).ignore();
    return StatusCode::SUCCESS;
  }

  // m_nextEvent is the context's own index: job-local bookkeeping that has to
  // be unique and monotonic, and is not an event identity. The identity comes
  // from the client, and validate() has already established that all four
  // fields it needs are present.
  const EventId& id = request.eventId;
  EventContext ctx( m_nextEvent, slot );
  ctx.setEventID( EventIDBase(
      *id.runNumber, static_cast<EventIDBase::event_number_t>( *id.eventNumber ),
      *id.timeStamp, id.timeStampNsOffset, *id.lumiBlock,
      id.bunchCrossingId ) );
  // Without the extended context every SG::ReadCondHandle constructor throws.
  // Its conditions run number is the client's too: a service that pinned it to
  // a property would resolve every request against one run's conditions,
  // whichever run the request actually came from.
  Atlas::setExtendedEventContext(
      ctx, Atlas::ExtendedEventContext( m_evtStore->hiveProxyDict(),
                                        *id.runNumber ) );
  Gaudi::Hive::setCurrentContext( ctx );
  m_aess->reset( ctx );

  if ( m_evtStore->record( std::make_unique<EventContext>( ctx ),
                           "EventContext" )
           .isFailure() ||
       // The payloads are moved into the descriptor: nothing downstream reads
       // request.inputs again (InFlight keeps the request only for its id and
       // sequence name), and copying them here was a second duplicate of the
       // whole payload on the critical path.
       m_evtStore
           ->record( std::make_unique<RpcRequestDescriptor>(
                         request.sequence, request.requestId,
                         std::move( request.inputs ) ),
                     m_requestKey )
           .isFailure() ) {
    ATH_MSG_ERROR( "Could not prepare slot " << slot << " for request "
                                             << request.requestId );
    respondDirect( request, job, AthExRpc::Status::AlgFailure,
                   "could not prepare the event store" );
    m_whiteboard->clearStore( slot ).ignore();
    m_whiteboard->freeStore( slot ).ignore();
    return StatusCode::SUCCESS;
  }

  // Age out conditions objects the server no longer needs, exactly as
  // AthenaHiveEventLoopMgr and AthenaMtesEventLoopMgr do once per event. It has
  // to run after the extended context is set, because the cleaner resolves
  // against that context's conditions run number, and before the event is
  // pushed, so cleaning never overlaps the request it belongs to. Asynchronous
  // cleaning is allowed: it is a TBB task, not work on this thread.
  if ( m_conditionsCleaner->event( ctx, true ).isFailure() ) {
    ATH_MSG_ERROR( "Conditions cleaning failed for request "
                   << request.requestId );
    respondDirect( request, job, AthExRpc::Status::AlgFailure,
                   "conditions cleaning failed" );
    m_whiteboard->clearStore( slot ).ignore();
    m_whiteboard->freeStore( slot ).ignore();
    return StatusCode::SUCCESS;
  }

  if ( job && job->timing ) {
    job->timing->recorded = Clock::now();
  }

  InFlight entry;
  entry.request = std::move( request );
  entry.job = std::move( job );
  if ( m_requestTimeout > 0.0 ) {
    entry.hasDeadline = true;
    entry.deadline =
        Clock::now() + std::chrono::milliseconds(
                           static_cast<long>( m_requestTimeout * 1000.0 ) );
  }

  if ( m_logEveryRequest ) {
    ATH_MSG_INFO( "RPC request " << entry.request.requestId << " ("
                                 << entry.request.sequence
                                 << ") dispatched to slot " << slot
                                 << " as run " << *id.runNumber << " lb "
                                 << *id.lumiBlock << " event "
                                 << *id.eventNumber );
  }

  m_incidentSvc->fireIncident(
      Incident( name(), IncidentType::BeginProcessing, ctx ) );

  const uint64_t requestId = entry.request.requestId;
  auto [placed, inserted] = m_inFlight.emplace( slot, std::move( entry ) );
  if ( !inserted ) {
    ATH_MSG_FATAL( "Slot " << slot << " was already in flight" );
    return StatusCode::FAILURE;
  }

  // Held here until pushNewEvent accepts it. The scheduler takes ownership on
  // success and deletes the context when the event comes back out; on failure
  // it has not, so a bare `new` would leak one context per refused request --
  // and a server refuses requests for a living.
  auto submitted = std::make_unique<EventContext>( ctx );
  if ( m_scheduler->pushNewEvent( submitted.get() ).isFailure() ) {
    ATH_MSG_ERROR( "Scheduler refused request " << requestId );
    // BeginProcessing was fired above, so it has to be balanced here: nothing
    // downstream will, because this event never reaches the scheduler and so
    // never comes back through complete().
    m_incidentSvc->fireIncident(
        Incident( name(), IncidentType::EndProcessing, ctx ) );
    respond( placed->second, ExecuteReply{ AthExRpc::Status::AlgFailure,
                                           "scheduler refused the event",
                                           {},
                                           requestId } );
    m_whiteboard->clearStore( slot ).ignore();
    m_whiteboard->freeStore( slot ).ignore();
    m_inFlight.erase( placed );
    Gaudi::Hive::setCurrentContext( EventContext() );
    return StatusCode::SUCCESS;
  }
  submitted.release();  // the scheduler owns it now

  if ( placed->second.job && placed->second.job->timing ) {
    placed->second.job->timing->pushed = Clock::now();
  }
  {
    // Tell the output thread there is something to wait for. Until this is
    // non-zero it must not call popFinishedEvent, which reports failure rather
    // than blocking when the scheduler believes it is idle.
    const std::lock_guard<std::mutex> lock( m_pendingMutex );
    ++m_awaitingCompletion;
  }
  m_inFlightCv.notify_one();

  ++m_nextEvent;
  Gaudi::Hive::setCurrentContext( EventContext() );
  return StatusCode::SUCCESS;
}

StatusCode RpcHiveEventLoopMgr::complete( const Finished& finished )
{
  if ( finished.ctx == nullptr ) {
    ATH_MSG_FATAL( "Scheduler returned a null context" );
    return StatusCode::FAILURE;
  }
  const std::unique_ptr<EventContext> ctx( finished.ctx );

  const size_t slot = ctx->slot();
  const auto entry = m_inFlight.find( slot );
  if ( entry == m_inFlight.end() ) {
    ATH_MSG_FATAL( "Slot " << slot << " finished but no request was in flight "
                                      "on it" );
    return StatusCode::FAILURE;
  }

  if ( entry->second.job && entry->second.job->timing ) {
    entry->second.job->timing->finished = finished.at;
  }

  Gaudi::Hive::setCurrentContext( *ctx );
  respond( entry->second, buildReply( *ctx, entry->second ) );
  m_incidentSvc->fireIncident(
      Incident( name(), IncidentType::EndProcessing, *ctx ) );

  // Nothing else clears this slot: unlike AthenaHiveEventLoopMgr we do not
  // listen for EndAlgorithms, precisely so the store survives until the reply
  // above has been read out of it.
  if ( m_whiteboard->clearStore( slot ).isFailure() ) {
    ATH_MSG_WARNING( "Could not clear whiteboard slot " << slot );
  }
  if ( m_whiteboard->freeStore( slot ).isFailure() ) {
    ATH_MSG_ERROR( "Could not free whiteboard slot " << slot );
    return StatusCode::FAILURE;
  }

  m_inFlight.erase( entry );
  ++m_completed;
  Gaudi::Hive::setCurrentContext( EventContext() );
  return StatusCode::SUCCESS;
}

ExecuteReply RpcHiveEventLoopMgr::buildReply( const EventContext& ctx,
                                              const InFlight& entry )
{
  ExecuteReply reply;
  reply.requestId = entry.request.requestId;

  if ( m_aess->eventStatus( ctx ) != EventStatus::Success ) {
    std::ostringstream detail;
    detail << "sequence '" << entry.request.sequence << "' failed ("
           << m_aess->eventStatus( ctx ) << ")";
    const std::string failed = failedAlgorithms( ctx );
    if ( !failed.empty() ) {
      detail << ": " << failed;
    }
    reply.status = AthExRpc::Status::AlgFailure;
    reply.detail = detail.str();
    return reply;
  }

  const std::string replyKey = replyKeyFor( entry.request.sequence );
  if ( replyKey.empty() ) {
    // A fragment may legitimately return nothing at all.
    reply.status = AthExRpc::Status::Ok;
    return reply;
  }

  if ( m_whiteboard->selectStore( ctx.slot() ).isFailure() ) {
    reply.status = AthExRpc::Status::AlgFailure;
    reply.detail = "could not select the event slot to read the reply from";
    return reply;
  }

  RpcReplyStaging* staged = nullptr;
  if ( m_evtStore->retrieve( staged, replyKey ).isFailure() ||
       staged == nullptr ) {
    reply.status = AthExRpc::Status::AlgFailure;
    reply.detail = "sequence '" + entry.request.sequence +
                   "' staged no reply under '" + replyKey +
                   "'; did its gate select it?";
    return reply;
  }

  reply = staged->takeReply();
  reply.requestId = entry.request.requestId;
  return reply;
}

void RpcHiveEventLoopMgr::expireDeadlines()
{
  if ( m_requestTimeout <= 0.0 ) {
    return;
  }
  const Clock::time_point now = Clock::now();
  for ( auto& [slot, entry] : m_inFlight ) {
    if ( entry.answered || !entry.hasDeadline || now < entry.deadline ) {
      continue;
    }
    ATH_MSG_WARNING( "Request " << entry.request.requestId << " ("
                                << entry.request.sequence << ") on slot " << slot
                                << " exceeded " << m_requestTimeout.value()
                                << "s; answering the client now and releasing "
                                   "the slot when the event finishes" );
    std::ostringstream detail;
    detail << "the server did not finish within " << m_requestTimeout.value()
           << " s";
    ExecuteReply reply;
    reply.status = AthExRpc::Status::Timeout;
    reply.detail = detail.str();
    respond( entry, std::move( reply ) );
  }
}

void RpcHiveEventLoopMgr::respond( InFlight& entry, ExecuteReply reply )
{
  if ( entry.answered ) {
    return;  // already timed out; the event just finished late
  }
  entry.answered = true;
  reply.requestId = entry.request.requestId;

  if ( m_logEveryRequest ) {
    ATH_MSG_INFO( "RPC request " << reply.requestId << " ("
                                 << entry.request.sequence << ") -> "
                                 << toString( reply.status )
                                 << ( reply.detail.empty() ? "" : ": " )
                                 << reply.detail );
    for ( const Payload& output : reply.outputs ) {
      ATH_MSG_INFO( "  output " << output.key << " = " << describe( output ) );
    }
  } else if ( reply.status != AthExRpc::Status::Ok ) {
    // Silence is for the happy path only; a client being told no is always
    // worth a line.
    ATH_MSG_WARNING( "RPC request " << reply.requestId << " ("
                                    << entry.request.sequence << ") -> "
                                    << toString( reply.status )
                                    << ( reply.detail.empty() ? "" : ": " )
                                    << reply.detail );
  }

  if ( entry.job ) {
    // Stamped before the promise, not after: set_value can wake the handler
    // thread immediately, and it stamps handlerExit. Doing it the other way
    // round is a data race that would also read as negative time.
    if ( entry.job->timing ) {
      entry.job->timing->replied = Clock::now();
    }
    entry.job->reply.set_value( std::move( reply ) );
  }
}

void RpcHiveEventLoopMgr::respondDirect(
    const ExecuteRequest& request, const std::unique_ptr<RpcServer::Job>& job,
    AthExRpc::Status status, const std::string& detail )
{
  ATH_MSG_INFO( "RPC request " << request.requestId << " (" << request.sequence
                               << ") -> " << toString( status )
                               << ( detail.empty() ? "" : ": " ) << detail );
  if ( job ) {
    ExecuteReply reply;
    reply.status = status;
    reply.detail = detail;
    reply.requestId = request.requestId;
    job->reply.set_value( std::move( reply ) );
  }
}

std::string RpcHiveEventLoopMgr::failedAlgorithms( const EventContext& ctx )
{
  SmartIF<IAlgManager> algMgr = serviceLocator()->as<IAlgManager>();
  if ( !algMgr ) {
    return {};
  }
  std::ostringstream names;
  for ( IAlgorithm* alg : algMgr->getAlgorithms() ) {
    if ( alg == nullptr ) {
      continue;
    }
    const AlgExecStateRef state = m_aess->algExecState( alg, ctx );
    if ( state.state() == AlgExecState::State::Done &&
         state.execStatus().isFailure() ) {
      names << ( names.tellp() == 0 ? "" : ", " ) << alg->name();
    }
  }
  return names.str();
}

std::string RpcHiveEventLoopMgr::replyKeyFor( const std::string& sequence ) const
{
  const auto found = m_replyKeys.value().find( sequence );
  return found == m_replyKeys.value().end() ? std::string{} : found->second;
}

StatusCode RpcHiveEventLoopMgr::cannedRequests()
{
  uint64_t requestId = 1;
  for ( const std::string& sequence : m_testRequests ) {
    if ( sequence.empty() ) {
      ATH_MSG_ERROR( "TestRequests contains an empty sequence name" );
      return StatusCode::FAILURE;
    }
    ExecuteRequest request;
    request.requestId = requestId++;
    request.sequence = sequence;
    // A canned request has no client to supply an identity, so it gets the
    // job's properties -- which is all those properties are for. The event
    // number still counts up, so a canned run looks like a sequence of events
    // rather than the same one over and over.
    request.eventId.runNumber = m_runNumber.value();
    request.eventId.lumiBlock = m_lumiBlock.value();
    request.eventId.timeStamp = m_timeStamp.value();
    request.eventId.eventNumber = request.requestId;
    m_canned.push_back( std::move( request ) );
  }
  return StatusCode::SUCCESS;
}

void RpcHiveEventLoopMgr::serverMessage( RpcServer::Level level,
                                         const std::string& text )
{
  // Called from gRPC threads: a MsgStream is not thread-safe, so every message
  // from a foreign thread funnels through here under a lock. The pattern comes
  // from graphics/JiveXML/src/ONCRPCServerSvc.cxx:142-149.
  std::lock_guard<std::mutex> lock( m_messageMutex );
  msg( static_cast<MSG::Level>( level ) ) << "[gRPC] " << text << endmsg;
}

}  // namespace AthExRpc
