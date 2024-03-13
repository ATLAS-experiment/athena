/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsAlignmentCondAlg.h"

// PACKAGE
#include "ActsGeometryInterfaces/AlignmentStore.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"

// ATHENA
#include "AthenaKernel/IOVInfiniteRange.h"
#include "StoreGate/WriteCondHandle.h"

// ACTS
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/DetectorElementBase.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Surfaces/Surface.hpp"

// STL
#include <memory>

using namespace ActsTrk;
ActsAlignmentCondAlg::ActsAlignmentCondAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

ActsAlignmentCondAlg::~ActsAlignmentCondAlg() = default;

StatusCode ActsAlignmentCondAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());
    ATH_CHECK(m_alignStoreKeys.initialize());
    ATH_CHECK(m_wchk.initialize());
    if (m_loadTrkGeoSvc){
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
    }
    
    return StatusCode::SUCCESS;
}

StatusCode ActsAlignmentCondAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());

    EventIDBase now(ctx.eventID());
    SG::WriteCondHandle<ActsGeometryContext> wch{m_wchk, ctx};

    if (wch.isValid(now)) {
        ATH_MSG_DEBUG("CondHandle is already valid for " << now << ". In theory this should not be called, but may happen"
                                                         << " if multiple concurrent events are being processed out of order.");

        return StatusCode::SUCCESS;
    }
    wch.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));

    ATH_MSG_DEBUG("  CondHandle " << wch.key() << " not valid now (" << now << "). Getting new info for dbKey \"" << wch.dbKey()
                                  << "\" from CondDb");

    // create an Acts aware geo alignment store from the one given
    // (this makes a copy for now, which is not ideal)
    std::unique_ptr<ActsGeometryContext> gctx = std::make_unique<ActsGeometryContext>();

    for (const SG::ReadCondHandleKey<RawGeomAlignStore>& key : m_alignStoreKeys) {
        SG::ReadCondHandle<RawGeomAlignStore> alignStore{key, ctx};
        if (!alignStore.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve alignment from " << key.fullKey());
            return StatusCode::FAILURE;
        }
        wch.addDependency(alignStore);
        GeoModel::TransientConstSharedPtr<AlignmentStore>& newStore = gctx->alignmentStores[alignStore->detType];
        if (newStore) {
            ATH_MSG_FATAL("The alignment constants of " << to_string(alignStore->detType) << " are already added to the context");
            return StatusCode::FAILURE;
        }
        newStore = alignStore->trackingAlignment;
    }

    // get a nominal alignment store from the tracking geometry service
    // and plug it into a geometry context
    if (m_loadTrkGeoSvc){
        ATH_CHECK(m_trackingGeometrySvc->checkAlignComplete(*gctx));
    }
    ATH_CHECK(wch.record(std::move(gctx)));
    ATH_MSG_INFO("Recorded new " << wch.key() << " "
                                 << " with range " << wch.getRange());

    return StatusCode::SUCCESS;
}
