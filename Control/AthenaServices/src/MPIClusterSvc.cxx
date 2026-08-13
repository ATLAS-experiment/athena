/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MPIClusterSvc.h"

#include <mpi.h>

#include <bit>

#include "CxxUtils/XXH.h"
#include "GaudiKernel/FileIncident.h"

StatusCode MPIClusterSvc::initialize() {
  ATH_MSG_DEBUG("Initializing MPI");
  m_env = std::make_unique<mpi3::environment>(mpi3::thread_level::multiple);
  // Suggestion from Codex (GPT 5.6-sol)
  if (m_env->thread_support() != mpi3::thread_level::multiple) {
    ATH_MSG_ERROR("MPI_THREAD_MULTIPLE is required but unavailable");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Created MPI environment");

  // Print version information
  char version_string[MPI_MAX_LIBRARY_VERSION_STRING];
  int version_string_len;
  MPI_Get_library_version(version_string, &version_string_len);
  ATH_MSG_INFO("Running on {}", version_string);
  m_world = m_env->world();

  if (m_world.size() < 2) {
    ATH_MSG_ERROR("Only have {} ranks! This is insufficient!",
                  m_world.size());
    return StatusCode::FAILURE;
  }
  m_datacom =
      m_world.duplicate();  // make a duplicate communicator for event data
  ATH_MSG_DEBUG("Got MPI_COMM_WORLD");
  m_rank = m_world.rank();
  ATH_MSG_INFO("On MPI rank {}", m_rank);
  const char* env_rank = std::getenv("RANK");
  // Could be nullptr if we're not using MPI to manage events
  if (env_rank != nullptr && env_rank != std::to_string(m_rank)) {
    ATH_MSG_WARNING("MPI rank ({}) does not match $RANK = {}", m_rank,
                    env_rank);
  }

  if (!m_mpiLog.empty()) {
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
    m_mpiLog
        ->createStatement(
            "CREATE TABLE files (fileId INTEGER PRIMARY KEY, fileName TEXT)")
        .run();
    m_mpiLog
        ->createStatement(
            "CREATE TABLE event_log (rank INTEGER, id INTEGER UNIQUE,"
            "inputFileId INTEGER,"
            "runNumber INTEGER, eventNumber INTEGER, complete INTEGER,"
            "status INTEGER, request_time_ns INTEGER, start_time FLOAT,"
            "end_time FLOAT, PRIMARY KEY (runNumber, eventNumber, id), "
            "FOREIGN KEY (rank) REFERENCES ranks(rank),"
            "FOREIGN KEY (inputFileId) REFERENCES files(fileId))")
        .run();
    m_mpiLog_addEvent = m_mpiLog->createStatement(
        "INSERT INTO event_log(id, rank, inputFileId, runNumber, eventNumber, "
        "complete, "
        "start_time, request_time_ns) "
        "VALUES(?1, ?4, ?6, ?2, ?3, 0, julianday('now'), ?5)");
    m_mpiLog_completeEvent = m_mpiLog->createStatement(
        "UPDATE event_log SET complete = 1, status = ?4, end_time = "
        "julianday('now') WHERE runNumber = ?2 "
        "AND eventNumber = ?3 AND id = ?1");
    m_mpiLog_addFile = m_mpiLog->createStatement(
        "INSERT INTO files (fileId, fileName) VALUES(?1, ?2)");
  }
  // Set up incident listener
  ServiceHandle<IIncidentSvc> incsvc("IncidentSvc", this->name());
  if (!incsvc.retrieve().isSuccess()) {
    ATH_MSG_FATAL("Cannot get IncidentSvc.");
    return (StatusCode::FAILURE);
  }
  incsvc->addListener(this, IncidentType::BeginInputFile, 100);
  incsvc->addListener(this, IncidentType::BeginProcessing, 100);

  return StatusCode::SUCCESS;
}

StatusCode MPIClusterSvc::finalize() {
  m_datacom = mpi3::communicator{};  // Ensure this is disconnected before
                                     // shutting down MPI
  m_env.reset(nullptr);
  if (m_mpiLog.empty()) {
    return StatusCode::SUCCESS;
  }
  m_mpiLog
      ->createStatement(
          "UPDATE ranks SET end_time = julianday('now') WHERE rank = ?1")
      .run(m_rank);
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
  ATH_MSG_DEBUG("Barrier on rank {} of {}", rank(), numRanks());
  m_world.barrier();
}

void MPIClusterSvc::abort() {
  m_world.abort();
}

void MPIClusterSvc::sendMessage(int destRank, ClusterMessage message,
                                ClusterComm communicator) {
  ATH_MSG_DEBUG("Sending message from rank {} to {}", rank(), destRank);
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
    const int tag = int(header[2]);
    comm.send_n(body->begin(), body->size(), destRank, tag);
    if (message.messageType == ClusterMessageType::Data) {
      const auto& data = std::get<ClusterMessage::DataDescr>(message.payload);
      // Offset the tag by 16384 to minimize chance of conflict
      // (max tag in MPI spec is 32767)
      constexpr int tag_offset = 16384;
      comm.send_n(static_cast<char*>(data.ptr), data.len, destRank,
                  tag + tag_offset);
    }
  }
}

ClusterMessage MPIClusterSvc::waitReceiveMessage(
    ClusterComm communicator,
    const MemoryResourceRegistry* memoryResourceRegistry) {
  // Same offset as line 114
  constexpr int tag_offset = 16384;
  constexpr std::uint64_t last32 = 0xFFFFFFFF;

  // Select correct communicator
  mpi3::communicator& comm =
      (communicator == ClusterComm::EventData) ? m_datacom : m_world;
  ClusterMessage::WireMsg msg{};
  auto&& [head, body] = msg;
  comm.receive_n(head.begin(), head.size());
  std::pmr::memory_resource* memoryResource = std::pmr::new_delete_resource();
  // Only time we need to figure out ourselves whether there's a body
  if (ClusterMessage::has_body(head)) {
    body = ClusterMessage::WireMsgBody{};
    comm.receive_n(body->begin(), body->size(), head[1], head[2]);
    if (head[0] == int(ClusterMessageType::Data)) {
      ClusterMessage::WireMsgBody& bdy = *body;
      // Decode the body to figure out what to recieve
      std::size_t len = (std::uint64_t(bdy[2]) << 32) + std::uint64_t(bdy[3]);
      // Codex pointed out (impossible barring corruption) point that bdy[4] >=
      // 64 is a problem
      if (bdy[4] >= 64) {
        throw std::runtime_error("Received invalid alignment > 2^64");
      }
      std::size_t align = 1ULL << bdy[4];
      if (len % align != 0) {
        ATH_MSG_ERROR("Received invalid alignment " << align << " for length "
                                                    << len);
        // throw an exception, the allocation will fail and with memory
        // resources we need the correct length later to de-allocate.
        throw std::runtime_error(
            std::format("Received invalid cluster message. {} is not a valid "
                        "alignment for length {}!",
                        align, len));
      }
      auto dest = Destination(bdy[5]);

      if (memoryResourceRegistry == nullptr) {
        if (dest != Destination::Host) {
          ATH_MSG_WARNING("Ignoring destination " << int(dest)
                                                  << " because no memory "
                                                     "resource registry was "
                                                     "provided");
        }
        dest = Destination::Host;
        bdy[5] = 0;
      } else if (int(dest) >= memoryResourceRegistry->size()) {
        ATH_MSG_ERROR(
            "Received message for destination "
            << int(dest)
            << " which is not valid for this rank. Assuming CPU memory.");
        dest = Destination::Host;
        bdy[5] = std::uint32_t(dest);  // So later decode works
      } else {
        memoryResource = memoryResourceRegistry->at(std::uint32_t(dest));
      }
      char* ptr = static_cast<char*>(memoryResource->allocate(len, align));
      comm.receive_n(ptr, len, head[1], head[2] + tag_offset);

      // update the pointer in the WireMsgBody
      bdy[0] = int(std::uint64_t(ptr) >> 32);
      bdy[1] = int(std::uint64_t(ptr) & last32);
    }
  }
  ClusterMessage message(msg, memoryResource);
  ATH_MSG_DEBUG("Rank {} received message from {}", rank(), message.source);
  return message;
}

void MPIClusterSvc::log_addEvent(int eventIdx, std::int64_t run_number,
                                 std::int64_t event_number,
                                 std::int64_t request_time_ns,
                                 std::size_t slot) {
  if (m_mpiLog.empty()) {
    ATH_MSG_WARNING("MPI SQLite log service is not setup!");
  }
  m_mpiLog_addEvent.run(eventIdx, run_number, event_number, m_rank,
                        request_time_ns, m_inputFileSlotMap[slot]);
}

void MPIClusterSvc::log_completeEvent(int eventIdx, std::int64_t run_number,
                                      std::int64_t event_number,
                                      std::int64_t status) {
  if (m_mpiLog.empty()) {
    ATH_MSG_WARNING("MPI SQLite log service is not setup!");
  }
  m_mpiLog_completeEvent.run(eventIdx, run_number, event_number, status);
}
