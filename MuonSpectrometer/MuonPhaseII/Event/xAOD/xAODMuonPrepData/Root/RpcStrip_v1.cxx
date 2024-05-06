/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/RpcStrip_v1.h"


namespace {
    static const std::string preFixStr{"Rpc_"};
}
namespace xAOD {
    IMPLEMENT_SETTER_GETTER(RpcStrip_v1, uint8_t, measuresPhi, setMeasuresPhi)
}  // namespace xAOD
