/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "AlignStoreProviderAlg.h"

#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteHandle.h"
namespace {
    /** @brief Count how many transforms have ben populated in the store
     *  @param store: Tracking alignment store to check
     *  @param detType: Associated detector type */
    unsigned countPopulated(const ActsTrk::detail::TransformStore& store, const ActsTrk::DetectorType detType) {
        unsigned n{0};
        for (unsigned ticket = 0; ticket < ActsTrk::detail::TrfStoreTicketCounter::distributedTickets(detType); ++ticket) {
             n += store.getTransform(ticket) != nullptr;
        }
        return n;
    }
}

namespace ActsTrk{

AlignStoreProviderAlg::~AlignStoreProviderAlg() = default;



StatusCode AlignStoreProviderAlg::initialize() {
    ATH_CHECK(m_inputKey.initialize(!m_inputKey.empty()));
    ATH_CHECK(m_outputKey.initialize());
    /// Fill the aligned transformations during algorithm execution. 
    if (m_fillAlignStoreCache) {
        ATH_MSG_DEBUG("Setup the tracking geometry service");
        ATH_CHECK(m_trackingGeoSvc.retrieve());
    }
    /// If the provider alg passes through the alignment from
    /// the conditions store, the detector type does not need to be specified
    ATH_MSG_DEBUG("Configuration: "<<m_detType<<" ("<<static_cast<DetectorType>(m_detType.value())
                <<"), "<<m_fillAlignStoreCache<<", "<<m_splitPhysVolCache<<", "
                <<m_splitActsTrfCache<<", inKey: "<<m_inputKey.fullKey()<<", outKey: "<<m_outputKey.fullKey());

    if (!m_inputKey.empty()) {
        return StatusCode::SUCCESS;
    }

    try {
        m_Type = static_cast<DetectorType>(m_detType.value());
    } catch (const std::exception& what) {
        ATH_MSG_FATAL("Invalid detType is configured " << m_detType);
        return StatusCode::FAILURE;
    }
    if (m_Type == DetectorType::UnDefined) {
        ATH_MSG_FATAL("Please configure the detType " << m_detType << " to be something not undefined");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

StatusCode AlignStoreProviderAlg::execute(const EventContext& ctx) const {
    std::unique_ptr<DetectorAlignStore> newAlignment{};
    using TrackingStore = detail::TransformStore;
    TrackingStore::Mode mode{m_fillAlignStoreCache ? TrackingStore::Mode::Block
                                                   : TrackingStore::Mode::LazyFill};
    if (!m_inputKey.empty()) {
        const DetectorAlignStore* inStore{};
        ATH_CHECK(SG::get(inStore, m_inputKey, ctx));
        newAlignment = std::make_unique<DetectorAlignStore>(*inStore);
        /// Setup a separate cache for the full physical volume transfomrations
        if (m_splitPhysVolCache && newAlignment->geoModelAlignment) {
            newAlignment->geoModelAlignment = std::make_unique<GeoAlignmentStore>(*newAlignment->geoModelAlignment);
            newAlignment->geoModelAlignment->clearPosCache();
        }
        if (m_splitActsTrfCache && newAlignment->geoModelAlignment) {
            newAlignment->trackingAlignment = std::make_unique<TrackingStore>(newAlignment->detType, mode);
        }
    } else {
        newAlignment = std::make_unique<DetectorAlignStore>(m_Type, mode);
    }
    /// Cache all transformations at the begining of the event. 
    /// if the conditions alg upstream already did the same, the geoModelAlignment store
    /// was released and hence there's no need to recall this block again
    if (m_fillAlignStoreCache && newAlignment->geoModelAlignment) {
        if(!m_trackingGeoSvc->populateAlignmentStore(*newAlignment)) {
            ATH_MSG_WARNING("No detector elements of " << m_Type << " are part of the tracking geometry");
        }
        /// There's no need of the absolute transform cache anymore
        newAlignment->geoModelAlignment.reset();
    }
    SG::WriteHandle writeHandle{m_outputKey, ctx};
    ATH_MSG_DEBUG("Record alignment store for detector technology "<<newAlignment->detType
                <<" with a capacity of "<<ActsTrk::detail::TrfStoreTicketCounter::distributedTickets(newAlignment->detType)<<". Already populated: "
                <<countPopulated(*newAlignment->trackingAlignment, newAlignment->detType));
    ATH_CHECK(writeHandle.record(std::move(newAlignment)));
    
    return StatusCode::SUCCESS;
}
}