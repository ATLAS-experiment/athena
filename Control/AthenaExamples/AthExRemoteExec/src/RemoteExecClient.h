/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_REMOTEEXECCLIENT_H
#define ATHEXREMOTEEXEC_REMOTEEXECCLIENT_H

/**
 * @file RemoteExecClient.h
 * @brief Minimal blocking client; the transport half of RemoteExecRequestAlg.
 *
 * Its only caller is RemoteExecRequestAlg, which needs *some* stub and must not see
 * generated protobuf headers (RemoteExecProtocol.h explains why); this is that stub with
 * the generated code kept on the other side of the wire library.
 *
 * It is not the way to talk to the server. The server is driven by the .proto
 * contract alone, so any gRPC client speaking the schema works just as well --
 * see test/remote_exec_menu_client.cxx for one that uses neither this class nor any
 * other part of Athena.
 */

#include "RemoteExecProtocol.h"

#include <memory>
#include <string>
#include <vector>

namespace AthExRemoteExec {

class RemoteExecClient {
public:
  /// @param target host:port, e.g. "localhost:50051"
  explicit RemoteExecClient( const std::string& target );
  ~RemoteExecClient();

  RemoteExecClient( const RemoteExecClient& ) = delete;
  RemoteExecClient& operator=( const RemoteExecClient& ) = delete;

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

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_RPCCLIENT_H
