
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "RpcMeasViewAlg.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>
#include <AthContainers/ConstDataVector.h>
namespace MuonR4{
    RpcMeasViewAlg::RpcMeasViewAlg(const std::string& name, ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}
    
    StatusCode RpcMeasViewAlg::initialize() {        
        ATH_CHECK(m_readKey1D.initialize());
        ATH_CHECK(m_readKeyBI.initialize());
        ATH_CHECK(m_writeKey.initialize());
        return StatusCode::SUCCESS;
    }
    template <class ContainerType>
        StatusCode RpcMeasViewAlg::retrieveContainer(const EventContext& ctx, 
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


    StatusCode RpcMeasViewAlg::execute(const EventContext& ctx) const {
        const xAOD::RpcStripContainer* legacyStrips{nullptr};
        const xAOD::RpcStrip2DContainer* bilStrips{nullptr};        
        ATH_CHECK(retrieveContainer(ctx, m_readKey1D, legacyStrips));
        ATH_CHECK(retrieveContainer(ctx, m_readKeyBI, bilStrips));        

        ConstDataVector<xAOD::RpcMeasurementContainer> outContainer{SG::VIEW_ELEMENTS};
        if (legacyStrips) {
            outContainer.insert(outContainer.end(), legacyStrips->begin(), legacyStrips->end());
        }
        if (bilStrips) {
            outContainer.insert(outContainer.end(), bilStrips->begin(), bilStrips->end());
        }       
        std::sort(outContainer.begin(), outContainer.end(), 
                  [](const xAOD::RpcMeasurement* a, const xAOD::RpcMeasurement* b){
                    return a->identifier() < b->identifier();
                  });
        
        SG::WriteHandle<xAOD::RpcMeasurementContainer> writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::RpcMeasurementContainer>(*outContainer.asDataVector())));
        return StatusCode::SUCCESS;
    } 
}