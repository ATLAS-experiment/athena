/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0RDODECODER_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0RDODECODER_H

#include "GaudiKernel/StatusCode.h"
#include "TgcL0FloatingData.h"

class TgcRdoContainer;

namespace L0Muon {
namespace TgcL0Floating {

/** @brief Decode Run-3 TGC hit RDOs and group the decoded hits. */
class RdoDecoder {
 public:
  StatusCode decode(const TgcRdoContainer& rdos, HitGroups& hitGroups,
                    DecodeStatistics& statistics) const;
};

}  // namespace TgcL0Floating
}  // namespace L0Muon

#endif
