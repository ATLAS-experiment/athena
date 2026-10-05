/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTrigL1Muon/versions/L1MDTCandDataAuxContainer_v1.h"
#include "xAODCore/tools/AuxVariable.h"

namespace xAOD {
    L1MDTCandDataAuxContainer_v1::L1MDTCandDataAuxContainer_v1()
    : AuxContainerBase() {

      // RPC/TGC candidate data
      AUX_VARIABLE(l1SubdetectorId);
      AUX_VARIABLE(l1SectorId);
      AUX_VARIABLE(bcTag);
      AUX_VARIABLE(l1MdtFlag);
      AUX_VARIABLE(l1NumSegments);
      AUX_VARIABLE(l1SegmentQualityFlag);
      AUX_VARIABLE(l1CandCharge);
      AUX_VARIABLE(l1Threshold);
      AUX_VARIABLE(l1Pt);
      AUX_VARIABLE(l1Eta);
      AUX_VARIABLE(coinType);
      AUX_VARIABLE(l1Phi);
      AUX_VARIABLE(l1SlCharge);
      AUX_VARIABLE(l1SlPtThreshold);
      AUX_VARIABLE(l1SlPhiPosition);
      AUX_VARIABLE(l1SlEtaPosition);
      AUX_VARIABLE(tcId);
      AUX_VARIABLE(l1MdtCharge);
      AUX_VARIABLE(l1MdtPt);
      AUX_VARIABLE(l1MdtEta);

    }
}
