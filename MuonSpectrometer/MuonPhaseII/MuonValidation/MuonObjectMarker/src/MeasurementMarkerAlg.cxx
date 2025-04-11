/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MeasurementMarkerAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "DerivationFrameworkMuons/Utils.h"

namespace MuonR4{

    using PrdCont_t = xAOD::UncalibratedMeasurementContainer;
    using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
    using SegLinkVec_t = std::vector<SegLink_t>;

    using PrdLink_t = ElementLink<PrdCont_t>;
    using PrdLinkVec_t = std::vector<PrdLink_t>;

    using MarkerHandle_t = SG::WriteDecorHandle<PrdCont_t, bool>;
    using LinkHandle_t = SG::WriteDecorHandle<PrdCont_t, SegLinkVec_t>;
    
    using WriteDecorKey_t = SG::WriteDecorHandleKey<xAOD::UncalibratedMeasurementContainer>;
    using namespace DerivationFramework;

    StatusCode MeasurementMarkerAlg::initialize() {
        ATH_CHECK(m_segKey.initialize());
        ATH_CHECK(m_readMarkKey.initialize());
        ATH_CHECK(m_prdLinkKey.initialize());
        ATH_CHECK(m_measKeys.initialize());
        if (m_measKeys.empty()) {
            ATH_MSG_FATAL("Please configure the measurement containers to decorate.");
            return StatusCode::FAILURE;
        }
        const std::string decor = SG::decorKeyFromKey(m_readMarkKey.key());
        for (const auto& key : m_measKeys) {
            m_writeMarkKeys.emplace_back(key, decor);
            if (m_segLink.value().size()) {
                m_writeSegLinkKeys.emplace_back(key, m_segLink);
            }
        }
        ATH_CHECK(m_writeMarkKeys.initialize());
        ATH_CHECK(m_writeSegLinkKeys.initialize());
        return StatusCode::SUCCESS; 
    }
    StatusCode MeasurementMarkerAlg::execute(const EventContext& ctx) const{ 
        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_segKey, ctx));
        SG::ReadDecorHandle<xAOD::MuonSegmentContainer, PrdLinkVec_t> prdLinks{m_prdLinkKey, ctx};
        SG::ReadDecorHandle<xAOD::MuonSegmentContainer, bool> readDecor{m_readMarkKey, ctx};
        /// Ensure that the decoration is actually written

        std::unordered_map<const SG::AuxVectorData*, MarkerHandle_t> writeDecorMap{};
        std::unordered_map<const SG::AuxVectorData*, LinkHandle_t> linkDecorMap{};
        for (const WriteDecorKey_t& decorKey : m_writeMarkKeys) {
            const xAOD::UncalibratedMeasurementContainer* measCont{nullptr};
            ATH_CHECK(SG::get(measCont, decorKey.contHandleKey(), ctx));
            writeDecorMap.emplace(std::make_pair(measCont,  makeHandle(ctx, decorKey, false)));
        }
        for (const WriteDecorKey_t& key : m_writeSegLinkKeys) {
            const xAOD::UncalibratedMeasurementContainer* measCont{nullptr};
            ATH_CHECK(SG::get(measCont, key.contHandleKey(), ctx));
            linkDecorMap.emplace(std::make_pair(measCont,  makeHandle(ctx, key, SegLinkVec_t{})));
        }
        

        for (const xAOD::MuonSegment* seg : *segments) {
            if (!readDecor(*seg)){
                continue;
            }
            SegLink_t segLink{segments, seg->index()};
            for (const PrdLink_t& link : prdLinks(*seg)) {
                const auto* prd = (*link);
                writeDecorMap.at(prd->container())(*prd) = readDecor(*seg);
                if (m_segLink.value().empty()) {
                    continue;
                }
                linkDecorMap.at(prd->container())(*prd).push_back(segLink);
            }
        }
        return StatusCode::SUCCESS; 
    }
}