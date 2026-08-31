/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGEXPARTIALEB_TRIGEXISPUBLISHING_H
#define TRIGEXPARTIALEB_TRIGEXISPUBLISHING_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include <atomic>
#include <memory>

namespace hltinterface{
  class GenericHLTContainer;
}

/**
 * IS Publishing test algorithm
 *
 * Demonstrate IS publishing using the LAr noise burst schema. The values are:
 *   Flag         cycling event counter 
 *   TimeStamp    wall-clock seconds, monotonically increasing
 *   TimeStamp_ns wall-clock nanosecond offset, different on every event
 **/
class TrigExISPublishing : public AthReentrantAlgorithm {
public:
  TrigExISPublishing(const std::string& name, ISvcLocator* svcLoc);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  std::shared_ptr<hltinterface::GenericHLTContainer> m_IsObject;

  size_t   m_evntPos{0};
  size_t   m_timeTagPos{0};
  size_t   m_timeTagPosns{0};

  /// Number of events processed so far
  mutable std::atomic<long> m_nEvents{0};
};

#endif
