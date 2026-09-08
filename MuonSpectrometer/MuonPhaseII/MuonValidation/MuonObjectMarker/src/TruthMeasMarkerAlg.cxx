/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthMeasMarkerAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonViews/ChamberViewer.h"
#include "xAODMuonViews/ContainerDecorator.h"

namespace MuonR4 {
    using PrdCont_t = xAOD::MuonMeasurementContainer;
    using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
    using SegLinkVec_t = std::vector<SegLink_t>;

    using MarkerHandle_t = xAOD::ContainerDecorator<PrdCont_t, std::uint8_t>;
    using LinkHandle_t = xAOD::ContainerDecorator<PrdCont_t, SegLinkVec_t>;
    
    using WriteDecorKey_t = SG::WriteDecorHandleKey<xAOD::MuonMeasurementContainer>;

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
            m_prdLinkKeys.emplace_back(key, m_simLink);
        }
        ATH_CHECK(m_prdLinkKeys.initialize());
        ATH_CHECK(m_writeMarkKeys.initialize());
        ATH_CHECK(m_writeSegLinkKeys.initialize());
        return StatusCode::SUCCESS; 
    }
    StatusCode TruthMeasMarkerAlg::execute(const EventContext& ctx) const {
        const xAOD::MuonSegmentContainer* segContainer{nullptr};
        ATH_CHECK(SG::get(segContainer, m_segKey, ctx));

        std::unordered_map<const SG::AuxVectorData*, MarkerHandle_t> markers{};
        using namespace Muon::MuonStationIndex;

        using ChamberView_t = xAOD::ChamberViewer<xAOD::MuonMeasurementContainer>;

        std::array<std::vector<ChamberView_t>, 
                    Acts::toUnderlying(TechnologyIndex::TechnologyIndexMax)> techConts{};
        for (const WriteDecorKey_t& key : m_writeMarkKeys) {
            const xAOD::MuonMeasurementContainer* measContainer{nullptr};
            ATH_CHECK(SG::get(measContainer, key.contHandleKey(), ctx));
            if (measContainer->empty()) {
                continue;
            }
            markers.insert(std::make_pair(measContainer, MarkerHandle_t{key, ctx}));
            const TechnologyIndex techIdx = m_idHelperSvc->technologyIndex(measContainer->at(0)->identify());
            techConts[Acts::toUnderlying(techIdx)].emplace_back(ChamberView_t{*measContainer});
        }
        std::unordered_map<const SG::AuxVectorData*, LinkHandle_t> links{};
        for (const WriteDecorKey_t& key : m_writeSegLinkKeys) {
            const xAOD::MuonMeasurementContainer* measContainer{nullptr};
            ATH_CHECK(SG::get(measContainer, key.contHandleKey(), ctx));
            if (measContainer->empty()) {
                continue;
            }
            links.insert(std::make_pair(measContainer, LinkHandle_t{key, ctx}));
        }
        
        auto fetchPrd = [&](const xAOD::MuonSimHit* hit) -> std::vector<const xAOD::MuonMeasurement*> {
            std::vector<const xAOD::MuonMeasurement*> prds{};
            const IdentifierHash idHash{m_idHelperSvc->detElementHash(hit->identify())};
            const TechnologyIndex techIdx = m_idHelperSvc->technologyIndex(hit->identify());
            for (ChamberView_t& prdCont : techConts[Acts::toUnderlying(techIdx)]){
                if (!prdCont.loadView(idHash)) {
                    continue;
                }
                for (const xAOD::MuonMeasurement* prd : prdCont) {
                    if (getTruthMatchedHit(*prd) == hit){
                        ATH_MSG_VERBOSE("Found hit matched to "<<m_idHelperSvc->toString(hit->identify()));
                        prds.emplace_back(prd);
                    }
                }
            }
            return prds;
        };
        
        for (const xAOD::MuonSegment* segment : *segContainer) {
            const auto truthHits{getMatchingSimHits(*segment)};
            
            SegLink_t segLink{segContainer, segment->index()};
            for (const xAOD::MuonSimHit* simHit : truthHits) {
                for (const xAOD::MuonMeasurement* prd : fetchPrd(simHit)) {
                    markers.at(prd->container())(*prd) = true;
                    links.at(prd->container())(*prd).push_back(segLink);
                }
            }
        }
        return StatusCode::SUCCESS;
    }

}
