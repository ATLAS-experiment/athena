/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_TGCL0RDODECODER_H
#define L1MUONS1TGCFLOATINGTOOLS_TGCL0RDODECODER_H

#include "GaudiKernel/StatusCode.h"
#include "TgcL0FloatingData.h"

class TgcRdoContainer;

namespace Muon {
class IMuonIdHelperSvc;
class TgcCablingMap;
}

namespace MuonGM {
class MuonDetectorManager;
}

namespace L1Muon {
namespace TgcL0Floating {

class RdoDecoder {
 public:
  StatusCode decode(const TgcRdoContainer& rdos,
                    const Muon::TgcCablingMap& cabling,
                    const Muon::IMuonIdHelperSvc& idHelperSvc,
                    const MuonGM::MuonDetectorManager& detectorManager,
                    HitGroups& hitGroups,
                    DecodeStatistics& statistics) const;
};

}  // namespace TgcL0Floating
}  // namespace L1Muon

#endif
