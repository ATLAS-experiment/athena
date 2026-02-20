/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MPIClusterSvc.h"

#include "CxxUtils/XXH.h"
#include "GaudiKernel/FileIncident.h"

#include <boost/serialization/variant.hpp>

StatusCode MPIClusterSvc::initialize() {
  ATH_MSG_DEBUG("Initializing MPI");
  m_env = std::make_unique<mpi3::environment>(mpi3::thread_level::single);
  ATH_MSG_DEBUG("Created MPI environment");
  m_world = m_env->world();
  m_datacom =
      m_world.duplicate();  // make a duplicate communicator for event data
  ATH_MSG_DEBUG("Got MPI_COMM_WORLD");
  m_rank = m_world.rank();
  ATH_MSG_INFO("On MPI rank " << m_rank);

  ATH_CHECK(m_mpiLog.retrieve());
  m_mpiLog->createStatement("PRAGMA foreign_keys = ON").run();

  m_mpiLog
      ->createStatement(
          "CREATE TABLE ranks (rank INTEGER PRIMARY KEY, "
          "node TEXT, start_time FLOAT, end_time FLOAT)")
      .run();
  m_mpiLog
      ->createStatement(
          "INSERT INTO ranks (rank, node, start_time) "
          "VALUES(?1, ?2, julianday('now'))")
      .run(m_rank, m_env->processor_name());
  m_mpiLog->createStatement(
      "CREATE TABLE files (fileId INTEGER PRIMARY KEY, fileName TEXT)")
      .run();
  m_mpiLog
      ->createStatement(
          "CREATE TABLE event_log (rank INTEGER, id INTEGER UNIQUE,"
          "inputFileId INTEGER,"
          "runNumber INTEGER, eventNumber INTEGER, complete INTEGER,"
          "status INTEGER, request_time_ns INTEGER, start_time FLOAT,"
          "end_time FLOAT, PRIMARY KEY (runNumber, eventNumber), "
          "FOREIGN KEY (rank) REFERENCES ranks(rank),"
          "FOREIGN KEY (inputFileId) REFERENCES files(fileId))")
      .run();
  m_mpiLog_addEvent = m_mpiLog->createStatement(
      "INSERT INTO event_log(id, rank, inputFileId, runNumber, eventNumber, complete, "
      "start_time, request_time_ns) "
      "VALUES(?1, ?4, ?6, ?2, ?3, 0, julianday('now'), ?5)");
  m_mpiLog_completeEvent = m_mpiLog->createStatement(
      "UPDATE event_log SET complete = 1, status = ?3, end_time = "
      "julianday('now') WHERE runNumber = ?1 "
      "AND "
      "eventNumber = ?2");
  m_mpiLog_addFile = m_mpiLog->createStatement(
      "INSERT INTO files (fileId, fileName) VALUES(?1, ?2)");

  // Set up incident listener
  ServiceHandle<IIncidentSvc> incsvc("IncidentSvc", this->name());
  if (!incsvc.retrieve().isSuccess()) {
    ATH_MSG_FATAL("Cannot get IncidentSvc.");
    return(StatusCode::FAILURE);
  }
  incsvc->addListener(this, IncidentType::BeginInputFile, 100);
  incsvc->addListener(this, IncidentType::BeginProcessing, 100);

  return StatusCode::SUCCESS;
}

StatusCode MPIClusterSvc::finalize() {
  m_mpiLog
      ->createStatement(
          "UPDATE ranks SET end_time = julianday('now') WHERE rank = ?1")
      .run(m_rank);
  m_env.reset(nullptr);
  return StatusCode::SUCCESS;
}

/// Handles BeginInputFile to keep track of which input file an event came from
void MPIClusterSvc::handle(const Incident& inc) {
  // Fill in slot map at start of every event
  if (inc.type() == IncidentType::BeginProcessing) {
    const std::size_t slot = Gaudi::Hive::currentContext().slot();
    m_inputFileSlotMap[slot] = m_lastInputFileHash;
  }

  // Cache new input filename on start of every file
  if (inc.type() == IncidentType::BeginInputFile) {
    const FileIncident* fileInc = dynamic_cast<const FileIncident*>(&inc);
    if (fileInc == nullptr) {
      ATH_MSG_ERROR("BeginInputFile does not have a file name attached");
      return;
    }

    const std::string fileName = fileInc->fileName();
    // Convert the hash into a signed int64. Just a hash so this doesn't matter.
    m_lastInputFileHash = static_cast<std::int64_t>(xxh3::hash64(fileName));
    m_mpiLog_addFile.run(m_lastInputFileHash, std::move(fileName));
  }
  return;
}


int MPIClusterSvc::numRanks() const {
  return m_world.size();
}

int MPIClusterSvc::rank() const {
  return m_rank;
}

void MPIClusterSvc::barrier() {
  ATH_MSG_DEBUG("Barrier on rank " << rank() << " of " << numRanks());
  m_world.barrier();
}

void MPIClusterSvc::abort() {
  m_world.abort();
}

void MPIClusterSvc::sendMessage(int destRank, ClusterMessage message,
                                ClusterComm communicator) {
  ATH_MSG_DEBUG("Sending message from rank " << rank() << " to " << destRank);
  // Don't send event request message if we're not the master *and* we have a
  // message waiting.
  // Probably an emergency stop message
  if (m_rank != 0 && message.messageType == ClusterMessageType::RequestEvent &&
      m_world.iprobe().has_value()) {
    return;
  }

  // Select correct communicator
  mpi3::communicator& comm =
      (communicator == ClusterComm::EventData) ? m_datacom : m_world;
  if (message.messageType == ClusterMessageType::Data &&
      communicator != ClusterComm::EventData) {
    ATH_MSG_WARNING(
        "Event data should be sent with EventData communicator. "
        "Dropping message");
    return;
  }

  message.source = m_rank;
  const auto& [header, body] = message.wire_msg();
  comm.send_n(header.begin(), header.size(), destRank, 0);
  if (body.has_value()) {
    comm.send_n(body->begin(), body->size(), destRank, header[2]);
    if (message.messageType == ClusterMessageType::Data) {
      const ClusterMessage::WireMsgBody& bdy = *body;
      // Decode the body to figure out what to send
      char* ptr = reinterpret_cast<char*>((std::uint64_t(bdy[0]) << 32) +
                                          std::uint64_t(bdy[1]));
      std::size_t len = (std::uint64_t(bdy[2]) << 32) + std::uint64_t(bdy[3]);

      // Offset the tag by 16384 to minimize chance of conflict
      // (max tag in MPI spec is 32767)
      constexpr int tag_offset = 16384;
      comm.send_n(ptr, len, destRank, header[2] + tag_offset);
    }
  }
}

ClusterMessage MPIClusterSvc::waitReceiveMessage(ClusterComm communicator) {
  // Same offset as line 114
  constexpr int tag_offset = 16384;
  constexpr std::uint64_t thirtytwo_ones = 0xFFFFFFFF;

  // Select correct communicator
  mpi3::communicator& comm =
      (communicator == ClusterComm::EventData) ? m_datacom : m_world;
  ClusterMessage::WireMsg msg{};
  auto&& [head, body] = msg;
  comm.receive_n(head.begin(), head.size());
  // Only time we need to figure out ourselves whether there's a body
  if (head[0] == int(ClusterMessageType::FinalWorkerStatus) ||
      head[0] == int(ClusterMessageType::WorkerError) ||
      head[0] == int(ClusterMessageType::Data)) {
    body = ClusterMessage::WireMsgBody{};
    comm.receive_n(body->begin(), body->size(), head[1], head[2]);
    if (head[0] == int(ClusterMessageType::Data)) {
      ClusterMessage::WireMsgBody& bdy = *body;
      // Decode the body to figure out what to recieve
      std::size_t len = (std::uint64_t(bdy[2]) << 32) + std::uint64_t(bdy[3]);
      std::size_t align = (std::uint64_t(bdy[4]) << 32) + std::uint64_t(bdy[5]);

      char* ptr = static_cast<char*>(std::aligned_alloc(align, len));
      comm.receive_n(ptr, len, head[1], head[2] + tag_offset);

      // update the pointer in the WireMsgBody
      bdy[0] = int(std::uint64_t(ptr) >> 32);
      bdy[1] = int(std::uint64_t(ptr) & thirtytwo_ones);
    }
  }
  ClusterMessage message(msg);
  ATH_MSG_DEBUG("Rank " << rank() << " received message from "
                        << message.source);
  return message;
}

void MPIClusterSvc::log_addEvent(int eventIdx, std::int64_t run_number,
                                 std::int64_t event_number,
                                 std::int64_t request_time_ns,
                                 std::size_t slot) {
  m_mpiLog_addEvent.run(eventIdx, run_number, event_number, m_rank,
                        request_time_ns,
                        m_inputFileSlotMap[slot]);
}

void MPIClusterSvc::log_completeEvent(std::int64_t run_number,
                                      std::int64_t event_number,
                                      std::int64_t status) {
  m_mpiLog_completeEvent.run(run_number, event_number, status);
}
