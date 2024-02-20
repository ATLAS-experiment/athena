/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
    static const xAOD::PosAccessor<3> accStripPos{preFixStr + "stripPosInStation"};

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
void TgcStrip_v1::setStripPosInStation(const MeasVector<3>& pos){
    VectorMap<3> v{accStripPos(*this).data()};
    v = pos;
}
ConstVectorMap<3> TgcStrip_v1::stripPosInStation() const {
    return ConstVectorMap<3>{accStripPos(*this).data()};
}
}  // namespace xAOD
#undef IMPLEMENT_SETTER_GETTER
