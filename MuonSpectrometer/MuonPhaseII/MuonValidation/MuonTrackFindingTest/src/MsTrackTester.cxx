/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MsTrackTester.h"

#include "StoreGate/ReadHandle.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"


using namespace MuonVal;
using namespace MuonPRDTest;

namespace {
    using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
    using SegLinkVec_t = std::vector<SegLink_t>;
    static const SG::ConstAccessor<SegLinkVec_t> acc_truthSegLinks{"truthSegmentLinks"};
}

namespace MuonValR4 {
    StatusCode MsTrackTester::initialize() {
        ATH_CHECK(m_truthSegmentKey.initialize(m_isMC));
        ATH_CHECK(m_truthKey.initialize(m_isMC));
        ATH_CHECK(m_msTrkSeedKey.initialize());
        ATH_CHECK(m_recoSegmentKey.initialize());

        int evOpts{0};

        m_recoSegs = std::make_unique<SegmentVariables>(m_tree, m_recoSegmentKey.key(), "Segments", msgLevel());
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
            
            m_tree.addBranch(m_truthSegs);

            m_truthTrks = std::make_unique<IParticleFourMomBranch>(m_tree, "TruthMuons");
            m_tree.addBranch(m_truthTrks);
            m_trkTruthLinks.emplace_back(m_truthSegmentKey, "truthParticleLink");
            m_trkTruthLinks.emplace_back(m_truthKey, "truthSegmentLinks");
            m_trkTruthLinks.emplace_back(m_recoSegmentKey, "truthSegmentLink");
        }

        m_tree.addBranch(m_recoSegs);
        m_tree.addBranch(std::make_unique<EventInfoBranch>(m_tree, evOpts));

        ATH_CHECK(m_trkTruthLinks.initialize());
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode MsTrackTester::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};

        /** Dump first all the MsTrackSeeds. */
        const MuonR4::MsTrackSeedContainer* trkSeeds{nullptr};
        ATH_CHECK(SG::get(trkSeeds, m_msTrkSeedKey, ctx));
        std::map<const xAOD::TruthParticle*, std::vector<unsigned>> truthToSeedMatchCounter{};
        for (const MuonR4::MsTrackSeed& seed : *trkSeeds) {
            unsigned int seedIdx = m_seedPos.size();
            m_seedPos += seed.position();
            for (const xAOD::MuonSegment* seg : seed.segments()) {
                m_seedRecoSegMatch[seedIdx].push_back(m_recoSegs->push_back(*seg));
                const xAOD::MuonSegment* truthSeg = MuonR4::getMatchedTruthSegment(*seg);
                if (!truthSeg) {
                    continue;
                }

                std::vector<unsigned>& matchCounter = truthToSeedMatchCounter[MuonR4::getTruthMatchedParticle(*truthSeg)];
                if (seedIdx >= matchCounter.size()) matchCounter.resize(seedIdx +1);
                ++matchCounter[seedIdx];
            }
        }
        /** Then dump the reconstructed segments */
        const xAOD::MuonSegmentContainer* recoSegments{nullptr};
        ATH_CHECK(SG::get(recoSegments, m_recoSegmentKey, ctx));
        for (const xAOD::MuonSegment* seg : *recoSegments) {
            m_recoSegs->push_back(*seg);
        }
        /** Dump all truth segments */
        const xAOD::MuonSegmentContainer* truthSegs{nullptr};
        ATH_CHECK(SG::get(truthSegs, m_truthSegmentKey, ctx));
        if (truthSegs) {
            for (const xAOD::MuonSegment* seg : *truthSegs) {
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
            m_truthMuToSeedIdx[truthMuons->size() -1];
            m_truthMuToSeedCounter[truthMuons->size() -1];
            m_truthMuTruthSegLinks[truthMuons->size() -1];
            m_truthMuRecoSegLinks[truthMuons->size() -1];

            for (const xAOD::TruthParticle* truth : *truthMuons) {
                m_truthTrks->push_back(*truth);
                unsigned truthIdx = m_truthTrks->find(truth);
                std::vector<unsigned>& matchCounter = truthToSeedMatchCounter[truth];
                
                std::vector<unsigned>::iterator maxSeed = matchCounter.begin();
                while ( (maxSeed = std::ranges::max_element(matchCounter))!=matchCounter.end() && (*maxSeed) > 0) {
                    m_truthMuToSeedIdx[truthIdx].push_back(std::distance(matchCounter.begin(), maxSeed));
                    m_truthMuToSeedCounter[truthIdx].push_back(*maxSeed);
                    (*maxSeed) = 0;
                }

                std::vector<unsigned short>& truthSegLinks = m_truthMuTruthSegLinks[truthIdx];
                for (const SegLink_t& link : acc_truthSegLinks(*truth)) {
                    truthSegLinks.push_back(m_truthSegs->push_back(**link));
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