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
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
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
Amg::Vector3D MdtDriftCircle_v1::localCirclePosition() const {
    if (numDimensions() == 1) {
        return Amg::Vector3D::Zero();
    }
    return localPosition<2>()[Trk::locZ] * Amg::Vector3D::UnitZ(); 
}
const Identifier& MdtDriftCircle_v1::identify() const {
    if (!m_identifier.isValid()){
        m_identifier.set(readoutElement()->measurementId(measurementHash()));
    }
    return (*m_identifier.ptr());
}
float MdtDriftCircle_v1::driftRadius() const {
    return numDimensions() == 1 ? localPosition<1>()[Trk::locR] 
                                : localPosition<2>()[Trk::locR];
}
/** @brief Returns the covariance of the drift radius*/
float MdtDriftCircle_v1::driftRadiusCov() const {
    return numDimensions() == 1 ? localCovariance<1>()(Trk::locR, Trk::locR) 
                                : localCovariance<2>()(Trk::locR, Trk::locR);
}
/** @brief Returns the uncertainty on the drift radius*/
float MdtDriftCircle_v1::driftRadiusUncert() const {
    return std::sqrt(driftRadiusCov());
}
}  // namespace xAOD
