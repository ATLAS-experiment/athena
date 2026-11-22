/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXREMOTEEXEC_TEST_UTILS_H
#define ATHEXREMOTEEXEC_TEST_UTILS_H

/**
 * @file test_IntsCodec.h
 * @brief Everything that's needed to test this package
 */

#include <AthRemoteExecCore/PayloadCodecBase.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadHandleKeyArray.h>
#include <StoreGate/WriteHandleKey.h>
#include <StoreGate/WriteHandleKeyArray.h>

#include <cstdint>
#include <string>

namespace AthRemoteExec {

/// Copy of DemoIntsCodec, but without using custom proto definition
class test_IntsCodec : public PayloadCodecBase {
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
  SG::ReadHandleKeyArray<int32_t> m_inputInts{
      this, "InputInts", {}, "Ints to endcode"};
  SG::WriteHandleKeyArray<int32_t> m_outputInts{
      this, "OutputInts", {}, "Ints to decode"};
};

/// Copy of demo alg; Adds two ints from SG together
class test_SumAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<int32_t> m_a{this, "IntA", "intA", "First addend"};
  SG::ReadHandleKey<int32_t> m_b{this, "IntB", "intB", "Second addend"};
  SG::WriteHandleKey<int32_t> m_sum{this, "IntSum", "intSum", "Result"};
};

/// Copy of DemoNumbersAlg
class test_NumbersAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::WriteHandleKeyArray<int32_t> m_values{
      this, "Values", {}, "One key per number to produce"};
  Gaudi::Property<std::vector<int>> m_offsets{
      this,
      "Offsets",
      {},
      "Per key, what to add to the event number. Must be as long as Values"};
};

/// Copy of DemoCheckAlg
class test_CheckAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

 private:
  SG::ReadHandleKeyArray<int32_t> m_values{
      this, "Values", {}, "The numbers that were sent"};
  SG::ReadHandleKey<int32_t> m_result{this, "Result", "", "What came back"};
  Gaudi::Property<int> m_offset{
      this, "Offset", 0, "Constant the fragment is expected to have added"};

  mutable std::atomic<size_t> m_checked{0};
};

/// Copy of FailAlg
class test_FailAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode execute(const EventContext& ctx) const override;
};

/// Copy od DelayAlg
class test_DelayAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  Gaudi::Property<unsigned int> m_milliseconds{this, "Milliseconds", 0,
                                               "How long to sleep for"};
};

}  // namespace AthRemoteExec

#endif  // ATHEXREMOTEEXEC_DEMOINTSCODEC_H
