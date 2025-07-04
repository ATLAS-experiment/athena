/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "MlHitDumperAlg.h"

#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include <unordered_map>

namespace MuonR4{
    StatusCode MlHitDumperAlg::initialize() {
        ATH_CHECK(m_spacePointKeys.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());

        ATH_CHECK(m_truthSegKey.initialize());
        ATH_CHECK(m_truthLinkKey.initialize());
        m_tree.addBranch(std::make_unique<MuonVal::EventInfoBranch>(m_tree, MuonVal::EventInfoBranch::isMC));

        
        m_spCollection = std::make_unique<MuonValR4::SpacePointTesterModule>(m_tree, m_spacePointKeys[0].key(), msgLevel());
        m_tree.addBranch(m_spCollection);
        m_muonP4 = std::make_unique<MuonValR4::IParticleFourMomBranch>(m_tree, "truthMuon");
        m_tree.addBranch(m_muonP4);
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode MlHitDumperAlg::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        const ActsGeometryContext* gctx{};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        std::unordered_map<const xAOD::MuonSimHit*, const xAOD::TruthParticle*> hitPartMap{};
        const xAOD::MuonSegmentContainer* truthSegs{nullptr};
        ATH_CHECK(SG::get(truthSegs, m_truthSegKey, ctx));
        for (const xAOD::MuonSegment* segment : *truthSegs) {
            const xAOD::TruthParticle* truthPart = getTruthMatchedParticle(*segment);
            for (const xAOD::MuonSimHit* hit : getMatchingSimHits(*segment)) {
                hitPartMap[hit] = truthPart;
            }
        }


        for (const SG::ReadHandleKey<SpacePointContainer>& key : m_spacePointKeys){
            const SpacePointContainer* spCont{nullptr};
            ATH_CHECK(SG::get(spCont,key, ctx));
            for (const SpacePointBucket* bucket : *spCont) {
                const Amg::Transform3D& lToGlob = bucket->msSector()->localToGlobalTrans(*gctx);
                for (const auto& sp : *bucket) {
                   unsigned idx = m_spCollection->push_back(*sp);
                   m_spGlobPos.set(lToGlob * sp->positionInChamber(), idx);
                   const xAOD::MuonSimHit* simHit = getTruthMatchedHit(*sp->primaryMeasurement());
                   m_muonP4->push_back(hitPartMap[simHit]);
                   m_truthLink[idx] = m_muonP4->find(hitPartMap[simHit]);
                }
            }
        }
        ATH_CHECK(m_tree.fill(ctx));

        return StatusCode::SUCCESS;
    }
    StatusCode MlHitDumperAlg::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
}