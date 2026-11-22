/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_DEMOCLIENTALGS_H
#define ATHEXREMOTEEXEC_DEMOCLIENTALGS_H

/**
 * @file DemoClientAlgs.h
 * @brief A collection of demo algorithms for the client
 */

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <Gaudi/Property.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadHandleKeyArray.h>
#include <StoreGate/WriteHandleKeyArray.h>

#include <atomic>
#include <cstdint>
#include <vector>

namespace AthRemoteExec::Example {

/// Makes up the numbers a client sends: one per declared key, offset by the
/// event number so that no two events send the same request.
class DemoNumbersAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::WriteHandleKeyArray<int64_t> m_values{
      this, "Values", {}, "One key per number to produce"};
  Gaudi::Property<std::vector<int>> m_offsets{
      this,
      "Offsets",
      {},
      "Per key, what to add to the event number. Must be as long as Values"};
};

/// Fails the job unless Result is the sum of Values plus Offset.
class DemoCheckAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

 private:
  SG::ReadHandleKeyArray<int64_t> m_values{
      this, "Values", {}, "The numbers that were sent"};
  SG::ReadHandleKey<int64_t> m_result{this, "Result", "", "What came back"};
  Gaudi::Property<int> m_offset{
      this, "Offset", 0, "Constant the fragment is expected to have added"};

  mutable std::atomic<size_t> m_checked{0};
};

}  // namespace AthRemoteExec::Example

#endif  // ATHEXREMOTEEXEC_DEMOCLIENTALGS_H
