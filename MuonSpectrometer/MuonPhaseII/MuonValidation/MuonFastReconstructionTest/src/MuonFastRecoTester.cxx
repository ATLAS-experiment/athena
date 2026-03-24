/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoTester.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonSpacePoint/SpacePointHelpers.h"
#include "MuonStationIndex/MuonStationIndex.h"


namespace MuonValR4 {
    using namespace MuonR4;
    using namespace MuonVal;
    using StIndex = Muon::MuonStationIndex::StIndex;
    using simHitSet = std::unordered_set<const xAOD::MuonSimHit*>;
    using TruthParticleMap = std::map<const xAOD::TruthParticle*, std::vector<simHitSet>>;
    std::optional<std::size_t> isTruthMatched (const SpacePoint* sp,
                                               const TruthParticleMap& truthHits) {
        for (const xAOD::MuonSimHit* hit : getMatchingSimHits(std::vector<const SpacePoint*>{sp})) {
            std::size_t idx{0};
            for (const auto& [tp, truthHitSets] : truthHits) {
                for (const simHitSet& truthHitSet : truthHitSets) {
                    if (truthHitSet.count(hit)) return idx;
                }
                ++idx;
            }
        }
        return std::nullopt;
    };
    bool isInPattern (const SpacePoint* sp, 
                      const StIndex station,
                      const GlobalPattern& pattern){
        for (const SpacePoint* hit : pattern.hitsInStation(station)) {
            if (hit == sp) return true;
        }
        return false;
    };

    StatusCode MuonFastRecoTester::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        {
            int infoOpts = 0;
            if (m_isMC) infoOpts = EventInfoBranch::isMC;
            m_tree.addBranch(std::make_unique<EventInfoBranch>(m_tree, infoOpts));  
        }       
        ATH_CHECK(m_spKey.initialize(!m_spKey.empty()));
        ATH_CHECK(m_NSWspKey.initialize(!m_NSWspKey.empty()));
        if (m_writeSpacePoints) {
            if (!m_spKey.empty()) {
                m_spTester = std::make_shared<SpacePointTesterModule>(m_tree, m_spKey.key(), msgLevel(), "Muon");
                m_tree.addBranch(m_spTester);
                if (!m_isMC) m_tree.disableBranch(m_spMatchedToTruth.name());
            }
            if (!m_NSWspKey.empty()) {
                m_NSWspTester = std::make_shared<SpacePointTesterModule>(m_tree, m_NSWspKey.key(), msgLevel(), "Nsw");
                m_tree.addBranch(m_NSWspTester);
                if (!m_isMC) m_tree.disableBranch(m_NSWspMatchedToTruth.name());
            }
        } else {
            m_tree.disableBranch(m_spType.name());
            m_tree.disableBranch(m_NSWspType.name());
            m_tree.disableBranch(m_spMatchedToPattern.name());
            m_tree.disableBranch(m_NSWspMatchedToPattern.name());
            m_tree.disableBranch(m_spMatchedToTruth.name());
            m_tree.disableBranch(m_NSWspMatchedToTruth.name());
        }
        if (!m_isMC) {
            m_tree.disableBranch(m_gen_Eta.name());
            m_tree.disableBranch(m_gen_Phi.name());
            m_tree.disableBranch(m_gen_Pt.name());
            m_tree.disableBranch(m_gen_Q.name());
            m_tree.disableBranch(m_pat_nTrueNonPrecSpacePoints.name());
            m_tree.disableBranch(m_pat_nTruePrecSpacePoints.name());
            m_tree.disableBranch(m_pat_nTruePhiSpacePoints.name());
            m_tree.disableBranch(m_pat_MatchedToTruth.name());
            m_tree.disableBranch(m_pat_nTruthparticles.name());
        }
        ATH_CHECK(m_patternKey.initialize());
        ATH_CHECK(m_truthSegmentKey.initialize(!m_truthSegmentKey.empty()));   
        ATH_CHECK(m_tree.init(this)); 
        ATH_CHECK(m_idHelperSvc.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode MuonFastRecoTester::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    StatusCode MuonFastRecoTester::execute()  {
        
        const EventContext& ctx = Gaudi::Hive::currentContext();
        //const ActsTrk::GeometryContext* gctxPtr{nullptr};
        //ATH_CHECK(SG::get(gctxPtr, m_geoCtxKey, ctx));

        const SpacePointContainer* spContainer {nullptr};
        if (!m_spKey.empty()) ATH_CHECK(SG::get(spContainer, m_spKey, ctx));
            
        const SpacePointContainer* NSWspContainer {nullptr};
        if (!m_NSWspKey.empty()) ATH_CHECK(SG::get(NSWspContainer, m_NSWspKey, ctx));

        const GlobalPatternContainer* globPatterns{nullptr};
        ATH_CHECK(SG::get(globPatterns, m_patternKey, ctx));

        const xAOD::MuonSegmentContainer* readTruthSegments{nullptr};
        if(m_isMC){
             ATH_CHECK(SG::get(readTruthSegments , m_truthSegmentKey, ctx));
        }
            
        ATH_MSG_DEBUG("Succesfully retrieved input collections. Global Patterns: "<<globPatterns->size()
                    <<", truth segments: "<<(readTruthSegments? readTruthSegments->size() : -1)<<".");

        const TruthParticleMap truthMap{fillTruthMap(readTruthSegments)};
        fillTruthInfo(truthMap, readTruthSegments);

        if (m_writeSpacePoints) {
            fillSpacePointInfo(spContainer, globPatterns, truthMap, 
                *m_spTester, m_spType, m_spMatchedToPattern, m_spMatchedToTruth);
            fillSpacePointInfo(NSWspContainer, globPatterns, truthMap, 
                *m_NSWspTester, m_NSWspType, m_NSWspMatchedToPattern, m_NSWspMatchedToTruth);
        }
        fillGlobPatternInfo(globPatterns, truthMap, 
            std::vector<const MuonR4::SpacePointContainer*>{spContainer, NSWspContainer});

        ATH_CHECK(m_tree.fill(ctx));
        return StatusCode::SUCCESS;
    }
    TruthParticleMap MuonFastRecoTester::fillTruthMap(const xAOD::MuonSegmentContainer* truthSegments) const {
        TruthParticleMap truthMap{};
        if (!truthSegments) return truthMap;
        for (const xAOD::MuonSegment* truth : *truthSegments) {
            if (!truth) continue;
            const xAOD::TruthParticle* tp = getTruthMatchedParticle(*truth);
            if (!tp) {
                ATH_MSG_WARNING("Truth segment with no matched truth particle.");
                continue;
            }
            truthMap[tp].push_back(getMatchingSimHits(*truth));
        }
        return truthMap;
    }
    void MuonFastRecoTester::fillTruthInfo(const TruthParticleMap& truthHits,
                                           const xAOD::MuonSegmentContainer* truthSegments) {
        using enum eHitType;
        if (!m_isMC || truthHits.empty()) return;
        for (const auto& [tp, _] : truthHits) {
            m_gen_Eta.push_back(tp->eta());
            m_gen_Phi.push_back(tp->phi());
            m_gen_Pt.push_back(tp->pt());
            m_gen_Q.push_back(tp->charge());
            HitCounts tpHitCount{};
            for (const auto& segment : *truthSegments) {
                if (!segment) continue;
                if (getTruthMatchedParticle(*segment) != tp) continue;
                tpHitCount[Acts::toUnderlying(ePrec)] += segment->nPrecisionHits();
                tpHitCount[Acts::toUnderlying(eTriggerEta)] += segment->nTrigEtaLayers();
                tpHitCount[Acts::toUnderlying(ePhi)] += segment->nPhiLayers();
            }
            m_gen_nNonPrecSpacePoints.push_back(tpHitCount[Acts::toUnderlying(eTriggerEta)]);
            m_gen_nPrecSpacePoints.push_back(tpHitCount[Acts::toUnderlying(ePrec)]);
            m_gen_nPhiSpacePoints.push_back(tpHitCount[Acts::toUnderlying(ePhi)]);
        }
    }
    void MuonFastRecoTester::fillSpacePointInfo(const MuonR4::SpacePointContainer* spc,
                                                const MuonR4::GlobalPatternContainer* patternCont,
                                                const TruthParticleMap& truthHits,
                                                SpacePointTesterModule& spTester,
                                                MuonVal::VectorBranch<unsigned char>& spTypeBranch,
                                                MuonVal::MatrixBranch<unsigned char>& spMatchedToPatternBranch,
                                                MuonVal::MatrixBranch<unsigned char>& spMatchedToTruthBranch) const {
        if (!spc) return;
        for (const SpacePointBucket* bucket : *spc) {
            for (const auto& sp : *bucket) {
                spTypeBranch.push_back(MuonR4::isPrecisionHit(*sp) ? 2 : (sp->measuresEta() ? 1 : 3));
                if(!m_isMC) continue;
                if (const auto tpIdx {isTruthMatched(sp.get(), truthHits)}; tpIdx.has_value()) {
                    unsigned treeIdx = spTester.push_back(*sp);
                    spMatchedToTruthBranch[tpIdx.value()].push_back(treeIdx);
                }
            }
        }
        std::size_t patternIdx{0};
        for (const GlobalPattern* pattern : *patternCont) {
            for (const StIndex station : pattern->getStations()) {
                for (const SpacePoint* sp : pattern->hitsInStation(station)) {
                    unsigned treeIdx = spTester.push_back(*sp);
                    spMatchedToPatternBranch[patternIdx].push_back(treeIdx);
                }
            }       
            ++patternIdx;
        }
    }
    void MuonFastRecoTester::fillGlobPatternInfo(const MuonR4::GlobalPatternContainer* patternCont,
                                                 const TruthParticleMap& truthHits,
                                                 const std::vector<const MuonR4::SpacePointContainer*>& spContainers) {
        using enum eHitType;
        auto updateCounts = [](HitCounts& hitCount, const SpacePoint* sp){
            const bool isPrec = MuonR4::isPrecisionHit(*sp);
            const bool isTriggerEta = !isPrec && sp->measuresEta();
            hitCount[Acts::toUnderlying(ePrec)] += isPrec;
            hitCount[Acts::toUnderlying(eTriggerEta)] += isTriggerEta;
            hitCount[Acts::toUnderlying(ePhi)] += sp->measuresPhi();
        };
        m_pat_n = patternCont->size();
        std::size_t patternIdx{0};
        for (const GlobalPattern* pattern : *patternCont) {
            PatternHitCount patHitCount{};
            const std::vector<StIndex> stations {pattern->getStations()};
            for (const StIndex station : stations) {
                for (const SpacePoint* sp : pattern->hitsInStation(station)) {
                    updateCounts(patHitCount.hitCounts, sp);

                    if (!m_isMC) continue;
                    if (const auto tpIdx {isTruthMatched(sp, truthHits)}; tpIdx.has_value()) {
                        updateCounts(patHitCount.trueHitCounts, sp);
                        auto& matchedTPs = m_pat_MatchedToTruth[patternIdx];
                        if (std::ranges::find(matchedTPs, tpIdx.value()) == matchedTPs.end()) {
                            matchedTPs.push_back(tpIdx.value());
                        }
                    }
                }
            }
            for (const SpacePointContainer* spContainer : spContainers) {
                if (!spContainer) continue;
                for (const SpacePointBucket* bucket : *spContainer) {
                    bool bucketInPattern{false};
                    const StIndex bucketStation {m_idHelperSvc->stationIndex(bucket->front()->identify())};
                    std::vector<const SpacePoint*> allHits{};
                    
                    for (const auto& sp : *bucket) {
                        if (isInPattern(sp.get(), bucketStation, *pattern)) {
                            bucketInPattern = true;
                        }
                        allHits.push_back(sp.get());
                    }
                    if (bucketInPattern) {
                        for (const SpacePoint* sp : allHits) {
                            updateCounts(patHitCount.allHitCounts, sp);
                        }
                    }
                }
            }
            m_pat_nNonPrecSpacePoints.push_back(patHitCount.hitCounts[Acts::toUnderlying(eTriggerEta)]);
            m_pat_nPrecSpacePoints.push_back(patHitCount.hitCounts[Acts::toUnderlying(ePrec)]);
            m_pat_nPhiSpacePoints.push_back(patHitCount.hitCounts[Acts::toUnderlying(ePhi)]);

            m_pat_nAllNonPrecSpacePoints.push_back(patHitCount.allHitCounts[Acts::toUnderlying(eTriggerEta)]);
            m_pat_nAllPrecSpacePoints.push_back(patHitCount.allHitCounts[Acts::toUnderlying(ePrec)]);
            m_pat_nAllPhiSpacePoints.push_back(patHitCount.allHitCounts[Acts::toUnderlying(ePhi)]);

            m_pat_Eta.push_back(-std::log(std::tan(pattern->theta()/2.)));
            m_pat_phi.push_back(pattern->phi());
            m_pat_sector1.push_back(pattern->sector());
            m_pat_sector2.push_back(pattern->secondarySector());
            m_pat_residual.push_back(pattern->totalResidual());
            m_pat_side.push_back(pattern->hitsInStation(stations.front()).front()->msSector()->side());
            m_pat_nStations.push_back(stations.size());

            if(m_isMC) {
                /// Fill the truth info
                m_pat_nTrueNonPrecSpacePoints.push_back(patHitCount.trueHitCounts[Acts::toUnderlying(eTriggerEta)]);
                m_pat_nTruePrecSpacePoints.push_back(patHitCount.trueHitCounts[Acts::toUnderlying(ePrec)]);
                m_pat_nTruePhiSpacePoints.push_back(patHitCount.trueHitCounts[Acts::toUnderlying(ePhi)]);

                m_pat_nTruthparticles.push_back(m_pat_MatchedToTruth[patternIdx].size());
            }
            patternIdx++;
        }
                                 
    }
}  // namespace MuonValR4
