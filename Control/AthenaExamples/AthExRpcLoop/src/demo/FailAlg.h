/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_FAILALG_H
#define ATHEXRPCLOOP_FAILALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

namespace AthExRpc {

/**
 * @brief Always fails, so that tests can check what a failing payload does to
 *        the server.
 *
 * The rule borrowed from the HLT (HltEventLoopMgr::failedEvent) is that every
 * request gets a reply: a failure here must come back as an error status, and
 * the server must go on serving.
 */
class FailAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode execute( const EventContext& ctx ) const override;
};

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_FAILALG_H
