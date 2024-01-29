/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/MMCluster_v1.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/StoreGateSvc.h"

namespace {
    static const std::string preFixStr{"Mm_"};
    template <class T>
    using VectorAccesor = SG::AuxElement::Accessor<std::vector<T>>;  
}



namespace xAOD {
IdentifierHash MMCluster_v1::measurementHash() const {
    return MuonGMR4::MmReadoutElement::createHash(channelNumber(), gasGap());
}
IMPLEMENT_SETTER_GETTER(MMCluster_v1, uint16_t, time, setTime)
IMPLEMENT_SETTER_GETTER(MMCluster_v1, uint32_t, charge, setCharge)
IMPLEMENT_SETTER_GETTER(MMCluster_v1, float, driftDist, setDriftDist)
IMPLEMENT_SETTER_GETTER(MMCluster_v1, float, angle, setAngle)
IMPLEMENT_SETTER_GETTER(MMCluster_v1, float, chiSqProb, setChiSqProb)
IMPLEMENT_SETTER_GETTER(MMCluster_v1, uint8_t, gasGap, setGasGap)
IMPLEMENT_SETTER_GETTER(MMCluster_v1, uint16_t, channelNumber, setChannelNumber)
IMPLEMENT_SETTER_GETTER_WITH_CAST(MMCluster_v1, short, MMCluster_v1::Author, author, setAuthor)
IMPLEMENT_SETTER_GETTER_WITH_CAST(MMCluster_v1, uint8_t, MMCluster_v1::Quality, quality, setQuality)

IMPLEMENT_VECTOR_SETTER_GETTER(MMCluster_v1, uint16_t, stripNumbers, setStripNumbers)
IMPLEMENT_VECTOR_SETTER_GETTER(MMCluster_v1, int16_t, stripTimes, setStripTimes)
IMPLEMENT_VECTOR_SETTER_GETTER(MMCluster_v1, int, stripCharges, setStripCharges)
IMPLEMENT_VECTOR_SETTER_GETTER(MMCluster_v1, float, stripDriftDist, setStripDriftDist)
IMPLEMENT_VECTOR_SETTER_GETTER(MMCluster_v1, MMCluster_v1::DriftCov_t, stripDriftErrors, setStripDriftErrors)

IMPLEMENT_READOUTELEMENT(MMCluster_v1, m_readoutEle, MmReadoutElement)

void MMCluster_v1::setStripDriftErrors(const std::vector<Amg::MatrixX>& stripDriftErrors) {
    std::vector<DriftCov_t> covariance{};
    std::transform(stripDriftErrors.begin(), stripDriftErrors.end(), 
                   std::back_inserter(covariance), 
                   [](const Amg::MatrixX & cov) {
            DriftCov_t toRet{};
            toRet [0] = cov(0,0);
            toRet [1] = cov(1,1);
            return toRet;
    });
    setStripDriftErrors(covariance);
}

}  // namespace xAOD
