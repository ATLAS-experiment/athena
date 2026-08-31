/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCCLIENT_H
#define ATHEXRPCLOOP_RPCCLIENT_H

/**
 * @file RpcClient.h
 * @brief Minimal blocking client; the transport half of RpcRequestAlg.
 *
 * Its only caller is RpcRequestAlg, which needs *some* stub and must not see
 * generated protobuf headers (RpcWire.h explains why); this is that stub with
 * the generated code kept on the other side of the wire library.
 *
 * It is not the way to talk to the server. The server is driven by the .proto
 * contract alone, so any gRPC client speaking the schema works just as well --
 * see test/rpc_menu_client.cxx for one that uses neither this class nor any
 * other part of Athena.
 */

#include "RpcWire.h"

#include <memory>
#include <string>
#include <vector>

namespace AthExRpc {

class RpcClient {
public:
  /// @param target host:port, e.g. "localhost:50051"
  explicit RpcClient( const std::string& target );
  ~RpcClient();

  RpcClient( const RpcClient& ) = delete;
  RpcClient& operator=( const RpcClient& ) = delete;

  /// Block until the server replies. @c detail carries the transport error if
  /// the call itself failed, in which case the returned status is Unspecified.
  ExecuteReply execute( const ExecuteRequest& request );

  /// Empty on failure; @c error, if given, receives the transport error.
  std::vector<SequenceInfo> listSequences( std::string* error = nullptr );

  /// Block until the server answers a ListSequences call or @c timeoutSeconds
  /// elapses. Lets a test client start before the server is up.
  bool waitUntilReady( double timeoutSeconds );

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCCLIENT_H
