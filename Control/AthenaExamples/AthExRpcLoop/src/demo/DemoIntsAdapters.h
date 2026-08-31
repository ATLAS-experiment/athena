/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_DEMOINTSADAPTERS_H
#define ATHEXRPCLOOP_DEMOINTSADAPTERS_H

/**
 * @file DemoIntsAdapters.h
 * @brief The two halves of the demonstration fragment's integer boundary.
 *
 * A @c protobuf boundary arrives in the event store as an AthExRpc::RpcBlob:
 * bytes and a schema name, unopened, because opening it is the fragment's
 * business and not the framework's. These are the fragment doing that business.
 *
 * @c DemoIntsUnpackAlg parses one and records ordinary @c int64_t objects;
 * @c DemoIntsPackAlg reads ordinary @c int64_t objects and produces one. Both
 * use ordinary typed handles, so everything between them -- SumAlg, OffsetAlg,
 * whatever a fragment puts there -- is an unmodified Athena algorithm that has
 * never heard of an RPC.
 *
 * Declared in one header because they are exact mirrors and are always read
 * together, and because the same pair runs on *both* sides of the boundary: the
 * server unpacks a request and packs a reply, an Athena client packs a request
 * and unpacks the reply. One implementation, used four ways, is why the two
 * ends cannot disagree about what the bytes mean.
 */

#include "../RpcBlob.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandleKeyArray.h"

#include <cstdint>

namespace AthExRpc {

/// Bytes in, named integers out.
class DemoIntsUnpackAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::ReadHandleKey<RpcBlob> m_payload{
      this, "Payload", "", "The unopened payload, as the gate recorded it"};
  SG::WriteHandleKeyArray<int64_t> m_values{
      this, "Values", {},
      "One key per integer the message carries. The message names its entries, "
      "so these are matched by name rather than by position: a fragment that "
      "gains an input does not silently reinterpret the ones it had"};
};

/// Named integers in, bytes out.
class DemoIntsPackAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::ReadHandleKeyArray<int64_t> m_values{
      this, "Values", {}, "The integers to send, named by their own keys"};
  SG::WriteHandleKey<RpcBlob> m_payload{
      this, "Payload", "", "Where to leave the encoded message"};
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_DEMOINTSADAPTERS_H
