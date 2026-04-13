/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoTester.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonSpacePoint/SpacePointHelpers.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "CxxUtils/phihelper.h"

namespace {
    void resize_all (const std::size_t nEle, const std::size_t size, auto&&... vecs) {
        for (std::size_t idx = 0; idx < nEle; ++idx) {
            (vecs[idx].resize(size), ...);
        }
    };
}

namespace MuonValR4 {
    using namespace MuonR4;
    using namespace MuonVal;
    using StIndex = Muon::MuonStationIndex::StIndex;
    constexpr std::size_t s_nStations {Acts::toUnderlying(StIndex::StIndexMax)};
    using simHitSet = std::unordered_set<const xAOD::MuonSimHit*>;
    using TruthParticleMap = std::map<const xAOD::TruthParticle*, std::vector<simHitSet>>;
    std::optional<std::size_t> isTruthMatched (const SpacePoint* sp,
                                               const TruthParticleMap& truthHits) {
        for (const xAOD::MuonSimHit* hit : getMatchingSimHits(std::vector<const SpacePoint*>{sp})) {
            std::size_t idx{0};
            for (const auto& [tp, truthHitSets] : truthHits) {
                for (const simHitSet& truthHitSet : truthHitSets) {
                    if (truthHitSet.count(hit)) {
                        if (tp) return idx;
                        else return truthHits.size(); // Pileup muon
                    }
                }
                if (tp) ++idx;
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
        ATH_CHECK(m_roiCollectionKey.initialize(m_isSeededReco));
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
            m_tree.disableBranch(m_pat_nTruthNonPrecMeas.name());
            m_tree.disableBranch(m_pat_nTruthPrecMeas.name());
            m_tree.disableBranch(m_pat_nTruthPhiMeas.name());
            m_tree.disableBranch(m_pat_nAllTruthNonPrecMeas.name());
            m_tree.disableBranch(m_pat_nAllTruthPrecMeas.name());
            m_tree.disableBranch(m_pat_nAllTruthPhiMeas.name());
            m_tree.disableBranch(m_pat_MatchedToTruth.name());
            m_tree.disableBranch(m_pat_nTruthparticles.name());
        }
        if (!m_isSeededReco) {
            m_tree.disableBranch(m_roi_EtaMin.name());
            m_tree.disableBranch(m_roi_EtaMax.name());
            m_tree.disableBranch(m_roi_PhiMin.name());
            m_tree.disableBranch(m_roi_PhiMax.name());
            m_tree.disableBranch(m_roi_ZMin.name());
            m_tree.disableBranch(m_roi_ZMax.name());
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
        ATH_CHECK(SG::get(spContainer, m_spKey, ctx));
            
        const SpacePointContainer* NSWspContainer {nullptr};
        ATH_CHECK(SG::get(NSWspContainer, m_NSWspKey, ctx));

        const GlobalPatternContainer* globPatterns{nullptr};
        ATH_CHECK(SG::get(globPatterns, m_patternKey, ctx));

        const xAOD::MuonSegmentContainer* readTruthSegments{nullptr};
        if(m_isMC){
             ATH_CHECK(SG::get(readTruthSegments , m_truthSegmentKey, ctx));
        }
        const TrigRoiDescriptorCollection* roiCollection{nullptr};
        if(m_isSeededReco) {
            ATH_CHECK(SG::get(roiCollection, m_roiCollectionKey, ctx));
        }
            
        ATH_MSG_DEBUG("Succesfully retrieved input collections: Global Patterns: "<<globPatterns->size()
                    <<", truth segments: "<<(readTruthSegments? readTruthSegments->size() : -1)
                    <<", Rois: "<<(roiCollection ? roiCollection->size() : -1));

        const TruthParticleMap truthMap{fillTruthMap(readTruthSegments, roiCollection)};
        fillTruthInfo(truthMap, readTruthSegments);
        fillRoIInfo(roiCollection);

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
    TruthParticleMap MuonFastRecoTester::fillTruthMap(const xAOD::MuonSegmentContainer* truthSegments,
                                                      const TrigRoiDescriptorCollection* roiCollection) const {
        if (!truthSegments) return TruthParticleMap{};
        ATH_MSG_VERBOSE("Filling truth map with "<<truthSegments->size());
        /** In seeded reco, we want to include only truth particles that are in the RoIs */                                                    
        auto isInROI = [roiCollection](const xAOD::TruthParticle* tp) {
            for (const TrigRoiDescriptor* roi : *roiCollection) {
                /** Check eta */
                if (tp->eta() < roi->etaMinus() || tp->eta() > roi->etaPlus()) continue;
                /** Check phi */
                const double dPhiPlus = CxxUtils::deltaPhi(roi->phiPlus(), tp->phi());
                const double dPhiMinus = CxxUtils::deltaPhi(roi->phiMinus(), tp->phi());
                if (dPhiPlus >= 0. && dPhiMinus <= 0.) return true;
            }
            return false;
        };

        TruthParticleMap truthMap{};
        for (const xAOD::MuonSegment* truth : *truthSegments) {
            if (!truth) continue;
            const xAOD::TruthParticle* tp = getTruthMatchedParticle(*truth);
            // In case of seeded reco, we only save truth particles that are in the RoIs
            if (m_isSeededReco && tp && roiCollection && !isInROI(tp)){
                continue;
            }
            truthMap[tp].push_back(getMatchingSimHits(*truth));
        }
        return truthMap;
    }
    void MuonFastRecoTester::fillTruthInfo(const TruthParticleMap& truthHits,
                                           const xAOD::MuonSegmentContainer* truthSegments) {
        if (!m_isMC || truthHits.empty()) return;
        resize_all(truthHits.size(), s_nStations, 
                   m_gen_nNonPrecMeas, m_gen_nPrecMeas, m_gen_nPhiMeas);
        int tpIdx{-1};
        for (const auto& [tp, _] : truthHits) {
            // We skip pileup muons
            if(!tp) continue;
            
            m_gen_Eta.push_back(tp->eta());
            m_gen_Phi.push_back(tp->phi());
            m_gen_Pt.push_back(tp->pt());
            m_gen_Q.push_back(tp->charge());

            ++tpIdx;
            for (const auto& segment : *truthSegments) {
                if (!segment) continue;
                if (getTruthMatchedParticle(*segment) != tp) continue;
                const auto StIdx {Acts::toUnderlying(toStationIndex(segment->chamberIndex()))};
                m_gen_nNonPrecMeas[tpIdx][StIdx] += segment->nTrigEtaLayers();
                m_gen_nPrecMeas[tpIdx][StIdx] += segment->nPrecisionHits();
                m_gen_nPhiMeas[tpIdx][StIdx] += segment->nPhiLayers();
            }
        }
    }
    void MuonFastRecoTester::fillRoIInfo(const TrigRoiDescriptorCollection* roiCollection) {
        if (!m_isSeededReco || !roiCollection || roiCollection->empty()) return;
        for (const TrigRoiDescriptor* roi : *roiCollection) {
            m_roi_EtaMin.push_back(roi->etaMinus());
            m_roi_EtaMax.push_back(roi->etaPlus());
            m_roi_PhiMin.push_back(roi->phiMinus());
            m_roi_PhiMax.push_back(roi->phiPlus());
            m_roi_ZMin.push_back(roi->zedMinus());
            m_roi_ZMax.push_back(roi->zedPlus());
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
        m_pat_n = patternCont->size();
        // Resize all the matrix branches
        resize_all(patternCont->size(), s_nStations,
                    m_pat_nNonPrecMeas, m_pat_nPrecMeas, m_pat_nPhiMeas, 
                    m_pat_nPileupNonPrecMeas, m_pat_nPileupPrecMeas, m_pat_nPileupPhiMeas,
                    m_pat_nAllNonPrecMeas, m_pat_nAllPrecMeas, m_pat_nAllPhiMeas);
        if (m_isMC) {
            resize_all(patternCont->size(), s_nStations,
                        m_pat_nTruthNonPrecMeas, m_pat_nTruthPrecMeas, m_pat_nTruthPhiMeas,
                        m_pat_nAllTruthNonPrecMeas, m_pat_nAllTruthPrecMeas, m_pat_nAllTruthPhiMeas);
        }
        std::size_t patternIdx{0};
        for (const GlobalPattern* pattern : *patternCont) {

            const std::vector<StIndex> stations {pattern->getStations()};
            for (const StIndex station : stations) {
                for (const SpacePoint* sp : pattern->hitsInStation(station)) {
                    updatePatHitInfo(ePatBranchType::eReco, patternIdx, station, sp);

                    if (!m_isMC) continue;
                    if (const auto tpIdx {isTruthMatched(sp, truthHits)}; tpIdx.has_value()) {
                        // By convention, if truth particle index is equal to the size of the truth map, it means it's a pileup particle
                        updatePatHitInfo(tpIdx.value() < truthHits.size() ? ePatBranchType::eTruth : ePatBranchType::ePileup, 
                                  patternIdx, station, sp);
                        // Save the matched truth particle index in the tree, either is a signal or pileup particle
                        auto& matchedTPs = m_pat_MatchedToTruth[patternIdx];
                        if (std::ranges::find(matchedTPs, tpIdx.value()) == matchedTPs.end()) {
                            matchedTPs.push_back(tpIdx.value());
                        }
                    }
                }
            }
            // Loop over the space point containers to count the number of hits in the buckets crossed by the pattern
            for (const SpacePointContainer* spContainer : spContainers) {
                if (!spContainer) continue;
                for (const SpacePointBucket* bucket : *spContainer) {
                    bool bucketInPattern{false};
                    const StIndex bucketStation {m_idHelperSvc->stationIndex(bucket->front()->identify())};
                    std::vector<const SpacePoint*> allHits{}, allTruthHits{};
                    
                    for (const auto& sp : *bucket) {
                        if (isInPattern(sp.get(), bucketStation, *pattern)) {
                            bucketInPattern = true;
                        }
                        allHits.push_back(sp.get());
                        if (m_isMC && isTruthMatched(sp.get(), truthHits).has_value()) {
                            allTruthHits.push_back(sp.get());
                        }
                    }
                    if (bucketInPattern) {
                        for (const SpacePoint* sp : allHits) {
                            updatePatHitInfo(ePatBranchType::eAll, patternIdx, bucketStation, sp);
                        }
                        for (const SpacePoint* sp : allTruthHits) {
                            updatePatHitInfo(ePatBranchType::eAllTruth, patternIdx, bucketStation, sp);
                        }
                    }
                }
            }

            m_pat_Eta.push_back(-std::log(std::tan(pattern->theta()/2.)));
            m_pat_phi.push_back(pattern->phi());
            m_pat_sector1.push_back(pattern->sector());
            m_pat_sector2.push_back(pattern->secondarySector());
            m_pat_meanNormResidual2.push_back(pattern->meanNormResidual2());
            m_pat_side.push_back(pattern->hitsInStation(stations.front()).front()->msSector()->side());
            m_pat_nStations.push_back(stations.size());

            if(m_isMC) {
                m_pat_nTruthparticles.push_back(std::ranges::count_if(m_pat_MatchedToTruth[patternIdx], 
                    [&truthHits](unsigned char tpIdx){ return tpIdx < truthHits.size(); }));
            }
            patternIdx++;
        }
                                 
    }

    void MuonFastRecoTester::updatePatHitInfo(ePatBranchType type, 
                                              const std::size_t patIdx,
                                              const Muon::MuonStationIndex::StIndex hitSt,
                                              const MuonR4::SpacePoint* sp) {
        using enum ePatBranchType;
        const bool isPrec = MuonR4::isPrecisionHit(*sp);
        const bool isTriggerEta = !isPrec && sp->measuresEta();
        const auto stIdx = Acts::toUnderlying(hitSt);
        
        switch (type) {
            case eReco:
                m_pat_nNonPrecMeas[patIdx][stIdx] += isTriggerEta;
                m_pat_nPrecMeas[patIdx][stIdx] += isPrec;
                m_pat_nPhiMeas[patIdx][stIdx] += sp->measuresPhi();
                return;
            case eTruth:
                m_pat_nTruthNonPrecMeas[patIdx][stIdx] += isTriggerEta;
                m_pat_nTruthPrecMeas[patIdx][stIdx] += isPrec;
                m_pat_nTruthPhiMeas[patIdx][stIdx] += sp->measuresPhi();
                return;
            case eAll:
                m_pat_nAllNonPrecMeas[patIdx][stIdx] += isTriggerEta;
                m_pat_nAllPrecMeas[patIdx][stIdx] += isPrec;
                m_pat_nAllPhiMeas[patIdx][stIdx] += sp->measuresPhi();
                return;
            case ePileup:
                m_pat_nPileupNonPrecMeas[patIdx][stIdx] += isTriggerEta;
                m_pat_nPileupPrecMeas[patIdx][stIdx] += isPrec;
                m_pat_nPileupPhiMeas[patIdx][stIdx] += sp->measuresPhi();
                return;
            case eAllTruth:
                m_pat_nAllTruthNonPrecMeas[patIdx][stIdx] += isTriggerEta;
                m_pat_nAllTruthPrecMeas[patIdx][stIdx] += isPrec;
                m_pat_nAllTruthPhiMeas[patIdx][stIdx] += sp->measuresPhi();
                return;
        }
    }
}  // namespace MuonValR4
