/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/sTgcStripCluster_v1.h"

#include "GaudiKernel/ServiceHandle.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/StoreGateSvc.h"

namespace {
    static const std::string preFixStr{"sTgc"};
}
                                                                          
namespace xAOD {
   IMPLEMENT_VECTOR_SETTER_GETTER(sTgcStripCluster_v1, uint16_t, stripNumbers, setStripNumbers);
   IMPLEMENT_VECTOR_SETTER_GETTER(sTgcStripCluster_v1, short int, stripTimes, setStripTimes);
   IMPLEMENT_VECTOR_SETTER_GETTER(sTgcStripCluster_v1, int, stripCharges, setStripCharges);
   IMPLEMENT_SETTER_GETTER_WITH_CAST(sTgcStripCluster_v1, uint8_t, sTgcStripCluster_v1::Quality, quality, setQuality);


}  // namespace xAOD
