/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_DEMODOUBLESADAPTERS_H
#define ATHEXRPCLOOP_DEMODOUBLESADAPTERS_H

/**
 * @file DemoDoublesAdapters.h
 * @brief The demonstration fragment's bulk boundary: a vector of doubles.
 *
 * The mirror pair of DemoIntsAdapters, for the case that actually matters in a
 * real fragment -- an array rather than a handful of scalars. It is here to
 * show that nothing about the arrangement changes with size: the same blob, the
 * same schema check, the same ordinary typed handle on the far side of it.
 */

#include "../RpcBlob.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <vector>

namespace AthExRpc {

/// Bytes in, a vector of doubles out.
class DemoDoublesUnpackAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::ReadHandleKey<RpcBlob> m_payload{
      this, "Payload", "", "The unopened payload, as the gate recorded it"};
  SG::WriteHandleKey<std::vector<double>> m_values{
      this, "Values", "", "Where to put the decoded values"};
};

/// A vector of doubles in, bytes out.
class DemoDoublesPackAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::ReadHandleKey<std::vector<double>> m_values{this, "Values", "",
                                                  "The values to send"};
  SG::WriteHandleKey<RpcBlob> m_payload{
      this, "Payload", "", "Where to leave the encoded message"};
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_DEMODOUBLESADAPTERS_H
