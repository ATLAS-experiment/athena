/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_DEMODOUBLESCODEC_H
#define ATHEXREMOTEEXEC_DEMODOUBLESCODEC_H

/**
 * @file DemoDoublesCodec.h
 * @brief The demonstration of codec de/encoding vector-of-doubles.
 */

#include <AthRemoteExecCore/PayloadCodecBase.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <string>
#include <vector>

namespace AthRemoteExec::Example {

class DemoDoublesCodec : public PayloadCodecBase {
 public:
  using PayloadCodecBase::PayloadCodecBase;

  virtual const std::string& encoding() const override;

 protected:
  virtual StatusCode initializeBoundary() override;

  virtual StatusCode doDecode(const EventContext& ctx, IProxyDict& store,
                              const Payload& payload) const override;
  virtual StatusCode doEncode(
      const EventContext& ctx, IProxyDict& store, Payload& payload,
      const std::vector<const IPayloadCodec*>& alsoCrossing,
      std::vector<std::string>& dangling) const override;

 private:
  SG::WriteHandleKey<std::vector<double>> m_decoded{this, "Decoded", "",
                                                    "Decoded values key"};
  SG::ReadHandleKey<std::vector<double>> m_encoded{this, "Encoded", "",
                                                   "Encoded values key"};
};

}  // namespace AthRemoteExec::Example

#endif  // ATHEXREMOTEEXEC_DEMODOUBLESCODEC_H
