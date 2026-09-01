/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCPAYLOADCODECBASE_H
#define ATHEXRPCLOOP_RPCPAYLOADCODECBASE_H

/**
 * @file RpcPayloadCodecBase.h
 * @brief What every codec has in common, so that no codec repeats it.
 *
 * Deliberately thin. It holds the boundary's three strings and performs the two
 * checks that are the same whatever the encoding:
 *
 * - the payload that arrived declares the encoding this boundary does. Getting
 *   this wrong is the difference between "that is not what you said you would
 *   send" and a codec confidently misreading someone else's bytes, which for a
 *   permissive wire format like protobuf usually succeeds and yields an empty
 *   object;
 * - and, when this boundary states a schema at all, that the payload declares
 *   the same one. A codec that carries exactly one payload type states no
 *   schema and has nothing to compare, which is not a weaker check but the
 *   absence of a check that would be vacuous -- see @c schema() in
 *   IPayloadCodec.h.
 *
 * What it deliberately does *not* hold is any notion of the boundary being one
 * object at one StoreGate key. Most codecs are exactly that, and
 * @c RpcObjectCodecBase adds it for them; a codec that converts between a wire
 * schema and the fragment's own types is not, and should not have to pretend.
 *
 * @c Direction says which way this instance carries. The same codec class runs
 * on both sides of a boundary and in both roles -- the server's gate decodes a
 * request and its pack algorithm encodes the reply, and a client does the
 * mirror image -- so which of the two a given instance is cannot be a property
 * of the class. It matters because it decides whether the keys this codec
 * touches are declared to the scheduler as inputs or as outputs.
 */

#include "IPayloadCodec.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ClassID.h"

#include <string>
#include <vector>

namespace AthExRpc {

class RpcPayloadCodecBase : public extends<AthAlgTool, IPayloadCodec> {
public:
  using extends::extends;

  virtual StatusCode initialize() override;

  virtual const std::string& key() const override final { return m_key; }
  virtual const std::string& schema() const override final { return m_schema; }

  /// The CLID this boundary materialises as, or CLID_NULL for a codec whose
  /// boundary is not one object at one StoreGate key.
  ///
  /// Asked of a boundary by *another* boundary: a codec whose payload can refer
  /// to others -- by link, by index -- has to know which of the things crossing
  /// with it could be the thing referred to, and a key alone does not say. A
  /// converting codec answers CLID_NULL, which is the honest answer rather than
  /// a missing one: its boundary is a name on the wire and nothing is recorded
  /// under it.
  virtual CLID boundaryClid() const { return CLID_NULL; }

  virtual StatusCode record( const EventContext& ctx,
                             const Payload& payload ) const override final;
  virtual StatusCode read(
      const EventContext& ctx, Payload& payload,
      const std::vector<const IPayloadCodec*>& alsoCrossing,
      std::vector<std::string>& dangling ) const override final;

protected:
  /// True if this instance puts objects *into* the store: the server's gate
  /// decoding a request, or a client decoding a reply.
  bool writing() const { return m_writing; }

  /// Hook for a subclass to resolve whatever it needs once the properties are
  /// set. Called from initialize(), after Direction has been validated.
  virtual StatusCode initializeBoundary() { return StatusCode::SUCCESS; }

  /// @c payload has already been checked against this boundary's declaration,
  /// and @c store is the one belonging to @c ctx rather than to this thread.
  /// Both are passed because a codec with a C++ type to name uses ordinary
  /// typed handles, and one without has only the store.
  virtual StatusCode doRecord( const EventContext& ctx, IProxyDict& store,
                               const Payload& payload ) const = 0;

  /// Fill @c payload.data only; the envelope fields are set by the caller.
  virtual StatusCode doRead(
      const EventContext& ctx, IProxyDict& store, Payload& payload,
      const std::vector<const IPayloadCodec*>& alsoCrossing,
      std::vector<std::string>& dangling ) const = 0;

private:
  Gaudi::Property<std::string> m_key{
      this, "Key", "", "What this boundary is called, on both sides"};
  Gaudi::Property<std::string> m_schema{
      this, "Schema", "",
      "What the bytes are, in this encoding's terms; never interpreted by the "
      "framework, and empty for a codec that carries exactly one payload type"};
  Gaudi::Property<std::string> m_direction{
      this, "Direction", "",
      "'Write' if this instance decodes into the store, 'Read' if it encodes "
      "out of it. Decides whether the keys this codec touches are declared to "
      "the scheduler as outputs or as inputs"};

  /// The store belonging to @c ctx. Errors and returns null if there is none.
  IProxyDict* storeOf( const EventContext& ctx ) const;

  bool m_writing = false;
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCPAYLOADCODECBASE_H
