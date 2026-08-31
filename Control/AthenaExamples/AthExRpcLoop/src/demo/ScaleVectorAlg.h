/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_SCALEVECTORALG_H
#define ATHEXRPCLOOP_SCALEVECTORALG_H

/**
 * @file ScaleVectorAlg.h
 * @brief Multiplies a vector of doubles by a constant.
 *
 * Exists to be run *twice*: once in a fragment whose boundary uses the typed
 * tier, and once in a fragment whose boundary uses ROOT-streamed blobs. It is
 * the same class with the same ordinary typed handles in both, which is the
 * check that the blob tier does not leak into payload algorithms -- if it did,
 * this algorithm could not be shared between the two fragments, let alone
 * between an RPC sequence and a normal job.
 *
 * Running the two fragments over identical data also gives the direct
 * comparison of what the two encodings cost in bytes and in time.
 */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "Gaudi/Property.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <vector>

namespace AthExRpc {

class ScaleVectorAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::ReadHandleKey<std::vector<double>> m_input{this, "Input", "points",
                                                 "Values to scale"};
  SG::WriteHandleKey<std::vector<double>> m_output{this, "Output", "scaled",
                                                   "Scaled values"};
  Gaudi::Property<double> m_factor{this, "Factor", 2.0, "Multiplier"};
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_SCALEVECTORALG_H
