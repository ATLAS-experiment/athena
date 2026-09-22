/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_DELAYALG_H
#define ATHEXREMOTEEXEC_DELAYALG_H

/**
 * @file DelayAlg.h
 * @brief Burns wall-clock time so that concurrency is observable.
 *
 * Without something slow in a fragment, requests complete faster than they can
 * be dispatched and the loop never has more than one event in flight -- which
 * would make the MT exit criterion untestable. Sleeping (rather than spinning)
 * is deliberate: it frees the TBB worker, which is what a real offloading
 * fragment does while waiting for a GPU or a remote call.
 */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "Gaudi/Property.h"

namespace AthExRemoteExec {

class DelayAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  Gaudi::Property<unsigned int> m_milliseconds{
      this, "Milliseconds", 0, "How long to sleep for"};
};

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_DELAYALG_H
