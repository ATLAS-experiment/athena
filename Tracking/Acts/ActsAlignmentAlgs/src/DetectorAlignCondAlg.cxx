/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "DetectorAlignCondAlg.h"

#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"
#include "GeoModelHelpers/TransformToStringConverter.h"

#include "GeoModelKernel/GeoClearAbsPosAction.h"

using namespace ActsTrk;
DetectorAlignCondAlg::DetectorAlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator) : 
        AthReentrantAlgorithm(name, pSvcLocator) {}

DetectorAlignCondAlg::~DetectorAlignCondAlg() = default;

StatusCode DetectorAlignCondAlg::initialize() {
    ATH_CHECK(m_inputKey.initialize());
    ATH_CHECK(m_outputKey.initialize());
    /// Fill the aligned transformations during algorithm execution. 
    if (m_fillAlignStoreCache) {
        if(m_loadTrkGeoSvc) {
            ATH_CHECK(m_trackingGeoSvc.retrieve());
        }
        if (m_loadDetVolSvc) {
            ATH_CHECK(m_detVolSvc.retrieve());
        }   
    }

    try {
        m_Type = static_cast<DetectorType>(m_detType.value());
    } catch (const std::exception& what) {
        ATH_MSG_FATAL("Invalid detType is configured " << m_detType);
        return StatusCode::FAILURE;
    }
    if (m_Type == DetectorType::UnDefined) {
        ATH_MSG_FATAL("Please configure the deType " << m_detType << " to be something not undefined");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

StatusCode DetectorAlignCondAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<DetectorAlignStore> writeHandle{m_outputKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("Nothing needs to be done for " << ctx.eventID().event_number());
        return StatusCode::SUCCESS;
    }
    SG::ReadCondHandle<GeoAlignmentStore> readHandle{m_inputKey, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve " << m_inputKey.fullKey());
        return StatusCode::FAILURE;
    }
    writeHandle.addDependency(readHandle);
    /// Create the new alignment
    std::unique_ptr<DetectorAlignStore> newAlignment = std::make_unique<DetectorAlignStore>(m_Type);
    newAlignment->geoModelAlignment = std::make_unique<GeoAlignmentStore>(**readHandle);
    newAlignment->geoModelAlignment->clearPosCache();
    /// Process using the tracking geometry
    if (m_fillAlignStoreCache) {
        
        if(m_loadTrkGeoSvc && !m_trackingGeoSvc->populateAlignmentStore(*newAlignment)) {
            ATH_MSG_WARNING("No detector elements of " << to_string(m_Type) << " are part of the tracking geometry");
        }
        if (m_loadDetVolSvc && !m_detVolSvc->populateAlignmentStore(*newAlignment)) {
            ATH_MSG_WARNING("No detector elements of " << to_string(m_Type) << " are part of the detector tracking volumes");
        }
        /// There's no need of the absolute transform cache anymore
        newAlignment->geoModelAlignment.reset();
    }
    ATH_CHECK(writeHandle.record(std::move(newAlignment)));
    return StatusCode::SUCCESS;
}
