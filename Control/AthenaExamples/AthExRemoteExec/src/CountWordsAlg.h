/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_COUNTWORDSALG_H
#define ATHEXREMOTEEXEC_COUNTWORDSALG_H

/**
 * @file CountWordsAlg.h
 * @brief An algorithm that counts number of elements in vector of unsigned
 * 32-bit integers and writes that number to SG (as signed 64-bit integer)
 */

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <cstdint>

namespace AthRemoteExec::Example {
class CountWordsAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<std::vector<uint32_t>> m_words{this, "Words", "words",
                                                   "Vector of words"};
  SG::WriteHandleKey<int64_t> m_count{this, "Count", "count",
                                      "Number of words"};
};

}  // namespace AthRemoteExec::Example

#endif  // ATHEXREMOTEEXEC_SUMALG_H
