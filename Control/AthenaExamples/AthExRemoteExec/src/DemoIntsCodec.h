/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_DEMOINTSCODEC_H
#define ATHEXREMOTEEXEC_DEMOINTSCODEC_H

/**
 * @file DemoIntsCodec.h
 * @brief The demonstration of codec de/encoding multiple named itegers.
 */

#include <AthRemoteExecCore/PayloadCodecBase.h>
#include <StoreGate/ReadHandleKeyArray.h>
#include <StoreGate/WriteHandleKeyArray.h>

#include <cstdint>
#include <string>

namespace AthRemoteExec::Example {

class DemoIntsCodec : public PayloadCodecBase {
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
  SG::WriteHandleKeyArray<int64_t> m_decoded{
      this, "Decoded", {}, "Decoded values key array"};
  SG::ReadHandleKeyArray<int64_t> m_encoded{
      this, "Encoded", {}, "Encoded values key array"};
};

}  // namespace AthRemoteExec::Example

#endif  // ATHEXREMOTEEXEC_DEMOINTSCODEC_H
