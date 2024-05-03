/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/TgcStrip_v1.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/StoreGateSvc.h"

namespace {
    static const std::string preFixStr{"Tgc_"};
}

namespace xAOD {

IMPLEMENT_SETTER_GETTER(TgcStrip_v1, uint16_t, bcBitMap, setBcBitMap)
IMPLEMENT_SETTER_GETTER(TgcStrip_v1, uint16_t, channelNumber, setChannelNumber)
IMPLEMENT_SETTER_GETTER(TgcStrip_v1, uint8_t, gasGap, setGasGap)
IMPLEMENT_SETTER_GETTER(TgcStrip_v1, uint8_t, measuresPhi, setMeasuresPhi)
IMPLEMENT_READOUTELEMENT(TgcStrip_v1, m_readoutEle, TgcReadoutElement)

IdentifierHash TgcStrip_v1::measurementHash() const {
   return MuonGMR4::TgcReadoutElement::constructHash(channelNumber(), gasGap(), measuresPhi());
}
IdentifierHash TgcStrip_v1::layerHash() const {
   return MuonGMR4::TgcReadoutElement::constructHash(0, gasGap(), measuresPhi());
}
}  // namespace xAOD
#undef IMPLEMENT_SETTER_GETTER
