/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_FLOATINGRDODECODER_H
#define L1MUONS1TGCFLOATINGTOOLS_FLOATINGRDODECODER_H

#include "GaudiKernel/StatusCode.h"
#include "FloatingData.h"

class TgcRdoContainer;

namespace Muon {
class IMuonIdHelperSvc;
class TgcCablingMap;
}

namespace MuonGM {
class MuonDetectorManager;
}

namespace L1Muon::L1TGC {

class FloatingRdoDecoder {
 public:
  StatusCode decode(const TgcRdoContainer& rdos,
                    const Muon::TgcCablingMap& cabling,
                    const Muon::IMuonIdHelperSvc& idHelperSvc,
                    const MuonGM::MuonDetectorManager& detectorManager,
                    FloatingHitGroups& hitGroups,
                    FloatingDecodeStatistics& statistics) const;
};

}  // namespace L1Muon::L1TGC

#endif
