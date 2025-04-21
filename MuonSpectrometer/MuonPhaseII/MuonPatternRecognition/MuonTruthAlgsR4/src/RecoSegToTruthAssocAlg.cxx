/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RecoSegToTruthAssocAlg.h"

#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "DerivationFrameworkMuons/Utils.h"

namespace {
    using TruthLink_t = ElementLink<xAOD::TruthParticleContainer>;
    using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
    using namespace DerivationFramework;
    
    unsigned countMatches(const std::unordered_set<const xAOD::MuonSimHit*>& recoHits,
                          const std::unordered_set<const xAOD::MuonSimHit*>& truthHits){
        return std::ranges::count_if(recoHits, [&truthHits](const xAOD::MuonSimHit* recoHit){
            return truthHits.count(recoHit);
        });
    }
}

namespace MuonR4 {
    StatusCode RecoSegToTruthAssocAlg::initialize() {
        ATH_CHECK(m_truthSegKey.initialize());
        ATH_CHECK(m_truthSegLinkKey.initialize());
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_segPrdLinkKey.initialize());
        ATH_CHECK(m_segTruthSegLinkKey.initialize());
        ATH_CHECK(m_segTruthLinkKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode RecoSegToTruthAssocAlg::execute(const EventContext& ctx) const {
        const xAOD::MuonSegmentContainer* truthSegments{nullptr};
        const xAOD::MuonSegmentContainer* recoSegments{nullptr};

        ATH_CHECK(SG::get(truthSegments, m_truthSegKey, ctx));
        ATH_CHECK(SG::get(recoSegments, m_segmentKey, ctx));

        const SegWithTruthVec_t truthSegMatches = matchSimHits(*truthSegments);
        const SegWithTruthVec_t recoSegMatches = matchSimHits(*recoSegments);
        
        auto dec_truthLink = makeHandle(ctx, m_segTruthLinkKey, TruthLink_t{});
        auto dec_truthSegLink = makeHandle(ctx, m_segTruthSegLinkKey, SegLink_t{});

        for (const SegmentWithTruth& matchMe : recoSegMatches) {
            const xAOD::MuonSegment* bestMatch{nullptr};
            unsigned int bestCount{0};
            ATH_MSG_DEBUG("Try to match segment in "<<Muon::MuonStationIndex::chName(matchMe.segment->chamberIndex())
                        <<", eta: "<<matchMe.segment->etaIndex()<<", sector: "<<matchMe.segment->sector());
            for (const SegmentWithTruth& truthCand :  truthSegMatches) {
                unsigned int candCount = countMatches(matchMe.hits, truthCand.hits);
                if (candCount > bestCount) {
                    ATH_MSG_VERBOSE("Found new candidate with better matches "<<bestCount<<" vs. "<<candCount);
                    candCount = bestCount;
                    bestMatch = truthCand.segment;
                }
            }
            if (!bestMatch) {
                ATH_MSG_DEBUG("No segment match was found ");
                continue;
            }
            ATH_MSG_DEBUG("Found a matching candidate with "<<bestCount<<"/ "<<matchMe.hits.size()<<" hits.");
            dec_truthSegLink(*matchMe.segment) = SegLink_t{truthSegments, bestMatch->index()};
            const xAOD::TruthParticle* truthPart = getTruthMatchedParticle(*bestMatch);
            if (truthPart) {
                const auto* truthCont = static_cast<const xAOD::TruthParticleContainer*>(truthPart->container());
                dec_truthLink(*matchMe.segment) = TruthLink_t(truthCont, truthPart->index());
            }
        }
        return StatusCode::SUCCESS;
    }
    RecoSegToTruthAssocAlg::SegWithTruthVec_t
        RecoSegToTruthAssocAlg::matchSimHits(const xAOD::MuonSegmentContainer& segments) const {
        SegWithTruthVec_t output{};
        for (const xAOD::MuonSegment* seg : segments) {
            SegmentWithTruth candidate{};
            candidate.segment = seg;
            candidate.hits = getMatchingSimHits(*seg);
            /// No hits could be found -> no hope of matching
            if (candidate.hits.empty()) {
                continue;
            }
            output.emplace_back(std::move(candidate));
        }
        return output;
    }
}