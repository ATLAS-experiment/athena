/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#pragma once
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
#include <atomic>
#include <vector>

#include "TestMessages.grpc.pb.h"
#include "TestMessages.pb.h"

/**
 * @class OffloadToken
 * @brief Asynchronous gRPC request/response handle with completion
 * synchronization.
 *
 * This class represents a single asynchronous RPC transaction. It owns the
 * request, response, and gRPC client context objects, and provides a
 * lightweight mechanism to track completion of the remote call.
 *
 * Conceptually, it behaves similarly to a simplified `std::future`, combining:
 *  - request/response storage,
 *  - asynchronous execution via gRPC callbacks,
 *  - a synchronization primitive for waiting on completion.
 *
 * @details
 * The OffloadToken encapsulates the full lifecycle of an RPC call:
 *  - A request is prepared and sent via `sendRequest()`.
 *  - A callback is invoked by gRPC when the response arrives.
 *  - The callback updates internal state and signals completion.
 *  - The caller may optionally wait for completion using `waitForResponse()`.
 *
 * The class ensures that all objects required by the asynchronous call
 * (request, response, and ClientContext) remain valid for the duration
 * of the RPC, preventing lifetime-related bugs.
 *
 * This design is suitable for high-throughput asynchronous environments
 * (e.g. AthenaMT), where blocking operations should be avoided and
 * work is offloaded to external services.
 *
 * @note
 * The internal synchronization mechanism acts as a completion event and
 * allows multiple waiting threads to be notified when the response is ready.
 *
 * @warning
 * Instances are not inherently thread-safe for concurrent modification.
 * Access patterns should ensure:
 *  - request is finalized before sending,
 *  - response is read only after completion notification.
 *
 * @par Design Pattern
 * This class implements a combination of:
 *  - Token / Handle pattern
 *  - Future-like asynchronous result holder
 *  - Callback-based continuation
 */
class OffloadToken {
 public:
  OffloadToken() {
    m_request = std::make_unique<TestInputData>();
    m_response = std::make_unique<TestOutputData>();
    m_context = std::make_unique<grpc::ClientContext>();
  }

  void responseReceived() const { 
    m_ready.test_and_set();
    m_ready.notify_all(); }

  void waitForResponse() const {
    m_ready.wait(false, std::memory_order_relaxed);
  }

  std::unique_ptr<TestInputData>& request() { return m_request; }

  std::unique_ptr<TestOutputData>& response() { return m_response; }
  const std::unique_ptr<TestOutputData>& response() const { return m_response; }

  std::unique_ptr<grpc::ClientContext>& clinetContext() { return m_context; }
  const std::unique_ptr<grpc::ClientContext>& clinetContext() const {
    return m_context;
  }

  void sendRequest(TestService::Stub* stub) {
    std::cout << "Sending request " << m_request->id() << "\n";
    stub->async()->doComputation(
        m_context.get(), m_request.get(), m_response.get(),
        [this](grpc::Status status) {
          if (status.ok()) {
            std::cout << "Response received for request " << m_request->id()
                      << " id of response " << m_response->id() << "\n";
          }
          this->responseReceived();
        });
  }

 private:
  mutable std::atomic_flag m_ready{ATOMIC_FLAG_INIT};

  std::unique_ptr<TestInputData> m_request;
  mutable std::unique_ptr<TestOutputData> m_response;
  mutable std::unique_ptr<grpc::ClientContext> m_context;
};

CLASS_DEF(OffloadToken, 38722758, 1)