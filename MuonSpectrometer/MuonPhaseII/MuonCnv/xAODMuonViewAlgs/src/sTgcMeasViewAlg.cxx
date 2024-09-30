
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "sTgcMeasViewAlg.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>
#include <AthContainers/ConstDataVector.h>
namespace MuonR4{
    sTgcMeasViewAlg::sTgcMeasViewAlg(const std::string& name, ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}
    
    StatusCode sTgcMeasViewAlg::initialize() {        
        ATH_CHECK(m_readKeyStrip.initialize());
        ATH_CHECK(m_readKeyWire.initialize());
        ATH_CHECK(m_readKeyPad.initialize());
        ATH_CHECK(m_writeKey.initialize());
        return StatusCode::SUCCESS;
    }
    template <class ContainerType>
        StatusCode sTgcMeasViewAlg::retrieveContainer(const EventContext& ctx, 
                                                      const SG::ReadHandleKey<ContainerType>& key,
                                                      const ContainerType*& contToPush) const {
        contToPush = nullptr;
        if (key.empty()) {
            ATH_MSG_VERBOSE("No key has been parsed for object " << typeid(ContainerType).name());
            return StatusCode::SUCCESS;
        }
        SG::ReadHandle<ContainerType> readHandle{key, ctx};
        ATH_CHECK(readHandle.isPresent());
        contToPush = readHandle.cptr();
        return StatusCode::SUCCESS;
    }


    StatusCode sTgcMeasViewAlg::execute(const EventContext& ctx) const {
        const xAOD::sTgcStripContainer* strips{nullptr};
        const xAOD::sTgcWireContainer* wires{nullptr};
        const xAOD::sTgcPadContainer* pads{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_readKeyStrip, strips));
        ATH_CHECK(retrieveContainer(ctx, m_readKeyWire, wires));
        ATH_CHECK(retrieveContainer(ctx, m_readKeyPad, pads));

        ConstDataVector<xAOD::sTgcMeasContainer> outContainer{SG::VIEW_ELEMENTS};
        if (strips) {
            outContainer.insert(outContainer.end(), strips->begin(), strips->end());
        }
        if (wires) {
            outContainer.insert(outContainer.end(),wires->begin(), wires->end());
        }
        if (pads){
            outContainer.insert(outContainer.end(), pads->begin(), pads->end());
        }
        std::sort(outContainer.begin(), outContainer.end(), 
                  [](const xAOD::sTgcMeasurement* a, const xAOD::sTgcMeasurement* b){
                    return a->identifier() < b->identifier();
                  });
        
        SG::WriteHandle<xAOD::sTgcMeasContainer> writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::sTgcMeasContainer>(*outContainer.asDataVector())));
        return StatusCode::SUCCESS;
    }
 
}