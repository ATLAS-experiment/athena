/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_OFFSETALG_H
#define ATHEXREMOTEEXEC_OFFSETALG_H

/**
 * @file OffsetAlg.h
 * @brief Adds a conditions-derived offset to an input.
 *
 * The conditions-using counterpart of SumAlg, and just as RemoteExec-ignorant: an
 * ordinary ReadCondHandle plus an ordinary ReadHandle. If the loop manager ever
 * stopped installing the Atlas::ExtendedEventContext, the ReadCondHandle
 * constructor here would throw and this algorithm is what would notice.
 */

#include "RemoteExecCondData.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <cstdint>

namespace AthExRemoteExec {

class OffsetAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::ReadHandleKey<int64_t> m_input{this, "Input", "x", "Value to offset"};
  SG::ReadCondHandleKey<RemoteExecCondData> m_offset{
      this, "Offset", "RemoteExecCondOffset", "Conditions object holding the offset"};
  SG::WriteHandleKey<int64_t> m_output{this, "Output", "offsetted", "Result"};
};

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_OFFSETALG_H
