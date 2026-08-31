/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCPACKALG_H
#define ATHEXRPCLOOP_RPCPACKALG_H

/**
 * @file RpcPackAlg.h
 * @brief Last algorithm of every fragment: stages the reply for the loop manager.
 *
 * The mirror image of RpcGateAlg. It reads the fragment's declared outputs and
 * records one RpcReplyStaging under its own key, which the loop manager picks
 * up once the scheduler reports the event finished.
 *
 * Each fragment has its own reply key, so exactly one algorithm produces each
 * one and the scheduler sees an unambiguous graph. Only the selected fragment
 * runs, so only its reply object ever exists in a given event.
 *
 * Like the gate, it enumerates no types: an output is a key, an encoding and a
 * schema name, resolved at run time. Because those keys cannot be typed handle
 * keys, the configuration declares them in ExtraInputs so the scheduler still
 * orders this algorithm after whatever produced them.
 */

#include "RpcPayloadStore.h"
#include "RpcReplyStaging.h"
#include "RpcRequestDescriptor.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <atomic>
#include <string>
#include <vector>

namespace AthExRpc {

class RpcPackAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::ReadHandleKey<RpcRequestDescriptor> m_request{
      this, "Request", "RpcRequest",
      "The request, for the correlation id to echo back"};
  SG::WriteHandleKey<RpcReplyStaging> m_reply{
      this, "Reply", "", "Where the loop manager will look for this reply"};

  Gaudi::Property<std::vector<std::string>> m_outputs{
      this, "Outputs", {},
      "This fragment's outputs, each as \"key#encoding#schema\""};

  /// References pointing outside the payload set are a configuration fact, so
  /// they are reported on the first reply that has any and not again.
  mutable std::atomic<bool> m_warnedDangling{false};

  ServiceHandle<IClassIDSvc> m_clidSvc{
      this, "ClassIDSvc", "ClassIDSvc",
      "Resolves a boundary's schema to a CLID, for codecs that need it"};

  std::vector<PayloadStore::Resolved> m_boundaries;
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCPACKALG_H
