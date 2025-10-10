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
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
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
const Identifier& TgcStrip_v1::identify() const {
   if (!m_identifier.isValid()){
      m_identifier.set(readoutElement()->measurementId(measurementHash()));
   }
   return (*m_identifier.ptr());
}
IdentifierHash TgcStrip_v1::layerHash() const {
   return MuonGMR4::TgcReadoutElement::constructHash(0, gasGap(), false);
}
Amg::Vector3D TgcStrip_v1::localMeasurementPos() const {
   return localPosition<1>()[0] * Amg::Vector3D::Unit(measuresPhi());
}
}  // namespace xAOD
#undef IMPLEMENT_SETTER_GETTER
