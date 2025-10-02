
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "RpcMeasViewAlg.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>
#include <AthContainers/ConstDataVector.h>

namespace MuonR4{
    StatusCode RpcMeasViewAlg::initialize() {        
        ATH_CHECK(m_readKey1D.initialize());
        ATH_CHECK(m_readKeyBI.initialize());
        ATH_CHECK(m_writeKey.initialize());
        return StatusCode::SUCCESS;
    }

    StatusCode RpcMeasViewAlg::execute(const EventContext& ctx) const {
        const xAOD::RpcStripContainer* legacyStrips{nullptr};
        const xAOD::RpcStrip2DContainer* bilStrips{nullptr};
        ATH_CHECK(SG::get(legacyStrips, m_readKey1D, ctx));
        ATH_CHECK(SG::get(bilStrips, m_readKeyBI, ctx));

        ConstDataVector<xAOD::RpcMeasurementContainer> outContainer{SG::VIEW_ELEMENTS};
        if (legacyStrips) {
            outContainer.insert(outContainer.end(), legacyStrips->begin(), legacyStrips->end());
        }
        if (bilStrips) {
            outContainer.insert(outContainer.end(), bilStrips->begin(), bilStrips->end());
        }
        SG::WriteHandle<xAOD::RpcMeasurementContainer> writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::RpcMeasurementContainer>(*outContainer.asDataVector())));
        return StatusCode::SUCCESS;
    } 
}