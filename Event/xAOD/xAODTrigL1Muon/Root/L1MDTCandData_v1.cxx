/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTrigL1Muon/versions/L1MDTCandData_v1.h"

#include "xAODCore/AuxStoreAccessorMacros.h"
#include <cstdint>

namespace xAOD {
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint8_t, l1MdtFlag, setL1MdtFlag )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint8_t, l1NumSegments, setL1NumSegments )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint8_t, l1SlPtThreshold, setL1SlPtThreshold )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint8_t, l1SlCharge, setL1SlCharge )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint16_t, l1SlPhiPosition, setL1SlPhiPosition )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint16_t, l1SlEtaPosition, setL1SlEtaPosition )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint8_t, l1SegmentQualityFlag, setL1SegmentQualityFlag )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint8_t, l1MdtCharge, setL1MdtCharge )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint8_t, l1MdtPt, setL1MdtPt )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1MDTCandData_v1, uint16_t, l1MdtEta, setL1MdtEta )

} // namespace xAOD
