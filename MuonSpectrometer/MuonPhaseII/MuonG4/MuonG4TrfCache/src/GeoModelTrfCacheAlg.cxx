/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelTrfCacheAlg.h"

#include "StoreGate/WriteCondHandle.h"
#include "AthenaKernel/IOVInfiniteRange.h"

namespace MuonG4{
    StatusCode GeoModelTrfCacheAlg::initialize(){
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_writeKey.initialize());
        try {
            m_type = static_cast<ActsTrk::DetectorType>(m_detType.value());
        } catch (const std::exception& what) {
            ATH_MSG_FATAL("Invalid detType is configured " << m_detType);
            return StatusCode::FAILURE;
        }
        if (m_type == ActsTrk::DetectorType::UnDefined) {
            ATH_MSG_FATAL("Please configure the detType " << m_detType 
                        << " to be something not undefined");
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }
    StatusCode GeoModelTrfCacheAlg::execute(const EventContext& ctx) const {
        SG::WriteCondHandle writeHandle{m_writeKey, ctx};
        if (writeHandle.isValid()) {
            ATH_MSG_DEBUG("Nothing needs to be done for " << ctx.eventID().event_number());
            return StatusCode::SUCCESS;
        }
        writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));

        auto trfCache = std::make_unique<ActsTrk::DetectorAlignStore>(m_type);
        std::ranges::for_each(m_detMgr->getAllReadoutElements(),
                              [&](const MuonGMR4::MuonReadoutElement* re){
                                if (re->detectorType() == m_type){
                                    re->getMaterialGeom()->getAbsoluteTransform(trfCache->geoModelAlignment.get());
                                    re->getMaterialGeom()->clearPositionInfo();
                                }
                              });
        ATH_CHECK(writeHandle.record(std::move(trfCache)));
        return StatusCode::SUCCESS;
    }
}