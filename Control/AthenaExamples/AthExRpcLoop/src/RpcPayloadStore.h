/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCPAYLOADSTORE_H
#define ATHEXRPCLOOP_RPCPAYLOADSTORE_H

/**
 * @file RpcPayloadStore.h
 * @brief Moving payloads between the wire and StoreGate, generically.
 *
 * This is where the genericity stops. Everything upstream of it deals in bytes,
 * encodings and schema names; everything downstream of it -- every payload
 * algorithm in every fragment -- deals in ordinary typed handles and has no
 * idea an RPC is involved. Once record() has run, a payload is an ordinary
 * StoreGate object under an ordinary key with an ordinary CLID,
 * indistinguishable from one a WriteHandle put there, so fragment algorithms
 * stay shareable with a normal Athena job.
 *
 * Nothing here knows any encoding. Each boundary's codec is found once by
 * resolve() and cached in the Resolved, and these three functions are a facade
 * over it: shared by the gate, the pack algorithm and the client-side request
 * algorithm, so there is exactly one implementation of "payload in" and
 * "payload out" and all three say the same thing when it goes wrong.
 */

#include "RpcPayloadCodec.h"
#include "RpcWire.h"

#include "GaudiKernel/StatusCode.h"

#include <string>
#include <vector>

class IClassIDSvc;
class IProxyDict;

namespace AthExRpc::PayloadStore {

using Resolved = PayloadCodec::Resolved;

/**
 * @brief Parse "key#encoding#schema" declarations and find their codecs.
 *
 * Done once at initialize() so that a typo in a fragment declaration, or an
 * encoding no library in this job implements, stops the job rather than failing
 * every request.
 */
StatusCode resolve( const std::vector<std::string>& specs, IClassIDSvc& clidSvc,
                    std::vector<Resolved>& resolved, std::string& error );

/// Decode @c payload and record it under the boundary's key.
StatusCode record( IProxyDict& store, IClassIDSvc& clidSvc,
                   const Resolved& resolved, const Payload& payload,
                   std::string& error );

/**
 * @brief Read the boundary's object back out and encode it into @c payload.
 *
 * @param alsoCrossing everything else being sent in the same direction, or
 *        nullptr to skip the check; @c dangling collects references it makes to
 *        keys not in that set. See IPayloadCodec::read for what that means and
 *        why it is a warning rather than an error.
 */
StatusCode read( IProxyDict& store, const Resolved& resolved, Payload& payload,
                 std::string& error,
                 const std::vector<Resolved>* alsoCrossing = nullptr,
                 std::vector<std::string>* dangling = nullptr );

/// Join what read() reported into one message. Here rather than in each caller
/// so that both senders say the same thing.
std::string describeDangling( const std::vector<std::string>& dangling );

}  // namespace AthExRpc::PayloadStore

#endif  // ATHEXRPCLOOP_RPCPAYLOADSTORE_H
