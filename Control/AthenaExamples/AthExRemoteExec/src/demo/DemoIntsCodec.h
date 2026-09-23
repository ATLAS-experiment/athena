/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_DEMOINTSCODEC_H
#define ATHEXREMOTEEXEC_DEMOINTSCODEC_H

/**
 * @file DemoIntsCodec.h
 * @brief The demonstration fragment's integer boundary, both directions.
 *
 * A @c protobuf boundary carries a message this package's framework has never
 * heard of, so somebody has to turn it into ordinary StoreGate objects. That
 * somebody is the fragment, and this is the fragment doing it -- one tool that
 * decodes a message into named @c int64_t objects and encodes named @c int64_t
 * objects back into one. Everything between, SumAlg and OffsetAlg and whatever
 * else a fragment puts there, reads and writes those with ordinary typed
 * handles and has never heard of an RemoteExec.
 *
 * This is the odd codec of the five, and it is worth being explicit about why,
 * because the rest of the design deliberately does not accommodate it. Every
 * other encoding *carries* its payload: the bytes are the StoreGate object, so
 * the boundary's key is the object's key and there is exactly one of them. This
 * one *converts*, and the two are then different things -- the boundary is
 * called @c addends on the wire while the objects are @c a and @c b -- and one
 * message can perfectly reasonably become several objects. Rather than make
 * every codec carry the weight of that, the shared base stays at one boundary
 * and this class holds its own handle arrays.
 *
 * The same class serves both directions, which is what makes the two ends of
 * the boundary unable to disagree: the server decodes a request and encodes a
 * reply with it, and an Athena client does the mirror image. @c Direction says
 * which, and only the matching handle array is configured.
 *
 * Entries are matched by *name*, not by position, so a fragment that gains an
 * input does not silently reinterpret the ones it had.
 */

#include "../RemoteExecPayloadCodecBase.h"

#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKeyArray.h"

#include <cstdint>
#include <string>

namespace AthExRemoteExec {

class DemoIntsCodec : public RemoteExecPayloadCodecBase {
public:
  using RemoteExecPayloadCodecBase::RemoteExecPayloadCodecBase;

  virtual const std::string& encoding() const override;

protected:
  virtual StatusCode initializeBoundary() override;

  virtual StatusCode doRecord( const EventContext& ctx, IProxyDict& store,
                               const Payload& payload ) const override;
  virtual StatusCode doRead(
      const EventContext& ctx, IProxyDict& store, Payload& payload,
      const std::vector<const IPayloadCodec*>& alsoCrossing,
      std::vector<std::string>& dangling ) const override;

private:
  /// Ordinary typed handles, so the scheduler learns who produces these the
  /// ordinary way -- nothing dynamic is needed where there is a type to name.
  SG::WriteHandleKeyArray<int64_t> m_decoded{
      this, "Decoded", {},
      "One key per integer this boundary yields, when decoding. The message "
      "names its entries, so these are matched by name rather than position"};
  SG::ReadHandleKeyArray<int64_t> m_encoded{
      this, "Encoded", {},
      "The integers to send, named by their own keys, when encoding"};
};

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_DEMOINTSCODEC_H
