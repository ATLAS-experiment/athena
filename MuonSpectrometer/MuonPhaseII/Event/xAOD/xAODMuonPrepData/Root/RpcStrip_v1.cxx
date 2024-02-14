/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/RpcStrip_v1.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/StoreGateSvc.h"

namespace {
    static const std::string preFixStr{"Rpc_"};
    static const xAOD::PosAccessor<3> accStripPos{preFixStr + "stripPosInStation"};
}
namespace xAOD {

IMPLEMENT_SETTER_GETTER(RpcStrip_v1, float, time, setTime)
IMPLEMENT_SETTER_GETTER(RpcStrip_v1, uint32_t, triggerInfo, setTriggerInfo)
IMPLEMENT_SETTER_GETTER(RpcStrip_v1, uint8_t, ambiguityFlag, setAmbiguityFlag)
IMPLEMENT_SETTER_GETTER(RpcStrip_v1, float, timeOverThreshold, setTimeOverThreshold)
IMPLEMENT_SETTER_GETTER(RpcStrip_v1, uint16_t, stripNumber, setStripNumber)
IMPLEMENT_SETTER_GETTER(RpcStrip_v1, uint8_t, gasGap, setGasGap)
IMPLEMENT_SETTER_GETTER(RpcStrip_v1, uint8_t, doubletPhi, setDoubletPhi)
IMPLEMENT_SETTER_GETTER(RpcStrip_v1, uint8_t, measuresPhi, setMeasuresPhi)
IMPLEMENT_READOUTELEMENT(RpcStrip_v1, m_readoutEle, RpcReadoutElement)

IdentifierHash RpcStrip_v1::measurementHash() const {
    return MuonGMR4::RpcReadoutElement::createHash(stripNumber(), 
                                                   gasGap(),
                                                   doubletPhi(),
                                                   measuresPhi());
}
void RpcStrip_v1::setStripPosInStation(const MeasVector<3>& pos){
    VectorMap<3> v{accStripPos(*this).data()};
    v = pos;
}
ConstVectorMap<3> RpcStrip_v1::stripPosInStation() const {
    return ConstVectorMap<3>{accStripPos(*this).data()};
}

}  // namespace xAOD
#undef IMPLEMENT_SETTER_GETTER
#undef THROW_EXCEPT
