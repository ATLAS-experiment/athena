/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCPAYLOADCODEC_H
#define ATHEXRPCLOOP_RPCPAYLOADCODEC_H

/**
 * @file RpcPayloadCodec.h
 * @brief What an encoding is, and how one is added.
 *
 * A boundary is three strings -- @c key#encoding#schema -- and nothing here
 * enumerates any of them. @c encoding names a *mechanism* and is looked up in
 * the registry below; @c schema identifies, in that mechanism's own terms, what
 * the bytes are. The framework never interprets a schema. Adding a boundary
 * type costs nothing; adding a whole encoding costs one implementation of
 * @c IPayloadCodec and one call to @c registerCodec, in whichever library
 * wants it.
 *
 * All three fields are required. An earlier version let the encoding be omitted
 * and inferred it from the C++ type -- "is this trivially copyable?" -- which
 * is the wrong question in two ways: it decides a *wire format* from a *host*
 * property, and it makes a boundary's serialisation change silently when its
 * type does. A boundary now says what it is.
 *
 * The framework itself defines exactly one encoding, @c protobuf (see
 * RpcBlobCodec.h): the bytes are opaque, they are recorded under the boundary's
 * key as an AthExRpc::RpcBlob, and the fragment that published the schema
 * converts them in an ordinary algorithm of its own. That is deliberate.
 * Turning a message into an EDM object is irreducibly per-type, so it belongs
 * to the fragment that wants it rather than to the transport -- and it keeps
 * the core of this package free of ROOT, of xAOD, and of any detector.
 *
 * A codec that carries dictionary-backed C++ objects between two Athena
 * processes is a natural second one, and lives outside this library: it links
 * ROOT, this does not, and the registry is the seam that keeps it that way.
 */

#include "RpcWire.h"

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/StatusCode.h"

#include <string>
#include <vector>

class IClassIDSvc;
class IProxyDict;

namespace AthExRpc::PayloadCodec {

/// One end of a fragment boundary, as declared by the configuration.
struct Boundary {
  std::string key;       ///< StoreGate key
  std::string encoding;  ///< which codec carries it
  std::string schema;    ///< what the bytes are, in that codec's terms
};

/**
 * @brief Parse a @c "key#encoding#schema" declaration.
 *
 * '#' rather than ':' or ',' because a schema may be a C++ type name, and
 * those contain both ("std::vector<double>", "std::map<int,int>").
 *
 * All three fields are required and none may be empty.
 */
bool parseBoundary( const std::string& spec, Boundary& boundary );

class IPayloadCodec;

/// A declared boundary, with its codec found and its CLID looked up. Produced
/// once at initialize() so that neither happens per request.
struct Resolved {
  Boundary boundary;
  CLID clid = CLID_NULL;
  const IPayloadCodec* codec = nullptr;
};

/**
 * @brief One serialisation mechanism.
 *
 * Implementations are stateless and outlive the job -- they are looked up once
 * per boundary at initialize() and then called from every slot concurrently, so
 * every method here must be thread-safe and const.
 */
class IPayloadCodec {
public:
  virtual ~IPayloadCodec() = default;

  /// The tag this codec answers to in Payload::encoding.
  virtual const std::string& encoding() const = 0;

  /**
   * @brief The CLID of the StoreGate object this boundary materialises as.
   *
   * Not always derivable from the schema: an opaque-bytes codec records every
   * boundary under one type whatever the schema says, while a codec carrying
   * C++ objects would look the schema up as a type name. Called once per
   * boundary at initialize().
   */
  virtual StatusCode clidFor( const Boundary& boundary, IClassIDSvc& clidSvc,
                              CLID& clid, std::string& error ) const = 0;

  /// Decode @c payload and record it under @c resolved.boundary.key.
  virtual StatusCode record( IProxyDict& store, IClassIDSvc& clidSvc,
                             const Resolved& resolved, const Payload& payload,
                             std::string& error ) const = 0;

  /**
   * @brief Read the boundary's object back out and encode it into @c payload.
   *
   * @param alsoCrossing everything else being sent in the same direction, or
   *        nullptr to skip the check. A codec whose payloads can *refer* to
   *        other payloads -- by StoreGate key, by index, by link -- uses this
   *        to report, in @c dangling, references to keys that are not in the
   *        set. It is not an error: a sender may legitimately send an object
   *        whose references are not meant to be followed on the far side. It is
   *        a fact only the sender can notice, because only the sender has both
   *        the object and the list of what is going with it, and which
   *        otherwise shows up on the far side as an absence rather than a
   *        failure. A codec carrying independent bytes ignores both arguments.
   */
  virtual StatusCode read( IProxyDict& store, const Resolved& resolved,
                           Payload& payload, std::string& error,
                           const std::vector<Resolved>* alsoCrossing,
                           std::vector<std::string>* dangling ) const = 0;
};

/**
 * @brief Make @c codec available under its own encoding tag.
 *
 * @c codec must outlive the job; the intended implementation is a function-local
 * static in the library that owns it. Registering a second codec for the same
 * tag fails, and is a configuration error rather than a race: registration
 * happens as libraries load, lookup happens at initialize().
 */
StatusCode registerCodec( const IPayloadCodec& codec );

/// The codec for @c encoding, or nullptr if nothing has registered one.
const IPayloadCodec* findCodec( const std::string& encoding );

/// Every registered tag, for the "this server cannot read X" message.
std::vector<std::string> knownEncodings();

}  // namespace AthExRpc::PayloadCodec

#endif  // ATHEXRPCLOOP_RPCPAYLOADCODEC_H
