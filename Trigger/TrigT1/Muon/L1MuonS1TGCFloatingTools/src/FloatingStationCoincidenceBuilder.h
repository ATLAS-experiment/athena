/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_FLOATINGSTATIONCOINCIDENCEBUILDER_H
#define L1MUONS1TGCFLOATINGTOOLS_FLOATINGSTATIONCOINCIDENCEBUILDER_H

#include "GaudiKernel/StatusCode.h"
#include "FloatingData.h"

namespace L1Muon::L1TGC {

/** @brief Build station representative points from layer-level TGC hits. */
class FloatingStationCoincidenceBuilder {
 public:
  StatusCode build(const FloatingHitGroups& hitGroups,
                   FloatingStationCoincidenceContainer& coincidences) const;

 private:
  static std::uint8_t nominalLayers(FloatingStation station, bool isStrip);
};

}  // namespace L1Muon::L1TGC

#endif
