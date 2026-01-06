/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EratioMaker.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TrigGepPerf/GepCellMap.h"

namespace Gep {

template <typename T>
GepEratioAlg<T>::GepEratioAlg(const std::string& name, ISvcLocator* pSvcLocator) 
    : AthReentrantAlgorithm(name, pSvcLocator) {
}


template <typename T>
StatusCode GepEratioAlg<T>::initialize() {
    // Init handlers
    ATH_CHECK(m_seedsKey.initialize());
    ATH_CHECK(m_gepCellsKey.initialize());
    
    m_eratioKey = m_seedsKey.key() + "." + m_eratioKey.key();
    ATH_CHECK(m_eratioKey.initialize());
    
    ATH_MSG_INFO("GepEratioAlg initialized successfully");
    ATH_MSG_INFO("  Seeds: " << m_seedsKey.key());
    ATH_MSG_INFO("  CaloCells: " << m_gepCellsKey.key());
    ATH_MSG_INFO("  Output variable: " << m_eratioKey.key());
    
    return StatusCode::SUCCESS;
}


template <typename T>
StatusCode GepEratioAlg<T>::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    // Retrieve form StoreGate
    SG::ReadHandle<DataVector<T>> seedsHandle(m_seedsKey, ctx);
    if (!seedsHandle.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve seeds: " << m_seedsKey.key());
        return StatusCode::FAILURE;
    }
    
    SG::ReadHandle<GepCellMap> caloCellsHandle(m_gepCellsKey, ctx);
    if (!caloCellsHandle.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve calo cells: " << m_gepCellsKey.key());
        return StatusCode::FAILURE;
    }

    // Build classs instance 
    EratioMaker eratioProcessor(*caloCellsHandle);

    // Clone seeds from original container and decorate with Eratio result
    SG::WriteDecorHandle<DataVector<T>, float> decorEratio(m_eratioKey, ctx);
    for(const T* seed : *seedsHandle) {
        // Decorate with eratio
        const EratioObj result = eratioProcessor.makeEratio(ROOT::Math::PtEtaPhiEVector(seed->et(), seed->eta(), seed->phi(), 0));
        decorEratio(*seed) = result.Eratio;
        ATH_MSG_DEBUG("Seed eta: " << result.seedEta << ", Eratio: " << result.Eratio);
    }

    ATH_MSG_DEBUG("Computed Eratio for " << seedsHandle->size() << " seeds");

    return StatusCode::SUCCESS;
}

}