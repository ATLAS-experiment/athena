/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthMeasMarkerAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "DerivationFrameworkMuons/Utils.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "xAODMuonPrepData/UtilFunctions.h"

namespace MuonR4 {
    using PrdCont_t = xAOD::UncalibratedMeasurementContainer;
    using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
    using SegLinkVec_t = std::vector<SegLink_t>;

    using MarkerHandle_t = SG::WriteDecorHandle<PrdCont_t, bool>;
    using LinkHandle_t = SG::WriteDecorHandle<PrdCont_t, SegLinkVec_t>;
    
    using WriteDecorKey_t = SG::WriteDecorHandleKey<xAOD::UncalibratedMeasurementContainer>;
    using namespace DerivationFramework;

    StatusCode TruthMeasMarkerAlg::initialize() {
        ATH_CHECK(m_segKey.initialize());
        if (m_measKeys.empty()) {
            ATH_MSG_FATAL("Please configure the measurement containers to decorate.");
            return StatusCode::FAILURE;
        }
        ATH_CHECK(m_measKeys.initialize());
        for (const auto& key : m_measKeys) {
            m_writeMarkKeys.emplace_back(key, m_writeMarker);
            m_writeSegLinkKeys.emplace_back(key, m_segLink);
        }
        ATH_CHECK(m_writeMarkKeys.initialize());
        ATH_CHECK(m_writeSegLinkKeys.initialize());
        return StatusCode::SUCCESS; 
    }
    StatusCode TruthMeasMarkerAlg::execute(const EventContext& ctx) const {
        SG::ReadHandle segContainer{m_segKey, ctx};

        std::unordered_map<const SG::AuxVectorData*, MarkerHandle_t> markers{};
        std::unordered_map<Muon::MuonStationIndex::TechnologyIndex, std::vector<const PrdCont_t*>> techConts{};
        for (const WriteDecorKey_t& key : m_writeMarkKeys) {
            SG::ReadHandle readHandle{key.contHandleKey(), ctx};
            ATH_CHECK(readHandle.isPresent());
            if (readHandle->empty()) {
                continue;
            }
            MarkerHandle_t decor{makeHandle(ctx, key, false)};
            markers.insert(std::make_pair(readHandle.cptr(), std::move(decor)));
            const Muon::MuonStationIndex::TechnologyIndex techIdx = 
                 m_idHelperSvc->technologyIndex(xAOD::identify(readHandle->at(0)));
            techConts[techIdx].push_back(readHandle.cptr());
        }
        std::unordered_map<const SG::AuxVectorData*, LinkHandle_t> links{};
        for (const WriteDecorKey_t& key : m_writeSegLinkKeys) {
            SG::ReadHandle readHandle{key.contHandleKey(), ctx};
            if (readHandle->empty()){
                continue;
            }
            LinkHandle_t decor{makeHandle(ctx, key,SegLinkVec_t{})};
            links.insert(std::make_pair(readHandle.cptr(), std::move(decor)));
        }
        
        auto fetchPrd = [&techConts,this](const xAOD::MuonSimHit* hit) -> const xAOD::UncalibratedMeasurement*{
            for (const PrdCont_t* prdCont : techConts[m_idHelperSvc->technologyIndex(hit->identify())]){
                for (const xAOD::UncalibratedMeasurement* prd : *prdCont) {
                    if (getTruthMatchedHit(*prd) == hit){
                        ATH_MSG_VERBOSE("Found hit matched to "<<m_idHelperSvc->toString(hit->identify()));
                        return prd;
                    }
                }
            }
            ATH_MSG_VERBOSE("Nothing could be matched to "<<m_idHelperSvc->toString(hit->identify()));
            return nullptr;
        };
        
        for (const xAOD::MuonSegment* segment : *segContainer) {
            const auto truthHits{getMatchingSimHits(*segment)};
            
            SegLink_t segLink{segContainer.cptr(), segment->index()};
            for (const xAOD::MuonSimHit* simHit : truthHits) {
                const xAOD::UncalibratedMeasurement* prd = fetchPrd(simHit);
                if (!prd) {
                    continue;
                }
                markers.at(prd->container())(*prd) = true;
                links.at(prd->container())(*prd).push_back(segLink);
            }
        }
        return StatusCode::SUCCESS;
    }

}
