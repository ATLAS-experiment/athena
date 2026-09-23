/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_DEMODOUBLESCODEC_H
#define ATHEXREMOTEEXEC_DEMODOUBLESCODEC_H

/**
 * @file DemoDoublesCodec.h
 * @brief The demonstration fragment's vector-of-doubles boundary.
 *
 * The same shape as DemoIntsCodec -- see there for why a converting codec is
 * the odd one of the five -- for a message that carries one named vector rather
 * than several named scalars. One key each way, so this one happens not to fan
 * out; that is a property of the message, not of the mechanism.
 *
 * The name travels with the values, and on the way out it is the StoreGate key
 * the vector came from, so a receiver that cares can tell what it was called
 * without the fragment restating it.
 */

#include "../RemoteExecPayloadCodecBase.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <string>
#include <vector>

namespace AthExRemoteExec {

class DemoDoublesCodec : public RemoteExecPayloadCodecBase {
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
  SG::WriteHandleKey<std::vector<double>> m_decoded{
      this, "Decoded", "", "Where the decoded values go"};
  SG::ReadHandleKey<std::vector<double>> m_encoded{
      this, "Encoded", "", "The values to send"};
};

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_DEMODOUBLESCODEC_H
