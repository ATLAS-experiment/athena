/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
#include "TrkEventPrimitives/ParamDefs.h"

// Local include(s):
#include "xAODMuonPrepData/versions/MdtTwinDriftCircle_v1.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
namespace {
    static const std::string preFixStr{"Mdt_"};    
}

namespace xAOD {

IMPLEMENT_SETTER_GETTER(MdtTwinDriftCircle_v1, int16_t, twinTdc, setTwinTdc)
IMPLEMENT_SETTER_GETTER(MdtTwinDriftCircle_v1, int16_t, twinAdc, setTwinAdc)
IMPLEMENT_SETTER_GETTER(MdtTwinDriftCircle_v1, uint8_t, twinLayer, setTwinLayer)
IMPLEMENT_SETTER_GETTER(MdtTwinDriftCircle_v1, uint16_t, twinTube, setTwinTube)

Identifier MdtTwinDriftCircle_v1::twinIdentify() const {
    return readoutElement()->measurementId(readoutElement()->measurementHash(twinLayer(), twinTube()));
}
float MdtTwinDriftCircle_v1::posAlongWire() const {
    return localPosition<2>()(Trk::locZ);
}
float MdtTwinDriftCircle_v1::posAlongWireCov() const {
    return localCovariance<2>()(Trk::locZ, Trk::locZ);
}
float MdtTwinDriftCircle_v1::posAlongWireUncert() const {
    return std::sqrt(posAlongWireCov());
}

}  // namespace xAOD
