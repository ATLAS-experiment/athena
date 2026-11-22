/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_DEMOWORDSCODEC_H
#define ATHEXREMOTEEXEC_DEMOWORDSCODEC_H

/**
 * @file DemoWordsCodec.h
 * @brief The demonstration of codec that de/encodes vector of unsigned integers
 * it on SG
 */

#include <AthRemoteExecCore/PayloadCodecBase.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <cstdint>
#include <string>

namespace AthRemoteExec::Example {

class DemoWordsCodec : public PayloadCodecBase {
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
  SG::WriteHandleKey<std::vector<uint32_t>> m_decoded{this, "Decoded", "",
                                                      "Decoded words key"};
  SG::ReadHandleKey<std::vector<uint32_t>> m_encoded{this, "Encoded", "",
                                                     "Encoded words key"};
};

}  // namespace AthRemoteExec::Example

#endif  // ATHEXREMOTEEXEC_DEMOINTSCODEC_H
