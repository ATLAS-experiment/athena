/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCORE_REMOTEEXECHIVEEVENTLOOPMGR_H
#define ATHREMOTEEXECCORE_REMOTEEXECHIVEEVENTLOOPMGR_H

/**
 * @file RemoteExecHiveEventLoopMgr.h
 * @brief Multi-threaded event loop manager driven by RPC requests.
 *
 * Each request becomes a real event in its own whiteboard slot, pushed to the
 * Avalanche scheduler. N slots for N requests -> requests for different
 * sequences are run concurrently in different slots.
 *
 * Structurally this is AthenaHiveEventLoopMgr with the event selector replaced
 * by a request queue, plus differences that follow from it being a server:
 *
 * - The sequence to run is chosen by control flow, not by name. The manager
 *   only records the request into the slot store; InputCnvAlg does the
 *   selecting (see InputCnvAlg.h).
 * - The slot store must outlive the event, because the reply is read out of it
 *   after the scheduler is done. So this manager deliberately defers
 *   the base class's EndAlgorithms clearing until eventFinished() has taken
 *   the reply. The base then clears and frees the slot.
 * - Every request gets a reply, similar to HltEventLoopMgr::failedEvent.
 *
 * RemoteExecHiveEventLoopMgr runs three threads:
 * - input thread:  receives and queues the requests, see @c ::inputLoop
 * - output thread: pops finished event from the queue and sends them, see
 *                  @c ::outputLoop
 * - loop:          waits for either and does all the work
 *
 * All Gaudi interactions: whiteboard, StoreGate, incidents, replies stay
 * on the loop thread. The two helpers only move pointers across a mutex.
 */

#include <AthRemoteExecWire/Protocol.h>
#include <AthRemoteExecWire/Server.h>

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

#include "AthenaServices/AthenaHiveEventLoopMgr.h"
#include "Gaudi/Property.h"

class EventContext;
class StoreGateSvc;

namespace AthRemoteExec {

class RemoteExecHiveEventLoopMgr : public AthenaHiveEventLoopMgr {
 public:
  RemoteExecHiveEventLoopMgr(const std::string& name, ISvcLocator* svcLoc);
  virtual ~RemoteExecHiveEventLoopMgr() override;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /// Serve requests. @c maxevt < 0 serves until the server is stopped,
  /// otherwise at most @c maxevt requests are accepted.
  virtual StatusCode nextEvent(int maxevt) override;

  /// Ask the loop to stop accepting work and drain what is in flight.
  virtual StatusCode stopRun() override;

 protected:
  StatusCode eventFinished(const EventContext& ctx) override;
  bool clearStoreOnEndAlgorithms() const override { return false; }

  /// Where the next request came from.
  enum class Take {
    Job,     ///< got one
    None,    ///< none available right now
    Stopped  ///< no more requests will ever come
  };

  /// Take the next request from the incoming event queue.
  virtual Take takeRequest(ExecuteRequest& request,
                           std::unique_ptr<Server::Job>& job);

  /// Create and start the server.
  virtual StatusCode startServing();

 private:
  using Clock = std::chrono::steady_clock;

  /// A request that has been given a slot and handed to the scheduler.
  struct InFlight {
    ExecuteRequest request;
    /// The RPC job whose promise must be fulfilled.
    std::unique_ptr<Server::Job> job;
    Clock::time_point deadline;
    bool hasDeadline = false;
    /// Set once the client has been answered. A request that timed out is
    /// answered early but keeps its slot until the event really finishes.
    bool answered = false;
  };

  /// A finished event, from the scheduler's point of view. The timestamp is
  /// taken by the output thread, as the loop may be busy completing an earlier
  /// event.
  struct Finished {
    EventContext* ctx = nullptr;
    Clock::time_point at;
  };

  /// Loop and wait for incoming job. Push it onto incoming event queue and
  /// notify main loop.
  void inputLoop();

  /// Loop and wait for finished job. Push it onto finished event queue and
  /// notify main loop.
  void outputLoop();

  /// Starts input/output loops.
  void startHelpers();

  /// Stops input/output loops.
  void stopHelpers();

  /// Take whatever the output thread has collected from finished event queue.
  std::vector<Finished> takeFinished();

  /// Answer to anything that the loop did not manage to finish.
  void drainPending();

  /// Reject requests that cannot possibly run, before a slot is spent on them.
  AthRemoteExec::Status validate(const ExecuteRequest& request,
                                 std::string& diagnostics) const;

  /// Give a request a slot and push it to the scheduler.
  StatusCode dispatch(ExecuteRequest request,
                      std::unique_ptr<Server::Job> job);

  /// Read the result of one finished event and release its slot.
  StatusCode complete(const Finished& finished);

  /// Turn a finished event into the reply for its request.
  ExecuteReply buildReply(const EventContext& ctx, const InFlight& entry);

  /// Reply to all request that passed their deadline. The slot is kept until
  /// the event finishes.
  void expireDeadlines();

  /// Reply to the request.
  void respond(InFlight& entry, ExecuteReply reply);

  /// Reply to the request that never got assigned to slot.
  void respondDirect(const ExecuteRequest& request,
                     const std::unique_ptr<Server::Job>& job,
                     AthRemoteExec::Status status,
                     const std::string& diagnostics);

  /// Names of the algorithms that failed in @c ctx. For the reply diagnostics.
  std::string failedAlgorithms(const EventContext& ctx);

  /// The reply key of @c sequence, or an empty string if it declares none.
  std::string replyKeyFor(const std::string& sequence) const;

  /// Thread-safe log sink handed to the RPC server.
  void serverMessage(Server::Level level, const std::string& text);

  // clang-format off

  Gaudi::Property<std::vector<std::string>> m_sequences{
      this, "Sequences", {}, "Names of the algorithm sequences (fragments) this server offers"};
  Gaudi::Property<std::map<std::string, std::vector<std::string>>> m_inputKeys{
      this, "Decoders", {}, "Per sequence, The boundaries the client must supply."};
  Gaudi::Property<std::map<std::string, std::vector<std::string>>> m_outputKeys{
      this, "Encoders", {}, "Per sequence, the boundaries returned to the client."};
  Gaudi::Property<std::map<std::string, std::string>> m_replyKeys{
      this, "ReplyKeys", {}, "Per sequence, the key its pack algorithm stages the reply under."};
  Gaudi::Property<std::string> m_requestKey{
      this, "RequestKey", "RemoteExecRequest", "StoreGate key that the request is recorded under."};

  Gaudi::Property<std::string> m_address{
      this, "Address", "0.0.0.0", "Interface the server binds to"};
  Gaudi::Property<int> m_port{
      this, "Port", 0, "TCP port to bind. 0 lets the OS choose."};
  Gaudi::Property<std::string> m_portFile{
      this, "PortFile", "", "If set, the bound port is written to this file once the server is up. Lets a test client discover a port chosen by the OS"};
  Gaudi::Property<int> m_maxMessageBytes{
      this, "MaxMessageBytes", 256 * 1024 * 1024, "Largest request or reply the server will accept."};
  Gaudi::Property<size_t> m_queueLimit{
      this, "QueueLimit", 128, "Maximum number of queued requests before clients are told the server is overloaded"};

  Gaudi::Property<double> m_requestTimeout{
      this, "RequestTimeout", 0.0, "Seconds before an in-flight request is answered with TIMEOUT. 0 disables the timeout."};
  Gaudi::Property<unsigned int> m_pollInterval{
      this, "PollInterval", 5, "Milliseconds between scheduler polls."};

  Gaudi::Property<bool> m_reportTiming{
      this, "ReportTiming", true, ""};

  // clang-format on

  std::unique_ptr<Server> m_server;
  std::map<size_t, InFlight> m_inFlight;  ///< keyed by whiteboard slot
  size_t m_nextEvent = 0;
  size_t m_completed = 0;

  std::thread m_inputThread;
  std::thread m_outputThread;
  std::mutex m_pendingMutex;
  std::condition_variable m_pending;       ///< loop waits here
  std::condition_variable m_inFlightCv;    ///< output thread waits here
  std::condition_variable m_incomingRoom;  ///< input thread waits here
  std::deque<std::unique_ptr<Server::Job>> m_incoming;
  size_t m_incomingLimit = 1;
  std::deque<Finished> m_finishedEvents;
  size_t m_awaitingCompletion = 0;
  bool m_inputDone = false;
  bool m_helpersStopping = false;

  std::mutex m_messageMutex;
  std::atomic<bool> m_stopRequested{false};
};

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECCORE_REMOTEEXECHIVEEVENTLOOPMGR_H
