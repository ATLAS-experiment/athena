/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoTester.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonSpacePoint/SpacePointHelpers.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "CxxUtils/phihelper.h"

namespace {
    static const Muon::MuonSectorMapping sectorMap{};

    void resize_all (const std::size_t nEle, const std::size_t size, auto&&... vecs) {
        for (std::size_t idx = 0; idx < nEle; ++idx) {
            (vecs[idx].resize(size), ...);
        }
    };
    double inDegrees(double angle) {
        return angle / Gaudi::Units::deg;
    }
}

namespace MuonValR4 {
    using namespace MuonR4;
    using namespace MuonVal;
    using StIndex = Muon::MuonStationIndex::StIndex;
    constexpr std::size_t s_nStations {Acts::toUnderlying(StIndex::StIndexMax)};
    using simHitSet = std::unordered_set<const xAOD::MuonSimHit*>;
    using TruthParticleMap = std::map<const xAOD::TruthParticle*, std::vector<simHitSet>>;
    std::optional<std::size_t> isTruthMatched (const xAOD::MuonMeasurement& meas,
                                               const TruthParticleMap& truthHits) {
        const xAOD::MuonSimHit* hit {getTruthMatchedHit(meas)};
        if (!hit) return std::nullopt;
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
            m_tree.disableBranch(m_pat_MatchedToTruth.name());
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
    StatusCode MuonFastRecoTester::execute(const EventContext& ctx) {
        
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
                    <<", truth segments: "<<(readTruthSegments? std::to_string(readTruthSegments->size()) : std::to_string(-1))
                    <<", Rois: "<<(roiCollection ? std::to_string(roiCollection->size()) : std::to_string(-1)));

        const TruthParticleMap truthMap{fillTruthMap(readTruthSegments, roiCollection)};
        fillTruthInfo(truthMap, {spContainer, NSWspContainer});
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
            const xAOD::TruthParticle* tp = getTruthMatchedParticle(*truth);
            // In case of seeded reco, we only save truth particles that are in the RoIs
            if (!m_isSeededReco || !tp || isInROI(tp)){
                truthMap[tp].push_back(getMatchingSimHits(*truth));
            }    
        }
        return truthMap;
    }
    void MuonFastRecoTester::fillTruthInfo(const TruthParticleMap& truthHits,
                                           const std::vector<const MuonR4::SpacePointContainer*>& spContainers) {
        if (!m_isMC || truthHits.empty()) return;
        resize_all(truthHits.size(), s_nStations, 
                   m_gen_nNonPrecMeas, m_gen_nPrecMeas, m_gen_nPhiMeas);
        using enum measType;
        using LayerBookeeper = std::unordered_map<const MuonGMR4::SpectrometerSector*, std::set<unsigned>>;
        using MeasBookeeper = std::array<std::vector<const SpacePoint*>, Acts::toUnderlying(nTypes)>;
        /** Track which layers already have a spacepoint to avoid double counting. This is done per station. */
        MeasBookeeper measInStation{};
        LayerBookeeper seenEtaLayers{};
        LayerBookeeper seenPhiLayers{};

        auto processMeas = [&truthHits, &measInStation, &seenEtaLayers, &seenPhiLayers, this]
                    (const SpacePoint* sp, const measType type, const bool process2Dmeas, const int tpIdx, const std::size_t stIdx) {
            /** Select the measurement type */
            const bool isPrec {MuonR4::isPrecisionHit(*sp)};
            if ((type == Prec && !isPrec) || 
                (type == NonPrec && (isPrec || !sp->measuresEta())) || (type == Phi && sp->measuresEta())) {
                return;
            }
            /* Check if the hit has already been counted, i.e. duplicated hits. Needed for precision hits, since we can have multiple on the same layer */
            std::vector<const SpacePoint*>& outCont {measInStation[Acts::toUnderlying(type)]};
            if (std::ranges::find_if(outCont, [sp](const SpacePoint* s) { 
                    return s->primaryMeasurement() == sp->primaryMeasurement(); }) != outCont.end()) {
                return;
            }
            /** Select the truth particle */
            if (isTruthMatched(*sp->primaryMeasurement(), truthHits) != tpIdx) return;
 
            const bool isDoubleMatched {sp->dimension() == 2u && sp->secondaryMeasurement() && isTruthMatched(*sp->secondaryMeasurement(), truthHits) == tpIdx};
            if (process2Dmeas && !isDoubleMatched) return;

            /* Check if we have already measurements in the same layer, except for precision hits */
            const unsigned layNum {m_spSorter.sectorLayerNum(*sp)};
            
            if (sp->measuresEta()) {
                const bool isSeenLayer {seenEtaLayers[sp->msSector()].count(layNum) > 0};
                if (isSeenLayer && !sp->isStraw()) return;
                if (!isSeenLayer) seenEtaLayers[sp->msSector()].insert(layNum);
                auto& measCounter = isPrec ? m_gen_nPrecMeas : m_gen_nNonPrecMeas;
                ++measCounter[tpIdx][stIdx];

                // Eta hit can also have phi measurements. For combined spacepoints, need to be check the secondaty measurements. No need for 2D spacepoints.
                if (sp->measuresPhi() && isDoubleMatched) {
                    seenPhiLayers[sp->msSector()].insert(layNum);
                    ++m_gen_nPhiMeas[tpIdx][stIdx];
                    measInStation[Acts::toUnderlying(Phi)].push_back(sp);
                }
            } else {
                if (seenPhiLayers[sp->msSector()].count(layNum) > 0) return;
                seenPhiLayers[sp->msSector()].insert(layNum);
                ++m_gen_nPhiMeas[tpIdx][stIdx];
            }
            outCont.push_back(sp);
            ATH_MSG_VERBOSE("---> "<<(isPrec ? "Prec" : (type == NonPrec ? "Trig" : "Phi "))<< " " << *sp << " in sector "<<sp->msSector()->identString() << " lay "<<layNum);
        };

        int tpIdx{-1};
        for (const auto& [tp, _] : truthHits) {
            if(!tp) continue;
            m_gen_Eta.push_back(tp->eta());
            m_gen_Phi.push_back(tp->phi());
            m_gen_Pt.push_back(tp->pt());
            m_gen_Q.push_back(tp->charge());
            ++tpIdx;
            // Save the sectors compatible for the tp
            std::vector<int> sectors{};
            sectorMap.getSectors(tp->phi(), sectors);
            const int side {tp->eta() > 0 ? 1 : -1};
            /** Filling truth hit counts. We cannot use directly the segments because we have sim hits 
             *  that haven't made it into spacepoints due to inefficiencies */
            ATH_MSG_VERBOSE("tp: " << tpIdx << ", Eta: " << tp->eta() << ", Phi: " << inDegrees(tp->phi()) << ", Pt [GeV]: " << tp->pt() * 1e-3 << ", Q: " << tp->charge());
            for (std::size_t stIdx = 0; stIdx < s_nStations; ++stIdx) {
                ATH_MSG_VERBOSE("\tStation "<< stName(static_cast<StIndex>(stIdx)) << ": matched hits before layer deduplication: ");
                // Clear the bookkeeping structures for the new station
                for (auto& vec : measInStation) vec.clear();
                seenEtaLayers.clear();
                seenPhiLayers.clear();
                // We fill per measurement type, so we give priority to precision hits if they are in the same layer as trigger hits, e.g. sTGCs
                for (const measType type : {Prec, NonPrec, Phi}) {
                    // We fill first 2D measurements and then 1D ones
                    for (const bool process2Dmeas : {true, false}) {
                        for (const auto& spContainer : spContainers) {
                            if (!spContainer) continue;
                            for (const SpacePointBucket* bucket : *spContainer) {
                                /** Select the buckets in this station */
                                if (static_cast<std::size_t>(m_idHelperSvc->stationIndex(bucket->front()->identify())) != stIdx) continue;
                                /** Geometric (sanity) check: sector & side */
                                if (bucket->msSector()->side() != side ||
                                    std::ranges::none_of(sectors, [&](int s){ return bucket->msSector()->sector() == s; })) {
                                    continue;
                                }
                                for (const auto& sp : *bucket) {
                                    /* Process first the precision hits, 2D measurements first and then 1D */
                                    processMeas(sp.get(), type, process2Dmeas, tpIdx, stIdx);
                                }
                            } // End loop over buckets
                        } // End loop over containers
                    } // End loop over "process2Dmeas" flag
                } // End loop over measurement types
                if (msgLevel(MSG::VERBOSE)) {
                    const auto gen_nNonPrecHits {static_cast<unsigned>(m_gen_nNonPrecMeas[tpIdx][stIdx])};
                    const auto gen_nPrecHits    {static_cast<unsigned>(m_gen_nPrecMeas[tpIdx][stIdx])};
                    const auto gen_nPhiHits     {static_cast<unsigned>(m_gen_nPhiMeas[tpIdx][stIdx])};
                    if (gen_nNonPrecHits + gen_nPrecHits + gen_nPhiHits > 0) {
                        ATH_MSG_VERBOSE("\t after deduplication: N trig/Prec/phi meas: " 
                               << gen_nNonPrecHits << "/" << gen_nPrecHits << "/" << gen_nPhiHits);
                    }
                }
                 
            } // End loop over stations      
        } // End loop over truth particles
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
                measType type {MuonR4::isPrecisionHit(*sp) ? measType::Prec : (sp->measuresEta() ? measType::NonPrec : measType::Phi)};
                spTypeBranch.push_back(Acts::toUnderlying(type));
                if(!m_isMC) continue;
                if (const auto tpIdx {isTruthMatched(*sp->primaryMeasurement(), truthHits)}; tpIdx.has_value()) {
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
                        m_pat_nMisTruthNonPrecMeas, m_pat_nMisTruthPrecMeas, m_pat_nMisTruthPhiMeas);
        }
        auto isSecondaryMatched = [&truthHits](const SpacePoint* sp, std::size_t tpIdx) {
            return sp->secondaryMeasurement() && isTruthMatched(*sp->secondaryMeasurement(), truthHits) == tpIdx;
        };
        std::size_t patternIdx{0};
        for (const GlobalPattern* pattern : *patternCont) {
            // We define a map of truth particles and associated hits for the current pattern, so we can determine the main truth particle is multiple are associated
            std::unordered_map<std::size_t, std::vector<const SpacePoint*>> patternTruthHits{};

            const std::vector<StIndex> stations {pattern->getStations()};
            for (const StIndex station : stations) {
                for (const SpacePoint* sp : pattern->hitsInStation(station)) {
                    updatePatHitInfo(ePatBranchType::eReco, patternIdx, station, sp);

                    if (!m_isMC) continue;
                    if (const auto tpIdx {isTruthMatched(*sp->primaryMeasurement(), truthHits)}; tpIdx.has_value()) {
                        // By convention, if truth particle index is equal to the size of the truth map, it means it's a pileup particle
                        if (*tpIdx >= truthHits.size() ) {
                            updatePatHitInfo(ePatBranchType::ePileup, patternIdx, station, sp, isSecondaryMatched(sp, *tpIdx));
                        } else {
                            patternTruthHits[*tpIdx].push_back(sp);
                        }
                    }
                }
            }
            if (!patternTruthHits.empty()) {
                // Sort the TPs by number of matched hits and define the main truth particle as the one with the most hits in the pattern
                std::vector<std::size_t> sortedTPs {};
                std::ranges::transform(patternTruthHits, std::back_inserter(sortedTPs), [](const auto& pair){ return pair.first; });
                std::ranges::sort(sortedTPs, [&patternTruthHits](const auto& a, const auto& b){
                    return patternTruthHits.at(a).size() > patternTruthHits.at(b).size();
                });
                const auto mainTP = sortedTPs.front();
                for (const SpacePoint* sp : patternTruthHits[mainTP]) {
                    const StIndex station {m_idHelperSvc->stationIndex(sp->identify())};
                    updatePatHitInfo(ePatBranchType::eTruth, patternIdx, station, sp, isSecondaryMatched(sp, mainTP));
                }
                // Add truth hits that are matched to other truth particles than the main one
                for (const auto& [tpIdx, hits] : patternTruthHits) {
                    if (tpIdx == mainTP) continue;
                    for (const SpacePoint* sp : hits) {
                        const StIndex station {m_idHelperSvc->stationIndex(sp->identify())};
                        updatePatHitInfo(ePatBranchType::eMismatched, patternIdx, station, sp, isSecondaryMatched(sp, tpIdx));
                    }
                }
                // Save the matched truth particle index in the tree, either is a signal or pileup particle
                std::ranges::transform(sortedTPs, std::back_inserter(m_pat_MatchedToTruth[patternIdx]), [](const auto& tp){ return tp; });
            }
            // Loop over the space point containers to count the number of hits in the buckets crossed by the pattern
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
                            updatePatHitInfo(ePatBranchType::eAll, patternIdx, bucketStation, sp);
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

            if (msgLevel(MSG::VERBOSE)) {
                const auto mainTP {m_pat_MatchedToTruth[patternIdx].size() > 0 ? static_cast<int>(m_pat_MatchedToTruth[patternIdx].front()) : -1};
                unsigned nPrecHits{0}, nNonPrecHits{0}, nPhiHits{0};
                unsigned gen_nPrecHits{0}, gen_nNonPrecHits{0}, gen_nPhiHits{0};
                for (std::size_t stIdx = 0; stIdx < s_nStations; ++stIdx) {
                    nPrecHits += static_cast<unsigned>(m_pat_nTruthPrecMeas[patternIdx][stIdx]);
                    nNonPrecHits += static_cast<unsigned>(m_pat_nTruthNonPrecMeas[patternIdx][stIdx]);
                    nPhiHits += static_cast<unsigned>(m_pat_nTruthPhiMeas[patternIdx][stIdx]);
                    if (mainTP < 0) continue;
                    gen_nPrecHits += static_cast<unsigned>(m_gen_nPrecMeas[mainTP][stIdx]);
                    gen_nNonPrecHits += static_cast<unsigned>(m_gen_nNonPrecMeas[mainTP][stIdx]);
                    gen_nPhiHits += static_cast<unsigned>(m_gen_nPhiMeas[mainTP][stIdx]);
                }
                ATH_MSG_VERBOSE("Pat #" << patternIdx<< ": " << *pattern << "mainTP: "<<std::to_string(mainTP)
                    <<", nTruthMatchedHits Trig: "<<nNonPrecHits<<" / "<<gen_nNonPrecHits<<", Prec: "<<nPrecHits<<" / "<<gen_nPrecHits<<", Phi: "<<nPhiHits<<" / "<<gen_nPhiHits);
            }
            patternIdx++;
        }                        
    }

    void MuonFastRecoTester::updatePatHitInfo(const ePatBranchType type, 
                                              const std::size_t patIdx,
                                              const Muon::MuonStationIndex::StIndex hitSt,
                                              const MuonR4::SpacePoint* sp,
                                              const bool isSecondaryMatched) {
        using enum ePatBranchType;
        const bool isTruthInfo = (type == ePatBranchType::eTruth || type == ePatBranchType::eMismatched);
        const auto updateCounts = [&](unsigned char& nonPrecCount, 
                                      unsigned char& precCount, 
                                      unsigned char& phiCount) {
            if (sp->measuresEta()) {
                if (MuonR4::isPrecisionHit(*sp)) precCount++;
                else nonPrecCount++;
                if (type == ePatBranchType::eTruth) {
                    ATH_MSG_VERBOSE("---> "<<(MuonR4::isPrecisionHit(*sp) ? "Prec" : "Trig")<< " " << *sp << " in sector "<<sp->msSector()->identString() << " lay "<<m_spSorter.sectorLayerNum(*sp));
                }
            }
            if (sp->measuresPhi() && (!isTruthInfo || isSecondaryMatched)) {
                ++phiCount;
                if (type == ePatBranchType::eTruth) {
                    ATH_MSG_VERBOSE("---> "<<"Phi  " << *sp << " in sector "<<sp->msSector()->identString() << " lay "<<m_spSorter.sectorLayerNum(*sp));
                }
            }
        };
        const auto stIdx = Acts::toUnderlying(hitSt);
        switch (type) {
            case eReco:
                updateCounts(m_pat_nNonPrecMeas[patIdx][stIdx], 
                    m_pat_nPrecMeas[patIdx][stIdx], m_pat_nPhiMeas[patIdx][stIdx]);
                return;
            case eTruth:
                updateCounts(m_pat_nTruthNonPrecMeas[patIdx][stIdx], 
                    m_pat_nTruthPrecMeas[patIdx][stIdx], m_pat_nTruthPhiMeas[patIdx][stIdx]);
                return;
            case eMismatched:
                updateCounts(m_pat_nMisTruthNonPrecMeas[patIdx][stIdx], 
                    m_pat_nMisTruthPrecMeas[patIdx][stIdx], m_pat_nMisTruthPhiMeas[patIdx][stIdx]);
                return;
            case eAll:
                updateCounts(m_pat_nAllNonPrecMeas[patIdx][stIdx], 
                    m_pat_nAllPrecMeas[patIdx][stIdx], m_pat_nAllPhiMeas[patIdx][stIdx]);
                return;
            case ePileup:
                updateCounts(m_pat_nPileupNonPrecMeas[patIdx][stIdx], 
                    m_pat_nPileupPrecMeas[patIdx][stIdx], m_pat_nPileupPhiMeas[patIdx][stIdx]);
                return;
        }
    }
}  // namespace MuonValR4
