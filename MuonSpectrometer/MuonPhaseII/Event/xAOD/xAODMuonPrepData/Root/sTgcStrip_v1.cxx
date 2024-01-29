/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/sTgcStrip_v1.h"

#include "GaudiKernel/ServiceHandle.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/StoreGateSvc.h"

namespace {
    static const std::string preFixStr{"sTgc"};
}
                                                                          
namespace xAOD {

IMPLEMENT_SETTER_GETTER(sTgcStrip_v1, uint16_t, bcBitMap, setBcBitMap)
IMPLEMENT_SETTER_GETTER(sTgcStrip_v1, uint16_t, time, setTime)
IMPLEMENT_SETTER_GETTER(sTgcStrip_v1, uint32_t, charge, setCharge)
IMPLEMENT_READOUTELEMENT(sTgcStrip_v1, m_readoutEle, sTgcReadoutElement)

IdentifierHash sTgcStrip_v1::measurementHash() const {
   //  return MuonGMR4::TgcReadoutElement::measurementHash(tubeLayer(), // FIXME - check what we need to get the hash for TGCs
   //                                                      driftTube());
   return IdentifierHash();
}

}  // namespace xAOD
#undef IMPLEMENT_SETTER_GETTER
