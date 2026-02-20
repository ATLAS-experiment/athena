/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SimHitToTruthPartAssocAlg.h"

#include "xAODTruth/TruthVertex.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadHandle.h"

namespace {
    using SimHitVec_t = std::vector<const xAOD::MuonSimHit*>;
    using MCPartSimMap_t = std::unordered_map<HepMC::ConstGenParticlePtr, SimHitVec_t>;
    using IdDecorHandle_t = SG::WriteDecorHandle<xAOD::TruthParticleContainer, std::vector<unsigned long long>>;
}

namespace MuonR4{
    StatusCode SimHitToTruthPartAssocAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_truthKey.initialize());
        ATH_CHECK(m_hitDecorKey.initialize());
        ATH_CHECK(m_simHitKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode SimHitToTruthPartAssocAlg::execute(const EventContext& ctx) const {
        
        const xAOD::MuonSimHitContainer* simHits{nullptr};
        ATH_CHECK(SG::get(simHits, m_simHitKey, ctx));

        MCPartSimMap_t hitIdMap{};
        for (const xAOD::MuonSimHit* hit : *simHits) {
            if (!hit->genParticleLink().isValid()){
                continue;
            }
            const auto& pl{hit->genParticleLink()};
            hitIdMap[pl].push_back(hit);
        }
        
        const xAOD::TruthParticleContainer* truthMuons{nullptr};
        ATH_CHECK(SG::get(truthMuons,m_truthKey, ctx));
        IdDecorHandle_t idDecorator{m_hitDecorKey, ctx};
        for (const xAOD::TruthParticle* muon : *truthMuons) {
            std::vector<unsigned long long>& hitIds{idDecorator(*muon)};
            auto history = HepMC::simulation_history(muon, -1); // Returns a list of unique IDs
            for (const auto& [pl, hits] : hitIdMap) {
                const  auto linkId = HepMC::uniqueID(pl);
                if (std::ranges::any_of(history,[&linkId](const auto uniqueId){
                        return linkId == uniqueId;
                    })){
                    std::ranges::transform(hits, std::back_inserter(hitIds), 
                                          [](const xAOD::MuonSimHit* hit) {
                                                return hit->identify().get_compact();
                                          });
                }
            }
        }
        return StatusCode::SUCCESS;
    }
}