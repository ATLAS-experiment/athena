/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "TrkEventPrimitives/ParamDefs.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "xAODMuonPrepData/versions/RpcMeasurement_v1.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/StoreGateSvc.h"

namespace {
    static const std::string preFixStr{"Rpc_"};
}

namespace xAOD {
    IMPLEMENT_SETTER_GETTER(RpcMeasurement_v1, float, time, setTime)
    IMPLEMENT_SETTER_GETTER(RpcMeasurement_v1, uint32_t, triggerInfo, setTriggerInfo)
    IMPLEMENT_SETTER_GETTER(RpcMeasurement_v1, uint8_t, ambiguityFlag, setAmbiguityFlag)
    IMPLEMENT_SETTER_GETTER(RpcMeasurement_v1, float, timeOverThreshold, setTimeOverThreshold)
    IMPLEMENT_SETTER_GETTER(RpcMeasurement_v1, uint16_t, stripNumber, setStripNumber)
    IMPLEMENT_SETTER_GETTER(RpcMeasurement_v1, uint8_t, gasGap, setGasGap)
    IMPLEMENT_SETTER_GETTER(RpcMeasurement_v1, uint8_t, doubletPhi, setDoubletPhi)
    IMPLEMENT_SETTER_GETTER(RpcMeasurement_v1, float, timeCovariance, setTimeCovariance)
    IMPLEMENT_READOUTELEMENT(RpcMeasurement_v1, m_readoutEle, RpcReadoutElement)

    IdentifierHash RpcMeasurement_v1::measurementHash() const {
        return MuonGMR4::RpcReadoutElement::createHash(stripNumber(), 
                                                       gasGap(),
                                                       doubletPhi(),
                                                       measuresPhi());
    }
    IdentifierHash RpcMeasurement_v1::layerHash() const {
        return MuonGMR4::RpcReadoutElement::createHash(0, gasGap(), doubletPhi(), measuresPhi());
    }
    const Identifier& RpcMeasurement_v1::identify() const {
        if (!m_identifier.isValid()){
            m_identifier.set(readoutElement()->measurementId(measurementHash()));
        }
        return (*m_identifier.ptr());
    }
    Amg::Vector3D RpcMeasurement_v1::localMeasurementPos() const {
        Amg::Vector3D lPos{Amg::Vector3D::Zero()};
        if(numDimensions() == 1) {
            lPos[Trk::locX] =  localPosition<1>()[Trk::locX];
        } else {
            lPos.block<2,1>(0,0) = xAOD::toEigen(localPosition<2>());
        }
        return lPos;
    }
}