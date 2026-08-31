/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonToTruthAssocAlg.h"


#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h" 
#include "xAODMuonViews/ContainerDecorator.h"

using Link_t = ElementLink<xAOD::TruthParticleContainer>;

namespace {
    Link_t createLink(const xAOD::TruthParticle* truthPart) {
        if (!truthPart) {
            return Link_t{};
        }
        return Link_t{static_cast<const xAOD::TruthParticleContainer*>(truthPart->container()), truthPart->index()};
    }
}

#include <cassert>

namespace MuonR4{
    StatusCode MuonToTruthAssocAlg::initialize() {
        ATH_CHECK(m_truthKey.initialize());
        ATH_CHECK(m_segmentKey.initialize());
        m_decorKeys.emplace_back(m_segmentKey, "truthSegmentLink");
        ATH_CHECK(m_trkKeys.initialize());
        for (const SG::ReadHandleKey<xAOD::TrackParticleContainer>& key : m_trkKeys) {
            m_decorKeys.emplace_back(key, "truthParticleLink");
        }
        ATH_CHECK(m_decorKeys.initialize());
        ATH_CHECK(m_muonKey.initialize());
        ATH_CHECK(m_truthPartLinkKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode MuonToTruthAssocAlg::execute(const EventContext& ctx) const {
        const xAOD::TruthParticleContainer* truthMuons{nullptr};
        ATH_CHECK(SG::get(truthMuons, m_truthKey, ctx));
        /** Fill the map from the truth muon container -> truth particle 
         *  container*/
        std::unordered_map<const xAOD::TruthParticle*,
                           const xAOD::TruthParticle*> truthPartToMuonMap{};
        
        for (const xAOD::TruthParticle* truthMuon : *truthMuons) {
            const xAOD::TruthParticle* truthPart = xAOD::TruthHelpers::getTruthParticle(*truthMuon);
            assert(truthPart != nullptr);
            assert(truthMuon != truthPart);
            ATH_CHECK(truthPartToMuonMap.insert(std::make_pair(truthPart, truthMuon)).second);
        }

        const xAOD::MuonContainer* muons{nullptr};
        ATH_CHECK(SG::get(muons, m_muonKey, ctx));
        xAOD::ContainerDecorator truthLinkDecor{m_truthPartLinkKey, ctx, Link_t{}};
        using enum xAOD::Muon::TrackParticleType;
        for (const xAOD::Muon* muon : *muons) {
            Link_t& truthLink = truthLinkDecor(*muon);
            const xAOD::TrackParticle* idTrack = muon->trackParticle(Primary);
            /// If the ID track has a truth match particle, then use the match from this one. But bend the 
            /// link back to the MuonTruthContainer using the map that's filled at the top.
            if (idTrack != nullptr) {
                const xAOD::TruthParticle* truth = xAOD::TruthHelpers::getTruthParticle(*idTrack);
                truth = truthPartToMuonMap.insert(std::make_pair(truth, truth)).first->second;
                truthLink = createLink(truth);
                if (truth != nullptr ){
                    continue;
                }
            }
            std::vector<std::pair<const xAOD::TruthParticle*, unsigned>> counts{};
            for (unsigned seg =0 ; seg < muon->nMuonSegments(); ++ seg) {
                const xAOD::TruthParticle* truth = getTruthMatchedParticle(*muon->muonSegment(seg));
                auto itr = std::ranges::find_if(counts, [truth](const auto&  known){
                    return known.first == truth;
                });
                if(itr != counts.end()) {
                    ++itr->second;
                } else {
                    counts.emplace_back(std::make_pair(truth , 1));
                }
            }
            if (counts.empty()) {
                continue;
            }
            const auto* truth = std::ranges::max_element(counts, [](const auto& a, const auto& b){
                    return a.second < b.second;
            })->first;
            truthLink = createLink(truth);
        }

        return StatusCode::SUCCESS;
    }
}