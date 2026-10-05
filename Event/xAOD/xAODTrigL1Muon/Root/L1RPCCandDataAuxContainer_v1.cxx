/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTrigL1Muon/versions/L1RPCCandDataAuxContainer_v1.h"
#include "xAODCore/tools/AuxVariable.h"

namespace xAOD {
    L1RPCCandDataAuxContainer_v1::L1RPCCandDataAuxContainer_v1()
    : AuxContainerBase() {

      AUX_VARIABLE(l1SubdetectorId);
      AUX_VARIABLE(l1ZPos);
      AUX_VARIABLE(coinType);
      AUX_VARIABLE(l1SectorId);
      AUX_VARIABLE(bcTag);
      AUX_VARIABLE(l1Eta);
      AUX_VARIABLE(l1Phi);
      AUX_VARIABLE(l1Pt);
      AUX_VARIABLE(l1Threshold);
      AUX_VARIABLE(l1CandCharge);
      AUX_VARIABLE(tcId);

    }
}
