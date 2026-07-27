/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0STATIONCOINCIDENCE_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0STATIONCOINCIDENCE_H

#include "GaudiKernel/StatusCode.h"
#include "TgcL0FloatingData.h"

namespace L0Muon {
namespace TgcL0Floating {

/** @brief Build station representative points from layer-level TGC hits. */
class StationCoincidenceBuilder {
 public:
  StatusCode build(const HitGroups& hitGroups,
                   StationCoincidenceContainer& coincidences) const;

 private:
  static std::uint8_t nominalLayers(Station station, bool isStrip);
};

}  // namespace TgcL0Floating
}  // namespace L0Muon

#endif
