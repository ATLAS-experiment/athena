/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoEvent/FastRecoUtils.h"

#include "xAODMuon/MuonSegmentContainer.h"

namespace {
    /** Accessor for the global pattern link */
    using PatternLink = ElementLink<MuonR4::GlobalPatternContainer>;
    static const SG::ConstAccessor<PatternLink> patLinkAcc{"GlobalPatternLink"};

    /** Accessor for the momentum covariance */
    static const SG::Accessor<float> muonQOverPCovAcc{"QOverPCov"};
}

namespace MuonR4::FastReco {

    const GlobalPattern* getParentPattern(const xAOD::MuonSegment& segment) {
        const PatternLink patLink = patLinkAcc(segment);
        if (!patLink.isValid()) {
            return nullptr;
        }
        return *patLink;
    }
    const GlobalPattern* getParentPattern(const xAOD::Muon& muon) {
        using SegLinkVec_t = std::vector<ElementLink<xAOD::MuonSegmentContainer>>;
        const SegLinkVec_t& segLinks = muon.muonSegmentLinks();
        if (segLinks.empty() || !segLinks.front().isValid()) {
            return nullptr;
        }
        const xAOD::MuonSegment* segment = *segLinks.front();
        if (!segment) {
            return nullptr;
        }
        return getParentPattern(*segment);
    }
    double getQOverPCov(const xAOD::Muon& muon) {
        return muonQOverPCovAcc(muon);
    }
}