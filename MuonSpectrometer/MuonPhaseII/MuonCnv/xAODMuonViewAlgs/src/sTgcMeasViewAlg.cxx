/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "sTgcMeasViewAlg.h"

#include <StoreGate/WriteHandle.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODMuonViews/ContainerMerge.h>
namespace MuonR4{
    
    StatusCode sTgcMeasViewAlg::initialize() {        
        ATH_CHECK(m_readKeyStrip.initialize());
        ATH_CHECK(m_readKeyWire.initialize());
        ATH_CHECK(m_readKeyPad.initialize());
        ATH_CHECK(m_writeKey.initialize());
        return StatusCode::SUCCESS;
    }
    
    StatusCode sTgcMeasViewAlg::execute(const EventContext& ctx) const {
        const xAOD::sTgcStripContainer* strips{nullptr};
        const xAOD::sTgcWireContainer* wires{nullptr};
        const xAOD::sTgcPadContainer* pads{nullptr};
        ATH_CHECK(SG::get(strips, m_readKeyStrip, ctx));
        ATH_CHECK(SG::get(wires, m_readKeyWire, ctx));
        ATH_CHECK(SG::get(pads, m_readKeyPad, ctx));

        ConstDataVector<xAOD::sTgcMeasContainer> outContainer{SG::VIEW_ELEMENTS};
        mergeContainer(outContainer, *strips, *wires);
        mergeContainer(outContainer, *pads);
        
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::sTgcMeasContainer>(*outContainer.asDataVector())));
        return StatusCode::SUCCESS;
    }
 
}