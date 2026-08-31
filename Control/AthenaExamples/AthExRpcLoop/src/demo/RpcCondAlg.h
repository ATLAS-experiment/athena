/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCCONDALG_H
#define ATHEXRPCLOOP_RPCCONDALG_H

/**
 * @file RpcCondAlg.h
 * @brief Produces RpcCondData with an IOV covering the run it is called in.
 *
 * Deliberately has no database behind it: the point is to exercise the IOV
 * machinery against the *synthetic* EventID the loop manager fabricates, not to
 * test IOVDbSvc. The value depends on the run number so that a test can tell
 * "the conditions lookup used the event's run" apart from "there happened to be
 * exactly one object in the container".
 *
 * The IOV is bounded to a single run rather than infinite, for the same reason:
 * an infinite range would be satisfied by any EventID at all, including a
 * malformed one, and would prove nothing.
 */

#include "RpcCondData.h"

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "Gaudi/Property.h"
#include "StoreGate/WriteCondHandleKey.h"

#include <cstdint>

namespace AthExRpc {

class RpcCondAlg : public AthCondAlgorithm {
public:
  using AthCondAlgorithm::AthCondAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::WriteCondHandleKey<RpcCondData> m_key{
      this, "Offset", "RpcCondOffset", "Conditions object to produce"};
  Gaudi::Property<int64_t> m_base{
      this, "Base", 100,
      "The produced offset is Base + the run number of the event"};
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCCONDALG_H
