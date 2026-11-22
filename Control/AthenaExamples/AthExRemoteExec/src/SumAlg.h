/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_SUMALG_H
#define ATHEXREMOTEEXEC_SUMALG_H

/**
 * @file SumAlg.h
 * @brief An algorithm that adds two numbers from StoreGate
 */

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <cstdint>

namespace AthRemoteExec::Example {
class SumAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<int64_t> m_a{this, "A", "a", "First addend"};
  SG::ReadHandleKey<int64_t> m_b{this, "B", "b", "Second addend"};
  SG::WriteHandleKey<int64_t> m_sum{this, "Sum", "sum", "Result"};
};

}  // namespace AthRemoteExec::Example

#endif  // ATHEXREMOTEEXEC_SUMALG_H
