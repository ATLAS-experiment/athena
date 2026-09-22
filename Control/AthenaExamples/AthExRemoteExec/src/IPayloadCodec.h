/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_IPAYLOADCODEC_H
#define ATHEXREMOTEEXEC_IPAYLOADCODEC_H

/**
 * @file IPayloadCodec.h
 * @brief One boundary, and the mechanism that carries it.
 *
 * A codec is an @c AthAlgTool configured with the boundary it serves, so a
 * boundary is a *component* rather than a string parsed at initialize(). Gaudi's
 * component factory is the registry: there is no encoding-to-implementation
 * lookup anywhere, no process-wide table, and nothing whose behaviour depends on
 * which libraries happened to load first.
 *
 * Three strings describe a boundary and none of them is enumerated anywhere:
 *
 * - @c key      — what the boundary is called, on both sides. For most codecs
 *   this is also the StoreGate key, because the payload *is* the object; for one
 *   that converts between a wire schema and the fragment's own types it is only
 *   a name on the wire, and the objects live under keys of the fragment's
 *   choosing;
 * - @c encoding — which mechanism carries it. A property of the C++ class, not
 *   of the configuration: a codec answers to exactly one tag, which is what
 *   makes the tag a contract rather than a setting;
 * - @c schema   — what the bytes are, in that mechanism's own terms, and never
 *   interpreted by the framework. Optional, because it is only worth stating
 *   when one codec component can carry more than one thing: a protobuf codec
 *   needs a message name, whereas a codec that carries exactly one payload type
 *   is already named completely by its encoding tag.
 *
 * **A codec declares what it touches.** Whatever a codec records is an ordinary
 * StoreGate object that ordinary algorithms read through ordinary handles, so
 * the scheduler has to know who produces it. The codec is the only thing that
 * knows, and it says so itself -- with plain @c SG::WriteHandleKey members where
 * it has a C++ type to name, and, where it has none because being type-agnostic
 * is the whole point, by declaring a key built from a CLID it resolved at
 * initialize(). Nothing about this is restated in the configuration and nothing
 * hand-builds an ExtraOutputs.
 *
 * Adding a boundary type costs nothing. Adding an encoding costs one tool and
 * one line of configuration, in whichever library wants it — and the core of
 * this package stays free of ROOT, of xAOD and of any detector, because the
 * codecs that need those live elsewhere and are only ever named by the job.
 *
 * Implementations are retrieved once and then called from every slot
 * concurrently, so every method here must be thread-safe and const.
 */

#include "RemoteExecProtocol.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

#include <string>
#include <vector>

namespace AthExRemoteExec {

class IPayloadCodec : virtual public IAlgTool {
public:
  DeclareInterfaceID( IPayloadCodec, 1, 0 );

  /// The tag this codec answers to in Payload::encoding. Fixed by the class.
  virtual const std::string& encoding() const = 0;

  /// The name of the boundary this instance serves.
  virtual const std::string& key() const = 0;

  /// What the bytes are, in this encoding's terms. May be empty.
  virtual const std::string& schema() const = 0;

  /// Decode @c payload into the event's store.
  virtual StatusCode record( const EventContext& ctx,
                             const Payload& payload ) const = 0;

  /**
   * @brief Read this boundary's object back out and encode it into @c payload.
   *
   * @param alsoCrossing every codec sending in the same direction, this one
   *        included. A codec whose payloads can *refer* to other payloads --
   *        by StoreGate key, by index, by link -- uses this to report, in
   *        @c dangling, references to keys that are not in the set. It is not
   *        an error: a sender may legitimately send an object whose references
   *        are not meant to be followed on the far side. It is a fact only the
   *        sender can notice, because only the sender has both the object and
   *        the list of what is going with it, and which otherwise shows up on
   *        the far side as an absence rather than a failure. A codec carrying
   *        independent bytes ignores both arguments.
   */
  virtual StatusCode read( const EventContext& ctx, Payload& payload,
                           const std::vector<const IPayloadCodec*>& alsoCrossing,
                           std::vector<std::string>& dangling ) const = 0;
};

/// Join what read() reported into one message. Here so that both senders --
/// the server's pack algorithm and the client's request algorithm -- say the
/// same thing.
std::string describeDangling( const std::vector<std::string>& dangling );

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_IPAYLOADCODEC_H
