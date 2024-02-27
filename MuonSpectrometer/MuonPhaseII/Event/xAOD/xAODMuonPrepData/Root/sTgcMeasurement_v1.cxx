/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/sTgcMeasurement_v1.h"

#include "GaudiKernel/ServiceHandle.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/StoreGateSvc.h"

namespace {
    static const std::string preFixStr{"sTgc"};
}
                                                                          
namespace xAOD {

IMPLEMENT_SETTER_GETTER_WITH_CAST(sTgcMeasurement_v1, uint8_t, sTgcMeasurement_v1::Author, author, setAuthor);
IMPLEMENT_SETTER_GETTER(sTgcMeasurement_v1, uint8_t, gasGap, setGasGap)
IMPLEMENT_SETTER_GETTER(sTgcMeasurement_v1, uint16_t, channelNumber, setChannelNumber)
IMPLEMENT_SETTER_GETTER(sTgcMeasurement_v1, short int, time, setTime)
IMPLEMENT_SETTER_GETTER(sTgcMeasurement_v1, int, charge, setCharge)
IMPLEMENT_READOUTELEMENT(sTgcMeasurement_v1, m_readoutEle, sTgcReadoutElement)

IdentifierHash sTgcMeasurement_v1::measurementHash() const {
   return MuonGMR4::sTgcReadoutElement::createHash(gasGap(), channelType(), channelNumber());
}
}  // namespace xAOD
