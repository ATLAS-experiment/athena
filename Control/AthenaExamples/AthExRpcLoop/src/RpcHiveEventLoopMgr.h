/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCHIVEEVENTLOOPMGR_H
#define ATHEXRPCLOOP_RPCHIVEEVENTLOOPMGR_H

/**
 * @file RpcHiveEventLoopMgr.h
 * @brief Multi-threaded event loop manager driven by gRPC requests.
 *
 * Each request becomes a real event in its own whiteboard slot, pushed to the
 * Avalanche scheduler. N slots means N requests in flight, and requests for
 * different fragments run concurrently in different slots.
 *
 * Structurally this is AthenaHiveEventLoopMgr with the event selector replaced
 * by a request queue, plus three differences that follow from being a server:
 *
 * - The fragment to run is chosen by control flow, not by name. The manager
 *   only records the request into the slot store; RpcGateAlg does the
 *   selecting (see RpcGateAlg.h).
 * - The slot store must outlive the event, because the reply is read out of it
 *   after the scheduler is done. So this manager deliberately does *not*
 *   subscribe to the EndAlgorithms incident that AthenaHiveEventLoopMgr uses to
 *   clear slots; it clears and frees them itself once the reply is in hand.
 * - Every request gets a reply, whatever happens -- unknown sequence, bad
 *   payload, algorithm failure, timeout or shutdown. The rule is borrowed from
 *   HltEventLoopMgr::failedEvent.
 *
 * Three threads, which is one more than it looks and two more than a batch
 * loop needs. A server has two independent things worth blocking on -- the next
 * request and the next completion -- and one thread can only block on one of
 * them. An earlier single-threaded version blocked on completions, so a request
 * arriving while it waited was not accepted until the next event finished; at
 * two concurrent clients that ran requests almost serially. So:
 *
 *   input thread   blocks in RpcServer::pop, queues requests, wakes the loop
 *   output thread  blocks in IScheduler::popFinishedEvent, queues finished
 *                  events, wakes the loop
 *   the loop       waits for either and does all the work
 *
 * All Gaudi interaction -- whiteboard, StoreGate, incidents, replies -- stays
 * on the loop thread. The two helpers only move pointers across a mutex, which
 * is what makes the split cheap to reason about.
 */

#include "RpcServer.h"
#include "RpcWire.h"

#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/IConditionsCleanerSvc.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SmartIF.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class EventContext;
class StoreGateSvc;

namespace AthExRpc {

class RpcHiveEventLoopMgr : public extends<MinimalEventLoopMgr, IEventProcessor>,
                            public AthMessaging {
public:
  RpcHiveEventLoopMgr( const std::string& name, ISvcLocator* svcLoc );
  virtual ~RpcHiveEventLoopMgr() override;

  virtual StatusCode initialize() override;
  virtual StatusCode stop() override;
  virtual StatusCode finalize() override;

  /// Serve requests. @c maxevt < 0 serves until the server is stopped,
  /// otherwise at most @c maxevt requests are accepted.
  virtual StatusCode nextEvent( int maxevt ) override;

  virtual StatusCode executeRun( int maxevt ) override;

  /// Ask the loop to stop accepting work and drain what is in flight.
  virtual StatusCode stopRun() override;

  // Both AthMessaging and the Gaudi Service base offer these; pick one, the
  // way AthenaEventLoopMgr does.
  using AthMessaging::msg;
  using AthMessaging::msgLvl;
  virtual const std::string& name() const override { return Service::name(); }

private:
  using Clock = std::chrono::steady_clock;

  /// A request that has been given a slot and handed to the scheduler.
  struct InFlight {
    ExecuteRequest request;
    /// The gRPC job whose promise must be fulfilled. Null in canned test mode,
    /// where there is no client to answer.
    std::unique_ptr<RpcServer::Job> job;
    Clock::time_point deadline;
    bool hasDeadline = false;
    /// Set once the client has been answered. A request that timed out is
    /// answered early but keeps its slot until the event really finishes.
    bool answered = false;
  };

  /// Where the next request came from.
  enum class Take {
    Job,      ///< got one
    None,     ///< none available right now
    Stopped   ///< no more requests will ever come
  };

  /// A finished event, with the moment the scheduler gave it back. The
  /// timestamp is taken by the output thread, because the loop may be busy
  /// completing an earlier event when this one lands and charging that delay
  /// to the scheduler would hide it.
  struct Finished {
    EventContext* ctx = nullptr;
    Clock::time_point at;
  };

  /// Bring up the request source: the canned list, or a listening server.
  StatusCode startServing();

  /// Blocks in RpcServer::pop; runs until the server stops.
  void inputLoop();

  /// Blocks in IScheduler::popFinishedEvent whenever an event is in flight.
  void outputLoop();

  /// Start/stop the two helper threads. Idempotent.
  void startHelpers();
  void stopHelpers();

  /// Take the next request, without waiting. Requests come from the input
  /// thread, or from the canned list when there is no server.
  Take takeRequest( ExecuteRequest& request,
                    std::unique_ptr<RpcServer::Job>& job );

  /// Take whatever the output thread has collected, without waiting.
  std::vector<Finished> takeFinished();

  /// Answer anything the loop never got to, and release any finished event it
  /// never collected. Called once both helpers have stopped.
  void drainPending();

  /// Reject requests that cannot possibly run, before a slot is spent on them.
  AthExRpc::Status validate( const ExecuteRequest& request, std::string& detail ) const;

  /// Give a request a slot and push it to the scheduler, or answer it outright.
  StatusCode dispatch( ExecuteRequest request,
                       std::unique_ptr<RpcServer::Job> job );

  /// Read the result of one finished event and release its slot.
  StatusCode complete( const Finished& finished );

  /// Turn a finished event into the reply for its request.
  ExecuteReply buildReply( const EventContext& ctx, const InFlight& entry );

  /// Answer any in-flight request that has run out of time. The slot is kept
  /// until the event finishes, so the scheduler is never left inconsistent.
  void expireDeadlines();

  /// Fulfil a request's promise (or log it, in canned mode). Idempotent.
  void respond( InFlight& entry, ExecuteReply reply );

  /// Answer a request that never got as far as a slot.
  void respondDirect( const ExecuteRequest& request,
                      const std::unique_ptr<RpcServer::Job>& job,
                      AthExRpc::Status status,
                      const std::string& detail );

  /// Names of the algorithms that failed in @c ctx, for the reply detail.
  std::string failedAlgorithms( const EventContext& ctx );

  /// The reply key of @c sequence, or an empty string if it declares none.
  std::string replyKeyFor( const std::string& sequence ) const;

  /// Requests built from the TestRequests property, for tests with no client.
  StatusCode cannedRequests();

  /// Thread-safe log sink handed to the gRPC server.
  void serverMessage( RpcServer::Level level, const std::string& text );

  ServiceHandle<StoreGateSvc> m_evtStore{this, "EvtStore", "StoreGateSvc",
                                         "Event store to inject payloads into"};
  ServiceHandle<IIncidentSvc> m_incidentSvc{this, "IncidentSvc", "IncidentSvc",
                                            "Incident service"};
  /// Ages conditions objects out of the store. A server is long-lived and sees
  /// many runs, so without this the conditions containers grow for as long as
  /// the process is up -- one object per IOV, never collected. The standard
  /// Hive and MTES managers call it once per event; so does this one.
  ServiceHandle<Athena::IConditionsCleanerSvc> m_conditionsCleaner{
      this, "ConditionsCleanerSvc", "Athena::ConditionsCleanerSvc",
      "Conditions garbage collector, called once per request"};

  Gaudi::Property<std::string> m_whiteboardName{
      this, "WhiteboardSvc", "EventDataSvc", "Hive whiteboard service"};
  Gaudi::Property<std::string> m_schedulerName{
      this, "SchedulerSvc", "AvalancheSchedulerSvc", "Scheduler service"};

  SmartIF<IHiveWhiteBoard> m_whiteboard;
  SmartIF<IScheduler> m_scheduler;
  SmartIF<IAlgExecStateSvc> m_aess;

  Gaudi::Property<std::vector<std::string>> m_sequences{
      this, "Sequences", {},
      "Names of the algorithm sequences (fragments) this server offers"};
  Gaudi::Property<std::map<std::string, std::vector<std::string>>> m_inputKeys{
      this, "Inputs", {},
      "Per sequence, the boundaries the client must supply, each as "
      "\"key#encoding#schema\". Advertised by ListSequences, and used to "
      "reject requests that do not match before they cost a slot"};
  Gaudi::Property<std::map<std::string, std::vector<std::string>>> m_outputKeys{
      this, "Outputs", {},
      "Per sequence, the boundaries returned to the client. Advertised by "
      "ListSequences; the actual read-back is done by the fragment's pack "
      "algorithm"};
  Gaudi::Property<std::map<std::string, std::string>> m_replyKeys{
      this, "ReplyKeys", {},
      "Per sequence, the key its pack algorithm stages the reply under. A "
      "sequence with no entry here returns no outputs"};
  Gaudi::Property<std::string> m_requestKey{
      this, "RequestKey", "RpcRequest",
      "StoreGate key the request is recorded under, where the gates read it"};

  Gaudi::Property<std::string> m_address{this, "Address", "0.0.0.0",
                                         "Interface the server binds to"};
  Gaudi::Property<int> m_port{this, "Port", 0,
                              "TCP port to bind; 0 lets the OS choose, in "
                              "which case the bound port is logged"};
  Gaudi::Property<std::string> m_portFile{
      this, "PortFile", "",
      "If set, the bound port is written to this file once the server is up. "
      "Lets a test client discover a port chosen by the OS"};
  Gaudi::Property<int> m_maxMessageBytes{
      this, "MaxMessageBytes", 256 * 1024 * 1024,
      "Largest request or reply the server will accept. gRPC's own default is "
      "4 MB, which realistic payloads exceed easily"};
  Gaudi::Property<size_t> m_queueLimit{
      this, "QueueLimit", 128,
      "Maximum number of queued requests before clients are told the server "
      "is overloaded"};

  Gaudi::Property<std::vector<std::string>> m_testRequests{
      this, "TestRequests", {},
      "Sequence names to serve instead of starting a server, one request each. "
      "Mirrors AthenaMtesEventLoopMgr's ESTestPilotMessages, and exercises "
      "everything except the socket: slots, gates, control flow, replies and "
      "every error path.\n"
      "\n"
      "They carry no payloads, and cannot: a payload's bytes are its "
      "fragment's own schema, and this service has no way to build one without "
      "knowing that schema -- which is precisely what it must not know. A "
      "canned request for a fragment with inputs is therefore answered "
      "INVALID_REQUEST, which is itself worth testing. Payload paths are "
      "exercised by a client"};

  /// Serving canned requests, with no socket and no client.
  bool cannedMode() const { return !m_testRequests.empty(); }

  Gaudi::Property<double> m_requestTimeout{
      this, "RequestTimeout", 0.0,
      "Seconds before an in-flight request is answered with TIMEOUT. The event "
      "keeps running and its slot is released normally when it finishes; only "
      "the client stops waiting. 0 disables the timeout"};
  Gaudi::Property<unsigned int> m_pollInterval{
      this, "PollInterval", 5,
      "Milliseconds between scheduler polls, and so the latency a reply can "
      "wait for. Only used when RequestTimeout is set: without deadlines to "
      "notice, the loop blocks on completions instead of polling at all"};

  // Only canned requests use these. A request that arrives over the wire must
  // carry its own identity and is rejected if it does not: conditions are
  // resolved against it, so pinning it to a property would resolve every
  // request against one run's conditions whatever run it came from.
  Gaudi::Property<unsigned int> m_runNumber{
      this, "RunNumber", 1, "Run number given to canned test requests"};
  Gaudi::Property<unsigned int> m_lumiBlock{
      this, "LumiBlock", 1, "Lumi block given to canned test requests"};
  Gaudi::Property<unsigned int> m_timeStamp{
      this, "TimeStamp", 0, "Time stamp given to canned test requests"};

  Gaudi::Property<bool> m_logEveryRequest{
      this, "LogEveryRequest", true,
      "Log a line per request as it is dispatched and answered. On by default "
      "because it is how the tests observe the server, but a production "
      "deployment wants it off: formatting two MsgStreams per request is "
      "measurable next to the rest of the in-process cost, and it is the "
      "first thing to turn off before quoting a throughput number"};
  Gaudi::Property<bool> m_reportTiming{
      this, "ReportTiming", true,
      "Log the in-process phase table when the loop finishes. Costs eight "
      "clock reads and a push_back per request; see RpcTiming.h for what the "
      "phases mean and what they deliberately exclude"};

  std::unique_ptr<RpcServer> m_server;
  std::deque<ExecuteRequest> m_canned;
  std::map<size_t, InFlight> m_inFlight;  ///< keyed by whiteboard slot
  size_t m_nextEvent = 0;
  size_t m_completed = 0;

  // --- the input/output split ---------------------------------------------
  // One mutex, because the loop waits for "either queue non-empty" and two
  // mutexes cannot be waited on together. Two condition variables, because the
  // loop and the output thread wait for different things.
  std::thread m_inputThread;
  std::thread m_outputThread;
  std::mutex m_pendingMutex;
  std::condition_variable m_pending;      ///< loop waits here
  std::condition_variable m_inFlightCv;   ///< output thread waits here
  std::condition_variable m_incomingRoom; ///< input thread waits here
  std::deque<std::unique_ptr<RpcServer::Job>> m_incoming;
  /// How deep m_incoming is allowed to get. It is a hand-off buffer, not the
  /// queue: requests beyond this stay in RpcServer's own queue, which is where
  /// QueueLimit can see them and reject. An unbounded m_incoming would drain
  /// that queue empty and leave the server with no backpressure at all.
  size_t m_incomingLimit = 1;
  std::deque<Finished> m_finishedEvents;
  /// Events pushed to the scheduler but not yet handed back. Guarded by
  /// m_pendingMutex: it is the output thread's wait predicate, and calling
  /// popFinishedEvent when nothing is in flight would spin.
  size_t m_awaitingCompletion = 0;
  bool m_inputDone = false;      ///< the server will produce no more requests
  bool m_helpersStopping = false;

  std::mutex m_messageMutex;  ///< serialises logging from gRPC threads
  std::atomic<bool> m_stopRequested{false};
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCHIVEEVENTLOOPMGR_H
