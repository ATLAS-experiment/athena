/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCREQUESTALG_H
#define ATHEXRPCLOOP_RPCREQUESTALG_H

/**
 * @file RpcRequestAlg.h
 * @brief Calls a remote fragment from an ordinary Athena job.
 *
 * The client-side mirror of RpcGateAlg and RpcPackAlg together: it reads the
 * declared inputs out of the event store, sends them, and records what comes
 * back under the declared output keys. Between those two points the caller's
 * job looks like any other -- upstream algorithms write ordinary objects,
 * downstream algorithms read ordinary objects, and neither knows a network was
 * involved.
 *
 * Like the gate, it enumerates no types. A boundary is a key, an encoding and
 * a schema name resolved at run time, which is what lets this one algorithm
 * send a detector's data without containing a line of that detector's code: the
 * job configuration names the boundary, and something else entirely put the
 * object in the store. That is the whole reason the boundary is declared as
 * strings.
 *
 * Its codecs declare what they touch, as the gate's and the pack algorithm's
 * do, so the scheduler orders this algorithm correctly without the
 * configuration restating anything. Here they run the other way round: the ones
 * carrying the request read, and the ones carrying the reply write.
 *
 * This is deliberately the simple client: one blocking call per event, no
 * retry, no pipelining. A refusal (the server is busy or shutting down) is
 * reported as a distinct failure rather than retried, because how long to wait
 * and how many times is a policy decision belonging to whatever drives the job,
 * not to this algorithm.
 */

#include "IPayloadCodec.h"
#include "RpcClient.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"

#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace AthExRpc {

class RpcRequestAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  Gaudi::Property<std::string> m_target{
      this, "Target", "",
      "Server as \"host:port\", or the path of a file the server writes its "
      "port into, which is how a test starts the two in either order"};

  Gaudi::Property<double> m_readyTimeout{
      this, "ReadyTimeout", 120.0,
      "Seconds to wait in initialize() for the server to answer. A server that "
      "loads a detector description takes minutes to become ready, so failing "
      "immediately would only mean every job had to be started in order"};

  Gaudi::Property<std::string> m_sequence{
      this, "SequenceName", "", "Fragment to ask the server to run"};

  ToolHandleArray<IPayloadCodec> m_inputs{
      this, "Inputs", {},
      "One codec per boundary to send. Each reads its key from the event store "
      "and encodes it; the fragment sees the same key on the far side"};

  ToolHandleArray<IPayloadCodec> m_outputs{
      this, "Outputs", {},
      "One codec per boundary to expect back, recorded under the same keys. "
      "These are the *same* configured components the server's fragment uses, "
      "which is what makes the two ends unable to disagree"};

  /// References pointing outside the payload set are a configuration fact, so
  /// they are reported on the first request that has any and not again.
  mutable std::atomic<bool> m_warnedDangling{false};

  /// The input codecs as raw pointers; see RpcPackAlg for why.
  std::vector<const IPayloadCodec*> m_crossing;

  /// Shared by every slot. gRPC stubs are documented as safe for concurrent
  /// calls, and one channel per thread would defeat the connection pooling the
  /// transport does for us.
  mutable std::unique_ptr<RpcClient> m_client ATLAS_THREAD_SAFE;
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCREQUESTALG_H
