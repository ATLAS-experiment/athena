/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENTCNV_SEEDVALIDATIONALG_H
#define ACTSGPUEVENTCNV_SEEDVALIDATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ActsEvent/SeedContainer.h"
#include "StoreGate/ReadHandleKey.h"

namespace ActsTrk {

/*! Algo that compares two sets of xAOD seeds
 *
 * Its main usage is to validate that different Traccc measurement conversion
 * produce the same set of seed containers.
 *
 * Order of measurements matter so the algorithm expect to find the same
 * data at the same position.
 */

class SeedValidationAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  SG::ReadHandleKey<ActsTrk::SeedContainer> m_referenceSeedsKey{
    this, "ReferenceSeeds", "", "The reference seeds"};
  SG::ReadHandleKey<ActsTrk::SeedContainer> m_monitoredSeedsKey{
    this, "MonitoredSeeds", "", "The monitored seeds"};
 
  Gaudi::Property<bool> m_compareQuality{
    this, "CompareQuality", false,
    "Also compare seed quality (off by default: the host converter stores 0)"};
  
};

} // namespace ActsTrk

#endif // ACTSGPUEVENTCNV_SEEDVALIDATIONALG_H