/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODL0MuonCand/versions/MDTCandData_v1.h"

#include "xAODMuonPrepData/versions/AccessorMacros.h"
#include <cstdint>

namespace {
   static const std::string preFixStr{"L0Mu_"};
}

namespace xAOD
{
   // cppcheck-suppress unknownMacro
  IMPLEMENT_SETTER_GETTER( MDTCandData_v1, uint8_t, numSegments, setNumSegments )
  IMPLEMENT_SETTER_GETTER( MDTCandData_v1, uint8_t, slPtThreshold, setSlPtThreshold )
  IMPLEMENT_SETTER_GETTER( MDTCandData_v1, uint8_t, slCharge, setSlCharge )
  IMPLEMENT_SETTER_GETTER( MDTCandData_v1, uint16_t, slPhiPosition, setSlPhiPosition )
  IMPLEMENT_SETTER_GETTER( MDTCandData_v1, uint8_t, tcIdentifier, setTcIdentifier )
  IMPLEMENT_SETTER_GETTER( MDTCandData_v1, uint8_t, segmentQualityFlag, setSegmentQualityFlag )

} // namespace xAOD
