/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODL0MuonCand/versions/TGCCandDataAuxContainer_v1.h"
#include "xAODMuonPrepData/versions/AccessorMacros.h"
namespace {
   static const std::string preFixStr{"L0Mu_"};
}

namespace xAOD {
    TGCCandDataAuxContainer_v1::TGCCandDataAuxContainer_v1()
    : AuxContainerBase() {

      PRD_AUXVARIABLE(subdetectorId);
      PRD_AUXVARIABLE(candQuality);
      PRD_AUXVARIABLE(sectorId);
      PRD_AUXVARIABLE(bcTag);  
      PRD_AUXVARIABLE(eta);
      PRD_AUXVARIABLE(phi);
      PRD_AUXVARIABLE(pt);
      PRD_AUXVARIABLE(threshold);
      PRD_AUXVARIABLE(candCharge);
      PRD_AUXVARIABLE(mdtFlag);
      PRD_AUXVARIABLE(coinType);
      PRD_AUXVARIABLE(deltaPhi);
      PRD_AUXVARIABLE(deltaTheta);
      PRD_AUXVARIABLE(nswSegment);
      PRD_AUXVARIABLE(tcId);

    }
}
