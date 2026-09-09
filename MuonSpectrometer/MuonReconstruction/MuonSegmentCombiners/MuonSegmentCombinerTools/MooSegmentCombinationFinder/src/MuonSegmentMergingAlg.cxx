/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/



#include "MuonSegmentMergingAlg.h"

#include "AthContainers/ConstDataVector.h"
#include "MuonSegment/MuonSegment.h"

StatusCode MuonSegmentMergingAlg::initialize() {
    if (m_inKeys.empty()){
        ATH_MSG_FATAL("No containers to merge");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(m_inKeys.initialize());
    ATH_CHECK(m_writeKey.initialize());
    return StatusCode::SUCCESS;
}
StatusCode MuonSegmentMergingAlg::execute(const EventContext& ctx) const {
    
    SG::WriteHandle writeHandle{m_writeKey, ctx};
    
    ConstDataVector<Trk::SegmentCollection> outViewCont{SG::VIEW_ELEMENTS};
    auto outCopyCont = std::make_unique<Trk::SegmentCollection>();
    
    for (const auto& key : m_inKeys) {
        const Trk::SegmentCollection* inColl{nullptr};
        ATH_CHECK(SG::get(inColl, key, ctx));
        if (!m_hardCopy) {
            outViewCont.insert(outViewCont.end(), inColl->begin(), inColl->end());
        } else {
            for (const Trk::Segment* trkSeg : *inColl) {
                const auto* seg = static_cast<const Muon::MuonSegment*>(trkSeg);
                outCopyCont->push_back(std::make_unique<Muon::MuonSegment>(*seg));
            }
        }
    }
    if (m_hardCopy) {
        ATH_CHECK(writeHandle.record(std::move(outCopyCont)));
    } else {
        ATH_CHECK(writeHandle.record(std::make_unique<Trk::SegmentCollection>(*outViewCont.asDataVector())));
    }
    return StatusCode::SUCCESS;
}