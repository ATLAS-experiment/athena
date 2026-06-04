/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODL0MuonCand/versions/MDTCandDataAuxContainer_v1.h"
#include "xAODMuonPrepData/versions/AccessorMacros.h"
namespace {
   static const std::string preFixStr{"L0Mu_"};
}

namespace xAOD {
    MDTCandDataAuxContainer_v1::MDTCandDataAuxContainer_v1()
    : AuxContainerBase() {

      // RPC/TGC candidate data
      PRD_AUXVARIABLE(subdetectorId);
      PRD_AUXVARIABLE(sectorId);
      PRD_AUXVARIABLE(bcTag);
      PRD_AUXVARIABLE(mdtFlag);
      PRD_AUXVARIABLE(numSegments);
      PRD_AUXVARIABLE(segmentQualityFlag);
      PRD_AUXVARIABLE(candCharge);
      PRD_AUXVARIABLE(threshold);
      PRD_AUXVARIABLE(pt);
      PRD_AUXVARIABLE(eta);
      PRD_AUXVARIABLE(coinType);
      PRD_AUXVARIABLE(phi);
      PRD_AUXVARIABLE(slCharge);
      PRD_AUXVARIABLE(slPtThreshold);
      PRD_AUXVARIABLE(slPhiPosition);
      PRD_AUXVARIABLE(tcIdentifier);

    }
}
