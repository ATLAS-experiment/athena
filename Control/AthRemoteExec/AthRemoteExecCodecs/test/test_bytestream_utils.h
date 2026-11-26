/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHREMOTEEXECCODECS_TEST_BYTESTREAM_UTILS_H
#define ATHREMOTEEXECCODECS_TEST_BYTESTREAM_UTILS_H

/**
 * @file test_bytestream_utils.h
 * @brief Everything that's needed to test the bytestream codec end to end
 */

#include <AthRemoteExecCodecs/RawEventCache.h>
#include <AthRemoteExecCore/PayloadCodecBase.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <ByteStreamCnvSvcBase/IROBDataProviderSvc.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/ServiceHandle.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>
#include <eformat/SourceIdentifier.h>

#include <atomic>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace AthRemoteExec {

/// Copy of AthRemoteExecCore's test_IntsCodec: carries named uint32 values as
/// name:value# pairs.
class test_NamedIntsCodec : public PayloadCodecBase {
 public:
  using PayloadCodecBase::PayloadCodecBase;

  virtual const std::string& encoding() const override {
    static constexpr std::string tag = "protobuf";
    return tag;
  }

 protected:
  virtual StatusCode initializeBoundary() override;
  virtual StatusCode doDecode(const EventContext& ctx, IProxyDict& store,
                              const Payload& payload) const override;

  virtual StatusCode doEncode(
      const EventContext& ctx, IProxyDict& store, Payload& payload,
      const std::vector<const IPayloadCodec*>& alsoCrossing,
      std::vector<std::string>& dangling) const override;

 private:
  SG::ReadHandleKeyArray<uint32_t> m_inputInts{
      this, "InputInts", {}, "Ints to encode"};
  SG::WriteHandleKeyArray<uint32_t> m_outputInts{
      this, "OutputInts", {}, "Ints to decode"};
};

/// Server side of the round trip: digest the decoded RawEvent into two ints
/// that travel back to the client inside a tiny reply.
class test_RawEventDigestAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<RawEventCache> m_cache{
      this, "Cache", "",
      "The RawEvent cache the bytestream payload decoded to"};
  SG::WriteHandleKey<uint32_t> m_size{this, "Size", "",
                                      "Key for the event's word count"};
  SG::WriteHandleKey<uint32_t> m_hash{this, "Hash", "",
                                      "Key for the event's digest"};
};

/// Client side: digest the original event from the bytestream input and
/// compare with the digest that came back from the server.
class test_RawEventVerifyAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

 private:
  SG::ReadHandleKey<uint32_t> m_size{this, "Size", "",
                                     "Key with the event's word count"};
  SG::ReadHandleKey<uint32_t> m_hash{this, "Hash", "",
                                     "Key with the event's digest"};
  ServiceHandle<IROBDataProviderSvc> m_robsSvc{
      this, "ROBDataProvider", "ROBDataProviderSvc/ROBDataProviderSvc"};

  /// Sub-detectors whose ROBs are encoded; empty means the entire event
  Gaudi::Property<std::vector<std::string>> m_keepDetectors{
      this, "KeepDetectors", {}, "Sub-detectors whose ROBs are encoded; empty -> the entire event"};

  /// The decoded KeepDetectors
  std::set<eformat::SubDetectorGroup> m_keep;

  mutable std::atomic<size_t> m_checked{0};
};

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECCODECS_TEST_BYTESTREAM_UTILS_H
