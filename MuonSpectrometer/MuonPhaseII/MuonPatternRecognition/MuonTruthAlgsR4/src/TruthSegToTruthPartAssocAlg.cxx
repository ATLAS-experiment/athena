/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthSegToTruthPartAssocAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthLinks/ElementLink.h"
#include "Identifier/Identifier.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"

#include <unordered_set>

namespace {
    using IdSet_t = std::unordered_set<Identifier>;
    unsigned int countMatched(const std::unordered_set<const xAOD::MuonSimHit*>& simHits,
                              const IdSet_t& matchIds) {
        return std::ranges::count_if(simHits, [&matchIds](const xAOD::MuonSimHit* hit) {
                                return matchIds.count(hit->identify());
                            });
    }
}
namespace MuonR4{
    StatusCode TruthSegToTruthPartAssocAlg::initialize() {
        ATH_CHECK(m_truthKey.initialize());
        for (const std::string& hitIds  : m_simHitIds) {
            m_simHitKeys.emplace_back(m_truthKey, hitIds);
        }
        ATH_CHECK(m_simHitKeys.initialize());
        ATH_CHECK(m_segLinkKey.initialize());
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_truthLinkKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        return StatusCode::SUCCESS;
    }
    StatusCode TruthSegToTruthPartAssocAlg::execute(const EventContext& ctx) const {
        SG::ReadHandle truthParticles{m_truthKey, ctx};
        ATH_CHECK(truthParticles.isPresent());


        using IdDecorHandle_t = SG::ReadDecorHandle<xAOD::TruthParticleContainer, std::vector<unsigned long long>>;
        /// Initialize the Identifier decorators
        std::vector<IdDecorHandle_t> idDecorHandles{};
        for (const SG::ReadDecorHandleKey<xAOD::TruthParticleContainer>& hitKey : m_simHitKeys) {
            idDecorHandles.emplace_back(hitKey, ctx);
        }
        /// Setup a vector with the truth particles and the associated hits
        using IdSet_t = std::unordered_set<Identifier>;
        using TruthTuple_t = std::tuple<const xAOD::TruthParticle*, IdSet_t>;
        std::vector<TruthTuple_t> truthPartWithIds{};
        truthPartWithIds.reserve(truthParticles->size());
        for (const xAOD::TruthParticle* truthMuon : *truthParticles){
            IdSet_t assocIds{};
            ATH_MSG_DEBUG("Truth muon "<<truthMuon->pt()<<", eta: "<<truthMuon->eta()<<", "<<truthMuon->phi()
                         <<", barcode: "<<truthMuon->barcode());
            for (const IdDecorHandle_t& hitDecor : idDecorHandles) {
                std::ranges::transform(hitDecor(*truthMuon), std::inserter(assocIds, assocIds.begin()),
                                       [this](unsigned long long rawId){
                                           const Identifier id{rawId};
                                           ATH_MSG_VERBOSE(" --- associated hit id: "<<m_idHelperSvc->toString(id));
                                           return id; 
                                        });
            }
            truthPartWithIds.emplace_back(std::make_tuple(truthMuon, std::move(assocIds)));
        }
        /// Fetch the segment container
        SG::ReadHandle segments{m_segmentKey, ctx};
        ATH_CHECK(segments.isPresent());
        
        /// Setup the write decorators
        using TruthSegLink_t = std::vector<ElementLink<xAOD::MuonSegmentContainer>>;
        SG::WriteDecorHandle<xAOD::TruthParticleContainer, TruthSegLink_t> segLinkDecor{m_segLinkKey ,ctx};
        using TruthPartLink_t = ElementLink<xAOD::TruthParticleContainer>;
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, TruthPartLink_t> truthLinkDecor{m_truthLinkKey, ctx};


        for (const xAOD::MuonSegment* segment : *segments){
            std::unordered_set<const xAOD::MuonSimHit*> simHits = getMatchingSimHits(*segment);
            ATH_MSG_DEBUG("Reconstructed truth segment "<<SegmentFit::toString(SegmentFit::localSegmentPars(*segment))
                          <<" chamberId: "<<Muon::MuonStationIndex::chName(segment->chamberIndex())
                        <<", phi: "<<segment->sector()<<", nPrecHits: "<<segment->nPrecisionHits()
                        <<", nDoF: "<<segment->numberDoF()<<" sim hits: "<<simHits.size());
            if (msgLvl(MSG::VERBOSE)){
                std::vector<const xAOD::MuonSimHit*> sortedHits{simHits.begin(), simHits.end()};
                std::ranges::sort(sortedHits, [](const xAOD::MuonSimHit* a, const xAOD::MuonSimHit* b){
                    return a->identify() < b->identify();
                });
                for (const xAOD::MuonSimHit* hit: sortedHits) {
                    ATH_MSG_VERBOSE(" --- associated sim hit: "<<m_idHelperSvc->toString(hit->identify())
                           <<", locPos: "<<Amg::toString(xAOD::toEigen(hit->localPosition()))
                           <<", locDir: "<<Amg::toString(xAOD::toEigen(hit->localDirection())) 
                           <<", "<<hit->genParticleLink());
                }
            }
            /* now find the truth particle with all associated hits */
            const auto best_itr = std::ranges::max_element(truthPartWithIds, 
                                                           [&simHits](const TruthTuple_t& truthTupleA,
                                                                      const TruthTuple_t& truthTupleB) {
                return countMatched(simHits, std::get<1>(truthTupleA)) < 
                       countMatched(simHits, std::get<1>(truthTupleB));
            });
            if (best_itr == truthPartWithIds.end()) {
                ATH_MSG_WARNING("No truth particle matched the truth hits of the segment");
                continue;
            }
            if (1.*countMatched(simHits, std::get<1>(*best_itr)) < 0.5* simHits.size()) {
                if (msgLvl(MSG::VERBOSE)) {
                    for (const auto& [truthMuon, assocIds]: truthPartWithIds){
                        std::stringstream unMatchedStr{};
                        unsigned int counts{0};
                        for (const xAOD::MuonSimHit* hit: simHits) {
                            if (!assocIds.count(hit->identify())){
                                unMatchedStr<<" *** "<<m_idHelperSvc->toString(hit->identify())<<std::endl;
                            } else {
                                ++counts;
                            }
                        }
                        if (!counts) continue;
                        ATH_MSG_VERBOSE("Truth muon "<<truthMuon->pt()<<", eta: "<<truthMuon->eta()<<", "<<truthMuon->phi()
                             <<", barcode: "<<truthMuon->barcode()<<", matched hits: "<<counts<<", unmatched: "<<std::endl<<unMatchedStr.str());
                    }
                }
                continue;
            }
            const xAOD::TruthParticle* truthPart{std::get<0>(*best_itr)};
            segLinkDecor(*truthPart).emplace_back(segments.cptr(), segment->index());
            truthLinkDecor(*segment) = TruthPartLink_t{truthParticles.cptr(), truthPart->index()};
        }

        return StatusCode::SUCCESS;
    }
}


