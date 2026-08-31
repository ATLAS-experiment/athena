/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCGATEALG_H
#define ATHEXRPCLOOP_RPCGATEALG_H

/**
 * @file RpcGateAlg.h
 * @brief First algorithm of every fragment: selects it and unpacks its inputs.
 *
 * The scheduler owns whole-event control flow, so a request cannot pick a
 * sub-graph by name the way a serial dispatcher could. The supported mechanism
 * is control-flow gating: every fragment is a seqAND whose first member is one
 * of these. If the request does not name this fragment the gate fails the
 * filter and the rest of the seqAND is skipped.
 *
 * When the fragment *is* selected, the gate decodes the request payload into
 * the fragment's input keys. Two consequences worth being explicit about:
 *
 * - It makes the gate the producer of those keys, so the data-flow graph is
 *   closed and the scheduler can validate the wiring. Because the types are not
 *   known at compile time the keys cannot be typed handle keys, so the
 *   configuration declares them in ExtraOutputs instead; RpcFragments does both
 *   from one declaration.
 * - The genericity stops here. Downstream everything is an ordinary StoreGate
 *   object under an ordinary CLID, read through ordinary typed handles, so
 *   payload algorithms are RPC-ignorant and shareable with a normal job.
 *
 * There is one class, instantiated once per fragment, and it enumerates no
 * types at all: a boundary is a key, an encoding and a schema name, all
 * resolved at run time. It knows only the request name that selects it and the
 * boundaries it has to materialise.
 */

#include "RpcPayloadStore.h"
#include "RpcRequestDescriptor.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include <string>
#include <vector>

namespace AthExRpc {

class RpcGateAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  Gaudi::Property<std::string> m_sequence{
      this, "SequenceName", "",
      "Name of the fragment this gate guards. A request naming any other "
      "sequence fails the filter"};

  SG::ReadHandleKey<RpcRequestDescriptor> m_request{
      this, "Request", "RpcRequest", "The request, injected by the loop manager"};

  Gaudi::Property<std::vector<std::string>> m_inputs{
      this, "Inputs", {},
      "This fragment's inputs, each as \"key#encoding#schema\" -- e.g. "
      "\"addends#protobuf#athexrpc.demo.v1.Ints\""};

  ServiceHandle<IClassIDSvc> m_clidSvc{
      this, "ClassIDSvc", "ClassIDSvc",
      "Resolves a boundary's schema to a CLID, for codecs that need it"};

  std::vector<PayloadStore::Resolved> m_boundaries;
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCGATEALG_H
