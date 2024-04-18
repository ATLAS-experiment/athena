/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/MdtDriftCircle_v1.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/StoreGateSvc.h"

namespace {
    static const std::string preFixStr{"Mdt_"};    
}

namespace xAOD {
using MdtDriftCircleStatus = MdtDriftCircle_v1::MdtDriftCircleStatus;

IMPLEMENT_SETTER_GETTER(MdtDriftCircle_v1, int16_t, tdc, setTdc)
IMPLEMENT_SETTER_GETTER(MdtDriftCircle_v1, int16_t, adc, setAdc)
IMPLEMENT_SETTER_GETTER(MdtDriftCircle_v1, uint16_t, driftTube, setTube)
IMPLEMENT_SETTER_GETTER(MdtDriftCircle_v1, uint8_t, tubeLayer, setLayer)
IMPLEMENT_SETTER_GETTER_WITH_CAST(MdtDriftCircle_v1, uint8_t, MdtDriftCircleStatus, status, setStatus)
IMPLEMENT_READOUTELEMENT(MdtDriftCircle_v1, m_readoutEle, MdtReadoutElement)

IdentifierHash MdtDriftCircle_v1::measurementHash() const {
    return MuonGMR4::MdtReadoutElement::measurementHash(tubeLayer(),
                                                        driftTube());
}
float MdtDriftCircle_v1::driftRadius() const {
    return localPosition<1>()[Trk::locR];
}
/** @brief Returns the covariance of the drift radius*/
float MdtDriftCircle_v1::driftRadiusCov() const {
    return localCovariance<1>()(Trk::locR, Trk::locR);
}
/** @brief Returns the uncertainty on the drift radius*/
float MdtDriftCircle_v1::driftRadiusUncert() const {
    return std::sqrt(driftRadiusCov());
}
void MdtDriftCircle_v1::setDriftRadius(float r) {
    localPosition<1>()[Trk::locR] = r;
}
void MdtDriftCircle_v1::setDriftRadCov(float cov) {
    localCovariance<1>()(Trk::locR, Trk::locR) = cov;
}

}  // namespace xAOD
