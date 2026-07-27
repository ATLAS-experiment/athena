// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHEXMPI_REMOTEGPUSVC_H
#define ATHEXMPI_REMOTEGPUSVC_H

// Local include(s)
#include "HostDevicePtr.h"
#include "RPCFunctionEntry.h"

// Athena include(s)
#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"
#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/ClusterMessage.h"
#include "AthenaKernel/IMPIClusterSvc.h"
#include "CxxUtils/XXH.h"

// Gaudi include(s)
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ServiceHandle.h"

// Boost include(s)
#include <boost/fiber/future.hpp>

// TBB include(s)
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_set.h>

// System include(s)
#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
#include <thread>

namespace RemoteCall {

const std::size_t ServerRank = 0;  // For now server is always rank 0
const unsigned int GPUDestID = 1;  // For now, GPU is always device 1

struct ServerRequestID {
  std::size_t rank;
  std::size_t request;
  auto operator<=>(const ServerRequestID& rhs) const = default;
};
}  // namespace RemoteCall

template <>
struct std::hash<RemoteCall::ServerRequestID> {
  std::size_t operator()(
      const RemoteCall::ServerRequestID& reqID) const noexcept {
    return xxh3::hash64(reqID);
  }
};

namespace RemoteCall {
/// Skeleton service for asynchronous remote GPU communication over MPI.
class RemoteGPUSvc : public AthService {
 public:
  /// Inherit the base class constructor(s).
  using AthService::AthService;

  /// Start the server or client thread for this rank.
  virtual StatusCode initialize() override;

  /// Prevent further registration and MPI barrier
  virtual StatusCode start() override;

  /// Stop and join the background thread for this rank.
  virtual StatusCode finalize() override;

  /// Return whether this service instance runs the rank-zero server.
  bool isServer() const;

  /// Register an RPC function
  /// Clients must still have the callable built and available,
  /// but don't need to be able to run it.
  template <RPCCallable F>
  StatusCode registerFunction(F&& f, std::string name);

  /// Submit a request and suspend the asynchronous algorithm for its result.
  /// This will be called from a multithreaded context therefore it is marked
  /// const. Thread-safety is ensured by using a concurrent container for the
  /// request map
  template <RPCSupported... R, RPCSupported... Args>
  std::tuple<std::unique_ptr<R>...> call(
      const EventContext& ctx, const AthAsynchronousAlgorithm& algorithm,
      std::string_view functionName, const Args*... args) const;

 private:
  /// Description of a request on client side
  class ClientRequest {
   public:
    explicit ClientRequest(EventContext::ContextID_t eventNumber,
                           const RPCFunctionEntry* rpcFunction)
        : eventNumber(eventNumber),
          rpcFunction(rpcFunction),
          m_requestID(s_requestIDCtr++) {
      returnVals.reserve(rpcFunction->returnVals().size());
    }

    std::size_t id() const noexcept { return m_requestID; }
    EventContext::ContextID_t eventNumber;
    const RPCFunctionEntry* rpcFunction;
    std::vector<void*> returnVals;
    std::size_t argsSent = 0;
    boost::fibers::promise<std::unique_ptr<ClientRequest>> completion;

   private:
    std::size_t m_requestID;
    static std::atomic_size_t s_requestIDCtr;
  };

  /// Description of a request on server side
  class ServerRequest {
   public:
    explicit ServerRequest(EventContext::ContextID_t eventNumber,
                           const RPCFunctionEntry* rpcFunction)
        : eventNumber(eventNumber), rpcFunction(rpcFunction) {
      argData.reserve(rpcFunction->arguments().size());
      args.reserve(rpcFunction->arguments().size());
    }

    EventContext::ContextID_t eventNumber;
    const RPCFunctionEntry* rpcFunction = nullptr;
    std::vector<ClusterMessage::DataDescr> argData;
    std::vector<void*> args;
  };

  /// Run the worker-rank remote GPU client.
  StatusCode runClient();

  /// Run the rank-zero remote GPU server.
  StatusCode runServer();

  /// Start request
  void startRequest(const ClientRequest& req) const;

  /// Send a request argument
  template <RPCSupported T>
  void sendArg(ClientRequest& req, const T* arg) const;

  /// MPI service used to communicate with the client ranks.
  ServiceHandle<IMPIClusterSvc> m_clusterSvc{
      this, "MPIClusterSvc", "MPIClusterSvc", "MPI cluster service"};

  /// Background communication thread.
  std::jthread m_thread;

  /// Requests waiting for completion on a client
  mutable tbb::concurrent_hash_map<std::size_t, std::unique_ptr<ClientRequest>>
      m_clientRequests;

  /// Requests waiting for completion on server. Here the key is both rank and
  /// request id.
  tbb::concurrent_hash_map<ServerRequestID, std::unique_ptr<ServerRequest>>
      m_serverRequests;

  /// Registry for callable functions
  tbb::concurrent_unordered_set<RPCFunctionEntry,
                                /*Hash =*/RPCFunctionEntryHash>
      m_functionRegistry;

  /// Indicate when registration is permitted
  bool m_registrationPermitted = true;

  /// Indicate when the process is done
  std::atomic_bool m_processDone = false;

  /// Result returned by the background communication thread.
  StatusCode m_status{StatusCode::SUCCESS};
};

}  // namespace RemoteCall

#ifndef ATHEXMPI_REMOTEGPUSVC_ICC
#include "RemoteGPUSvc.icc"
#endif

#endif  // ATHEXMPI_REMOTEGPUSVC_H
