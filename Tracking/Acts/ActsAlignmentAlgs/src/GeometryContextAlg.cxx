/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeometryContextAlg.h"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"

// ATHENA
#include "AthenaKernel/IOVInfiniteRange.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "StoreGate/WriteCondHandle.h"


using namespace ActsTrk;
GeometryContextAlg::GeometryContextAlg(const std::string& name, ISvcLocator* pSvcLocator) : 
    AthReentrantAlgorithm(name, pSvcLocator) {}

GeometryContextAlg::~GeometryContextAlg() = default;

StatusCode GeometryContextAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());
    ATH_CHECK(m_alignStoreKeys.initialize());
    ATH_CHECK(m_wchk.initialize());
    return StatusCode::SUCCESS;
}

StatusCode GeometryContextAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());

    SG::WriteHandle<ActsGeometryContext> wch{m_wchk, ctx};

    // create an Acts aware geo alignment store from the one given
    // (this makes a copy for now, which is not ideal)
    std::unique_ptr<ActsGeometryContext> gctx = std::make_unique<ActsGeometryContext>();

    for (const SG::ReadHandleKey<DetectorAlignStore>& key : m_alignStoreKeys) {
        SG::ReadHandle<DetectorAlignStore> alignStore{key, ctx};
        if (!alignStore.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve alignment from " << key.fullKey());
            return StatusCode::FAILURE;
        }
        gctx->setStore(std::make_unique<DetectorAlignStore>(*alignStore));
    }

    // get a nominal alignment store from the tracking geometry service
    // and plug it into a geometry context
    ATH_CHECK(wch.record(std::move(gctx)));
    ATH_MSG_DEBUG("Recorded new " << wch.key());

    return StatusCode::SUCCESS;
}
