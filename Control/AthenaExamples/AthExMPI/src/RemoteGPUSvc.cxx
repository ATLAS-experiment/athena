// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s)
#include "RemoteGPUSvc.h"

// Athena include(s)
#include "AthenaKernel/ClusterMessage.h"
#include "CxxUtils/XXH.h"

// MPI3 include(s)
#include "mpi3/communicator.hpp"

// System include(s)
#include <chrono>
#include <cstddef>
#include <exception>
#include <memory>
#include <set>
#include <utility>

namespace RemoteCall {

// Initialize counter
std::atomic_size_t RemoteGPUSvc::ClientRequest::s_requestIDCtr{0};

StatusCode RemoteGPUSvc::initialize() {
  ATH_CHECK(m_clusterSvc.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode RemoteGPUSvc::start() {
  m_registrationPermitted = false;
  std::size_t ref_size = m_functionRegistry.size();
  std::size_t hash_sum = 0;
  RPCFunctionEntryHash hash{};
  for (auto&& it : m_functionRegistry) {
    hash_sum += hash(it);
  }

  std::size_t ref_hash_sum = hash_sum;
  m_clusterSvc->data_communicator().broadcast_value(ref_size, ServerRank);
  m_clusterSvc->data_communicator().broadcast_value(ref_hash_sum, ServerRank);
  // Now ref_size and ref_hash_sum were received from server
  bool success = true;
  if (ref_size != m_functionRegistry.size() || ref_hash_sum != hash_sum) {
    success = false;
  }
  success = m_clusterSvc->data_communicator().all_reduce_value(success);
  m_clusterSvc->barrier();

  if (!success) {
    ATH_MSG_ERROR("Function registry differs between ranks!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Starting remote GPU "
               << (m_clusterSvc->rank() == 0 ? "server" : "client")
               << " on MPI rank " << m_clusterSvc->rank());
  m_thread = std::jthread([this]() {
    try {
      m_status = m_clusterSvc->rank() == 0 ? runServer() : runClient();
    } catch (const std::exception& error) {
      ATH_MSG_ERROR("Remote GPU communication failed with an exception: "
                    << error.what());
      m_status = StatusCode::FAILURE;
    } catch (...) {
      ATH_MSG_ERROR(
          "Remote GPU communication failed with an unknown "
          "exception");
      m_status = StatusCode::FAILURE;
    }
  });
  return StatusCode::SUCCESS;
}

StatusCode RemoteGPUSvc::finalize() {
  m_processDone = true;

  ATH_MSG_INFO("Waiting for the remote GPU communication thread to stop");
  if (m_thread.joinable()) {
    m_thread.join();
  }

  if (!isServer()) {
    ATH_MSG_DEBUG("Notifying the remote GPU server that worker rank "
                  << m_clusterSvc->rank() << " has stopped");
    m_clusterSvc->sendMessage(0, ClusterMessage(ClusterMessageType::EventsDone),
                              ClusterComm::EventData);
  }
  return m_status;
}

bool RemoteGPUSvc::isServer() const {
  return m_clusterSvc->rank() == ServerRank;
}

std::optional<unsigned int> RemoteGPUSvc::registerDevice(
    std::pmr::memory_resource* deviceMemoryResource) {
  if (!m_registrationPermitted) {
    ATH_MSG_ERROR("Device registration may only occur during initialization");
    return std::nullopt;
  }
  m_memoryResources.push_back(deviceMemoryResource);
  return m_memoryResources.size() - 1;
}

StatusCode RemoteGPUSvc::runClient() {
  using namespace std::chrono_literals;
  ATH_MSG_INFO("Remote GPU client thread is running on MPI rank "
               << m_clusterSvc->rank());

  while (!m_processDone) {
    if (m_clientRequests.empty()) {
      std::this_thread::sleep_for(5ms);
      continue;  // busy loop if we don't have any in-flight requests
    }
    ClusterMessage msg =
        m_clusterSvc->waitReceiveMessage(ClusterComm::EventData);
    if (msg.messageType != ClusterMessageType::Data ||
        msg.source != ServerRank) {
      ATH_MSG_ERROR("Received unexpected message of type "
                    << std::format("{}", msg.messageType) << " from "
                    << msg.source);
      return StatusCode::FAILURE;
    }
    auto desc = std::move(std::get<ClusterMessage::DataDescr>(msg.payload));

    // Retrieve the request
    decltype(m_clientRequests)::accessor acc;
    if (!m_clientRequests.find(acc, desc.requestNumber)) {
      ATH_MSG_ERROR("Received response for request "
                    << desc.requestNumber
                    << " which was not sent, or is complete!");
      return StatusCode::FAILURE;
    }
    auto& [id, request] = *acc;
    request->returnVals.push_back(desc.release());

    // If complete, set the promise and remove the request
    if (request->returnVals.size() ==
        request->rpcFunction->returnVals().size()) {
      // Move the promise out so we can move the unique_ptr in
      auto promise = std::move(request->completion);
      promise.set_value(std::move(request));
      m_clientRequests.erase(acc);
    }
  }

  ATH_MSG_INFO("Remote GPU client thread has stopped");
  return StatusCode::SUCCESS;
}

StatusCode RemoteGPUSvc::runServer() {
  ATH_MSG_INFO("Remote GPU server thread is running");

  std::set<int> finishedWorkers;
  const std::size_t numWorkers = m_clusterSvc->numRanks() - 1;
  while (finishedWorkers.size() < numWorkers) {
    ClusterMessage message = m_clusterSvc->waitReceiveMessage(
        ClusterComm::EventData, &m_memoryResources);

    if (message.messageType == ClusterMessageType::EventsDone) {
      finishedWorkers.insert(message.source);
      ATH_MSG_DEBUG("Worker rank " << message.source << " stopped; waiting for "
                                   << numWorkers - finishedWorkers.size()
                                   << " worker(s)");
      continue;
    }

    if (message.messageType == ClusterMessageType::Data) {
      auto desc =
          std::move(std::get<ClusterMessage::DataDescr>(message.payload));

      // Retrieve / create request
      ServerRequestID reqID{.rank = static_cast<size_t>(message.source),
                            .request = desc.requestNumber};
      decltype(m_serverRequests)::accessor acc;
      if (m_serverRequests.insert(acc, reqID)) {
        // Newly inserted --> data pointer is to an std::size_t containing hash
        // of function name
        auto fn_iter =
            m_functionRegistry.find(*static_cast<std::size_t*>(desc.ptr));
        if (fn_iter == m_functionRegistry.end()) {
          // Really shouldn't happen, we verify that the registries are
          // identical
          ATH_MSG_ERROR("Received request for unrecognized function");
          return StatusCode::FAILURE;
        }
        auto& [id, request] = *acc;
        request = std::make_unique<ServerRequest>(desc.evtNumber, &*fn_iter);
      } else {
        // Already exists, this is an argument
        auto& [id, request] = *acc;
        request->args.push_back(desc.ptr);
        // Necessary because DataDescr manages the memory
        request->argData.push_back(std::move(desc));
        // Run if ready
        if (request->args.size() == request->rpcFunction->arguments().size()) {
          // TODO: Do this in a smarter way
          std::jthread(
              [&clusterSvc = this->m_clusterSvc](
                  ServerRequestID id,
                  std::unique_ptr<ServerRequest>&& request) {
                std::vector<void*> result =
                    request->rpcFunction->wrapper()(request->args);
                // Send back the results
                for (std::size_t i = 0; i < result.size(); ++i) {
                  const auto& argDef = request->rpcFunction->returnVals()[i];
                  ClusterMessage::DataDescr desc(result[i], argDef.len,
                                                 argDef.align);
                  desc.dest =
                      Destination::Host;  // Returned results always go to CPU
                  desc.evtNumber = request->eventNumber;
                  desc.requestNumber = id.request;
                  ClusterMessage msg(ClusterMessageType::Data, std::move(desc));
                  clusterSvc->sendMessage(int(id.rank), std::move(msg),
                                          ClusterComm::EventData);
                }
              },
              id, std::move(request))
              .detach();
          // Don't need to hold on to the request any more
          m_serverRequests.erase(acc);
        }
      }
      continue;  // Next message
    }

    ATH_MSG_ERROR("Ignoring unexpected remote GPU message type "
                  << static_cast<int>(message.messageType) << " from rank "
                  << message.source);
  }

  ATH_MSG_INFO("All worker ranks have stopped; stopping remote GPU server");
  return StatusCode::SUCCESS;
}

void RemoteGPUSvc::startRequest(const ClientRequest& req) const {
  std::size_t name_hash = xxh3::hash64(req.rpcFunction->name());
  ClusterMessage::DataDescr desc(&name_hash);
  desc.dest = Destination::Host;  // name always goes to CPU
  desc.evtNumber = req.eventNumber;
  desc.requestNumber = req.id();
  m_clusterSvc->sendMessage(
      ServerRank, ClusterMessage(ClusterMessageType::Data, std::move(desc)),
      ClusterComm::EventData);
}

}  // namespace RemoteCall
