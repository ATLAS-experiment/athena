/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0SEGMENTBUILDER_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0SEGMENTBUILDER_H

#include "GaudiKernel/StatusCode.h"
#include "TgcL0FloatingData.h"

namespace L0Muon {
namespace TgcL0Floating {

/** @brief Build station-level channel segments from grouped TGC hits. */
class SegmentBuilder {
 public:
  StatusCode build(const HitGroups& hitGroups,
                   SegmentContainer& segments) const;
};

}  // namespace TgcL0Floating
}  // namespace L0Muon

#endif
