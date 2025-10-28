/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MsTrackTester.h"

#include "StoreGate/ReadHandle.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonPRDTestR4/TrackContainerModule.h"

#include "Acts/Definitions/Units.hpp"

#include <format>

using namespace MuonVal;
using namespace MuonPRDTest;
using namespace MuonR4;
using namespace Acts::UnitLiterals;

namespace {
    using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
    using SegLinkVec_t = std::vector<SegLink_t>;
    static const SG::ConstAccessor<SegLinkVec_t> acc_truthSegLinks{"truthSegmentLinks"};
    constexpr double MeVtoGeV = 1.e-3;
    
    using Location = MsTrackSeeder::Location;



    constexpr const std::pair<double, double> absMin(const std::pair<double, double>& a, 
                                                     const std::pair<double, double>& b){
            return a.first < 0 ? b :(a.first < b.first ? a : b);
    }
    constexpr int overlapSector(const int sec1 , const int sec2) {
        constexpr int nSec = Muon::MuonStationIndex::numberOfSectors();
        if (sec2 > sec1) return overlapSector(sec2, sec1);
        if (sec1 == 1 && sec2 == nSec) return 0;
        return sec1 + sec2;
    }   

}

namespace MuonValR4 {
    std::optional<MsTrackSeed> MsTrackTester::makeSeedFromTruth(const ActsTrk::GeometryContext& gctx,
                                                                const xAOD::TruthParticle& truthMuon) const {
        int secMax{-1}, secMin{100};
        std::vector<const xAOD::MuonSegment*> matchedSegs = MuonR4::getTruthSegments(truthMuon);
        if (matchedSegs.empty()) {
            return std::nullopt;
        }
        for (const xAOD::MuonSegment* seg : matchedSegs) {
            secMax = std::max(secMax, seg->sector());
            secMin = std::min(secMin, seg->sector());
        }
        const int orSec = overlapSector(secMax, secMin);
        /// Construct the 2 candidate seeds
        MsTrackSeed barrelSeed{Location::Barrel, orSec};
        MsTrackSeed endcapSeed{Location::Endcap, orSec};
        for (const xAOD::MuonSegment* seg : matchedSegs) {
            barrelSeed.addSegment(seg);
            endcapSeed.addSegment(seg);
        }
        barrelSeed.setPosition(matchedSegs[0]->position());
        endcapSeed.setPosition(matchedSegs[0]->position());
        const auto [barrelLength, barrelTheta] = calcSeedLength(gctx, barrelSeed);
        const auto [endcapLength, endcapTheta] = calcSeedLength(gctx, endcapSeed);
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Constructed new seed from truth muon wih pT:"
                <<(truthMuon.pt()/ Gaudi::Units::GeV)<<" [GeV], eta: "<<truthMuon.eta()
                <<", phi: "<<(truthMuon.phi() / 1._degree)<<", q: "<<truthMuon.charge()
                <<", matchedSeg: "<<matchedSegs.size()<< " barrel (L/theta): "<<barrelLength
                <<"/"<<(barrelTheta / 1._degree)<<" - endcap (L/theta): "
                <<endcapLength<<"/"<<(endcapTheta / 1._degree)<<"\n"<<barrelSeed);
        if (barrelLength < 0 && endcapLength < 0) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Invalid seed");
            return std::nullopt;
        }
        return barrelLength < 0 || std::abs(endcapLength) < barrelLength 
              ? endcapSeed : barrelSeed;
    }

    std::pair<double, double> MsTrackTester::calcSeedLength(const ActsTrk::GeometryContext& gctx, 
                                                            const MuonR4::MsTrackSeed& seed) const {
        double maxL{-1.*Gaudi::Units::km}, minL{1.*Gaudi::Units::km},
               maxTheta{-M_PI}, minTheta{M_PI};
        for (const xAOD::MuonSegment* seg : seed.segments()) {
            const auto secProj = m_seeder->projectorFromSeed(*seg, seed);
            const Amg::Vector2D projPos{m_seeder->expressOnCylinder(gctx, *seg, seed.location(), secProj)};
            if (!m_seeder->withinBounds(projPos, seed.location())) {
                continue;
            }
            const double projected = projPos[seed.location()==Location::Barrel];
            const double theta = seg->direction().theta();
            minL = std::min(minL, projected);
            maxL = std::max(maxL, projected);
            minTheta = std::min(minTheta, theta);
            maxTheta = std::max(maxTheta, theta);
        }
        return std::make_pair(maxL - minL, maxTheta - minTheta);
    }
    StatusCode MsTrackTester::initialize() {
        ATH_CHECK(m_truthSegmentKey.initialize(m_isMC));
        ATH_CHECK(m_truthKey.initialize(m_isMC));
        ATH_CHECK(m_msTrkSeedKey.initialize());
        ATH_CHECK(m_recoSegmentKey.initialize());
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_fieldCacheKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));

        MsTrackSeeder::Config seederCfg{};
        seederCfg.detMgr = m_detMgr;
        seederCfg.seedHalfLength = 2.*Gaudi::Units::m;
        seederCfg.endcapDiscRadius = 40.*Gaudi::Units::m;
        m_seeder =  std::make_unique<MuonR4::MsTrackSeeder>(name(), std::move(seederCfg));

        int evOpts{0};

        m_recoSegs = std::make_unique<SegmentVariables>(m_tree, m_recoSegmentKey.key(), "Segments", msgLevel());
        m_recoSegs->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree, 
            "Segments_passSeedQual",[this](const SG::AuxElement* aux){
            const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
            return m_segSelector->passSeedingQuality(Gaudi::Hive::currentContext(),
                                                     *MuonR4::detailedSegment(*seg)); }));
        m_recoSegs->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree, 
                                "Segments_passTrackQual",[this](const SG::AuxElement* aux){
            const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
            return m_segSelector->passTrackQuality(Gaudi::Hive::currentContext(),
                                                   *MuonR4::detailedSegment(*seg)); 
        }));
        if (m_isMC) {
            evOpts |= EventInfoBranch::isMC;
            m_recoSegs->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree, 
                "Segments_truthSegLink",[this](const SG::AuxElement* aux){
                const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
                const xAOD::MuonSegment* truthS = MuonR4::getMatchedTruthSegment(*seg);
                const unsigned linkIdx = truthS ? m_truthSegs->push_back(*truthS) : -1;
                /// Link the truth segment to the reconstructed segment...
                if (truthS) {
                    m_truthSegToRecoLink.push_back(linkIdx, m_recoSegs->push_back(*seg));
                    const xAOD::TruthParticle* truthMuon = MuonR4::getTruthMatchedParticle(*truthS);
                    if (truthMuon){
                        m_truthTrks->push_back(truthMuon);
                        m_truthMuRecoSegLinks[m_truthTrks->find(truthMuon)].push_back(m_recoSegs->push_back(*seg));
                    }
                }
                return linkIdx;
            }));

            m_truthSegs = std::make_unique<SegmentVariables>(m_tree, m_truthSegmentKey.key(), "TruthSegments", msgLevel());
            
            m_truthSegs->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree, 
                    "TruthSegments_truthLink",[this](const SG::AuxElement* aux){
                    const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
                    const xAOD::TruthParticle* truthP = MuonR4::getTruthMatchedParticle(*seg);
                    m_truthTrks->push_back(truthP);
                    unsigned short linkIdx = m_truthTrks->find(truthP);                    
                    return linkIdx;
                }));
            m_truthSegs->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree,
                "TruthSegments_hasBarrelProj", [this](const SG::AuxElement* aux){
                    const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
                    const ActsTrk::GeometryContext* gctx{nullptr};
                    SG::get(gctx, m_geoCtxKey, Gaudi::Hive::currentContext()).ignore();
                    using enum MuonR4::MsTrackSeeder::SectorProjector;
                    for (const auto proj : {leftOverlap, center, rightOverlap}) {
                        const Amg::Vector2D projPos{m_seeder->expressOnCylinder(*gctx, *seg, Location::Barrel, proj)};
                        if (m_seeder->withinBounds(projPos, Location::Barrel)) {
                            return 1;
                        }
                    }
                    return 0;
                }));

            m_truthSegs->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree,
                "TruthSegments_hasEndcapProj", [this](const SG::AuxElement* aux){
                    const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
                    const ActsTrk::GeometryContext* gctx{nullptr};
                    SG::get(gctx, m_geoCtxKey, Gaudi::Hive::currentContext()).ignore();
                    using enum MuonR4::MsTrackSeeder::SectorProjector;
                    for (const auto proj : {leftOverlap, center, rightOverlap}) {
                        const Amg::Vector2D projPos{m_seeder->expressOnCylinder(*gctx, *seg, Location::Endcap, proj)};
                        if (m_seeder->withinBounds(projPos, Location::Endcap)) {
                            return 1;
                        }
                    }
                    return 0;
                }));
            m_tree.addBranch(m_truthSegs);

            m_truthTrks = std::make_unique<IParticleFourMomBranch>(m_tree, "TruthMuons");
            m_tree.addBranch(m_truthTrks);
            m_trkTruthLinks.emplace_back(m_truthSegmentKey, "truthParticleLink");
            m_trkTruthLinks.emplace_back(m_truthKey, "truthSegmentLinks");
            m_trkTruthLinks.emplace_back(m_recoSegmentKey, "truthSegmentLink");
        }

        m_tree.addBranch(m_recoSegs);
        m_tree.addBranch(std::make_unique<EventInfoBranch>(m_tree, evOpts));
        m_tree.addBranch(std::make_unique<TrackContainerModule>(m_tree, "MsTracks", msgLevel()));

        ATH_CHECK(m_trkTruthLinks.initialize());
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode MsTrackTester::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        /** Fetch the containers from store gate */
        const xAOD::MuonSegmentContainer* recoSegments{nullptr};
        ATH_CHECK(SG::get(recoSegments, m_recoSegmentKey, ctx));
        if (recoSegments->empty()){
            return StatusCode::SUCCESS;
        }
        const MuonR4::MsTrackSeedContainer* trkSeeds{nullptr};
        ATH_CHECK(SG::get(trkSeeds, m_msTrkSeedKey, ctx));
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        const AtlasFieldCacheCondObj* magCache{nullptr};
        ATH_CHECK(SG::get(magCache, m_fieldCacheKey, ctx));
        
        std::map<const xAOD::TruthParticle*, std::vector<unsigned>> truthToSeedMatchCounter{};
        for (const MuonR4::MsTrackSeed& seed : *trkSeeds) {
            unsigned int seedIdx = m_seedPos.size();
            m_seedPos += seed.position();
            m_seedType+= Acts::toUnderlying(seed.location());
            
            ATH_MSG_VERBOSE(" Dump new seed: "<<seed);
            for (const xAOD::MuonSegment* seg : seed.segments()){
                m_seedRecoSegMatch[seedIdx].push_back(m_recoSegs->push_back(*seg));
                if (const xAOD::MuonSegment* truthSeg =  MuonR4::getMatchedTruthSegment(*seg);
                    truthSeg != nullptr) {
                    std::vector<unsigned>& matchCounter = truthToSeedMatchCounter[MuonR4::getTruthMatchedParticle(*truthSeg)];
                    if (seedIdx >= matchCounter.size()) {
                        matchCounter.resize(seedIdx +1);
                    }
                    ++matchCounter[seedIdx];
                }
            }
            const auto[seedLength, theta] = calcSeedLength(*gctx, seed);
            m_seedLength+= seedLength;
            m_seedThetaCone+=theta;
            // m_seedQP +=  m_seeder->estimateQtimesP(*gctx, *magCache, seed) / Gaudi::Units::GeV; 
        }
        for (const xAOD::MuonSegment* seg : *recoSegments) {
            m_recoSegs->push_back(*seg);
        }
        /** Dump all truth segments */
        const xAOD::MuonSegmentContainer* truthSegs{nullptr};
        ATH_CHECK(SG::get(truthSegs, m_truthSegmentKey, ctx));
        if (truthSegs) {
            for (const xAOD::MuonSegment* seg : *truthSegs) {
                ATH_MSG_VERBOSE(std::format( "Dump truth segment: {:}{:}{:}{:}  @{:}, eta: {:.2f}, phi {:.2f}", 
                    chName(seg->chamberIndex()), std::abs(seg->etaIndex()),
                    seg->etaIndex() > 0 ? 'A' : 'C', seg->sector(),
                    Amg::toString(seg->position()),
                    seg->direction().eta(), seg->direction().phi() / 1._degree));

                m_truthSegs->push_back(*seg);
            }
            if (truthSegs->size()) {
                m_truthSegToRecoLink[truthSegs->size()-1];
            }
        }
        /** Finally dump the truth muons */
        const xAOD::TruthParticleContainer* truthMuons{nullptr};
        ATH_CHECK(SG::get(truthMuons, m_truthKey, ctx));
        if (truthMuons && truthMuons->size()) {
            /// Allocate the memory
            m_truthMuToSeedIdx[truthMuons->size() -1];
            m_truthMuToSeedCounter[truthMuons->size() -1];
            m_truthMuTruthSegLinks[truthMuons->size() -1];
            m_truthMuRecoSegLinks[truthMuons->size() -1];

            for (const xAOD::TruthParticle* truth : *truthMuons) {
                ATH_MSG_DEBUG("Truth muon: pT: "<<(truth->pt() *MeVtoGeV)<<", eta: "<<truth->eta()
                             <<", phi: "<<(truth->phi() / 1._degree)<<", q: "<<truth->charge());

                m_truthTrks->push_back(*truth);
                unsigned truthIdx = m_truthTrks->find(truth);
                std::vector<unsigned>& matchCounter = truthToSeedMatchCounter[truth];                
                std::vector<unsigned>::iterator maxSeed = matchCounter.begin();
                /// Match the seeds to the truth particle
                while ( (maxSeed = std::ranges::max_element(matchCounter))!=matchCounter.end() && (*maxSeed) > 0) {
                    m_truthMuToSeedIdx[truthIdx].push_back(std::distance(matchCounter.begin(), maxSeed));
                    m_truthMuToSeedCounter[truthIdx].push_back(*maxSeed);
                    (*maxSeed) = 0;
                }
                
                std::vector<unsigned short>& truthSegLinks = m_truthMuTruthSegLinks[truthIdx];
                const std::vector<const xAOD::MuonSegment*> truthSegs = getTruthSegments(*truth);
                m_truthMuTruthNSegs += truthSegs.size();
                for (const xAOD::MuonSegment* truthSeg: truthSegs) {
                    truthSegLinks.push_back(m_truthSegs->push_back(*truthSeg));
                }

                auto truthSeed = makeSeedFromTruth(*gctx, *truth);
                if (!truthSeed) {
                    m_truthMuonsSeedLength[truthIdx] = -1;
                    m_truthMuonsSeedCone[truthIdx] = -1;
                    m_truthMuonQP[truthIdx] =0;
                } else {
                    const auto [length, cone] = calcSeedLength(*gctx, *truthSeed);
                    m_truthMuonsSeedLength[truthIdx] = length;
                    m_truthMuonsSeedCone [truthIdx]  = cone;
                    m_truthMuonQP[truthIdx]  =  m_seeder->estimateQtimesP(*gctx, *magCache, *truthSeed) / Gaudi::Units::GeV;
                }
                 
            }
        }
        ATH_CHECK(m_tree.fill(ctx));
        return StatusCode::SUCCESS;
    }
    StatusCode MsTrackTester::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
}