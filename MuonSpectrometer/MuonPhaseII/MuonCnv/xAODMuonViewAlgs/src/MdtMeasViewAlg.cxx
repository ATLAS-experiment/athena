
/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtMeasViewAlg.h"

#include <StoreGate/WriteHandle.h>
#include <AthContainers/ConstDataVector.h>

#include <ranges>

namespace MuonR4 {
    StatusCode MdtMeasViewAlg::initialize() {
        ATH_CHECK(m_readKey1D.initialize());
        ATH_CHECK(m_readKey2D.initialize());        
        ATH_CHECK(m_writeKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode MdtMeasViewAlg::execute(const EventContext& ctx) const {
        const xAOD::MdtDriftCircleContainer* driftCircles{nullptr};
        const xAOD::MdtTwinDriftCircleContainer* twinCircles{nullptr};

        ATH_CHECK(SG::get(driftCircles, m_readKey1D, ctx));
        ATH_CHECK(SG::get(twinCircles, m_readKey2D, ctx));

        ConstDataVector<xAOD::MdtDriftCircleContainer> outContainer{SG::VIEW_ELEMENTS};
        outContainer.reserve(driftCircles->size() + twinCircles->size());
        std::merge(driftCircles->begin(), driftCircles->end(), twinCircles->begin(), twinCircles->end(),
                   std::back_inserter(outContainer), 
                    [](const xAOD::MdtDriftCircle*a, const xAOD::MdtDriftCircle* b){
                        return a->identifierHash() < b->identifierHash(); 
                    });

        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MdtDriftCircleContainer>(*outContainer.asDataVector())));
        return StatusCode::SUCCESS;
    }
}
