/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_SUMALG_H
#define ATHEXRPCLOOP_SUMALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <cstdint>

namespace AthExRpc {

/**
 * @brief Adds two numbers taken from StoreGate.
 *
 * Deliberately RPC-ignorant: it has ordinary read and write handles and would
 * behave identically in a normal Athena job. Everything about the request lives
 * on the loop manager's side of the boundary.
 *
 * int64_t is used because that is what the wire contract's integer type maps
 * to; both `long` and `long long` have CLIDs (SGTools/BuiltinsClids.h).
 */
class SumAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::ReadHandleKey<int64_t> m_a{this, "A", "a", "First addend"};
  SG::ReadHandleKey<int64_t> m_b{this, "B", "b", "Second addend"};
  SG::WriteHandleKey<int64_t> m_sum{this, "Sum", "sum", "Result"};
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_SUMALG_H
