/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECWIRE_REMOTEEXECCLIENT_H
#define ATHREMOTEEXECWIRE_REMOTEEXECCLIENT_H

/**
 * @file Client.h
 * @brief Minimal blocking client; the transport part of RequestAlg.
 *
 * The interface for RPC client. Should be implemented using one of the RPC
 * frameworks (e.g. gRPC or Cap'n Proto) and be used by RequestAlg
 * (see Control/AthRemoteExec/AthRemoteExecCore). It consumes the request, waits
 * for the reply from the server, and returns it back.
 */

#include <AthRemoteExecWire/Protocol.h>

#include <memory>
#include <string>
#include <vector>

namespace AthRemoteExec {

class Client {
 public:
  /// @param target host:port, e.g. "localhost:50051"
  explicit Client(const std::string& target);
  ~Client();

  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;

  /// Consume request, block and wait util the server replies, return reply.
  ExecuteReply execute(const ExecuteRequest& request);

  /// List of available sequences. Empty on failure, with @c error describing
  /// what went wrong.
  std::vector<SequenceInfo> listSequences(std::string* error = nullptr);

  /// Block until the server answers a ListSequences call or @c timeoutSeconds
  /// elapses. Lets a test client start before the server is up.
  bool waitUntilReady(double timeoutSeconds);

 private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECWIRE_REMOTEEXECCLIENT_H
