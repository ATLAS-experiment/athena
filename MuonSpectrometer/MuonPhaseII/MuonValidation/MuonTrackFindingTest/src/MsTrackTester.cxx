/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MsTrackTester.h"

#include "StoreGate/ReadHandle.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonTrackEvent/HitSummary.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonPRDTestR4/TrackContainerModule.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "ActsEvent/Decoration.h"

#include "Acts/Definitions/Units.hpp"

#include <format>

using namespace MuonVal;
using namespace MuonPRDTest;
using namespace MuonR4;
using namespace Acts::UnitLiterals;

namespace {
    constexpr double MeVtoGeV = 1.e-3;
    constexpr double toDeg(const double rad) {
        return rad / 1._degree;
    }
    using Location = MsTrackSeed::Location;
 
}

namespace MuonValR4 {
    std::optional<MsTrackSeed> MsTrackTester::makeSeedFromTruth(const ActsTrk::GeometryContext& gctx,
                                                                const xAOD::TruthParticle& truthMuon) const {
        std::vector<const xAOD::MuonSegment*> matchedSegs = MuonR4::getTruthSegments(truthMuon);
        if (matchedSegs.empty() || toLayerIndex(matchedSegs.front()->chamberIndex()) == toLayerIndex(matchedSegs.back()->chamberIndex())) {
            return std::nullopt;
        }
        ExpandedSector sector{matchedSegs[0]->position().phi()};
        /// Construct the 2 candidate seeds
        MsTrackSeed barrelSeed{Location::Barrel, sector};
        MsTrackSeed endcapSeed{Location::Endcap, sector};
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
                <<", phi: "<<toDeg(truthMuon.phi())<<", q: "<<truthMuon.charge()
                <<", matchedSeg: "<<matchedSegs.size()<< " barrel (L/theta): "<<barrelLength
                <<"/"<<barrelTheta<<" - endcap (L/theta): "
                <<endcapLength<<"/"<<endcapTheta<<"\n"<<barrelSeed);
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
               maxTheta{-181.}, minTheta{181};
        for (const xAOD::MuonSegment* seg : seed.segments()) {
            const Amg::Vector2D projPos{m_seedingTool->expressOnCylinder(gctx.context(),*seg, seed.location(), seed.sector())};
            if (!m_seedingTool->withinBounds(projPos, seed.location())) {
                continue;
            }
            const double projected = projPos[seed.location()==Location::Barrel];
            const double theta = seg->direction().theta();
            minL = std::min(minL, projected);
            maxL = std::max(maxL, projected);
            minTheta = std::min(minTheta, toDeg(theta));
            maxTheta = std::max(maxTheta, toDeg(theta));
        }
        return std::make_pair(maxL - minL, maxTheta - minTheta);
    }
    StatusCode MsTrackTester::initialize() {
        ATH_CHECK(m_truthSegmentKey.initialize(m_isMC));
        ATH_CHECK(m_truthKey.initialize(m_isMC));

        ATH_CHECK(m_muonKey.initialize());
        ATH_CHECK(m_msTrkSeedKey.initialize());
        ATH_CHECK(m_recoSegmentKey.initialize());

        ATH_CHECK(m_summaryTool.retrieve());
        ATH_CHECK(m_seedingTool.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_fieldCacheKey.initialize());

        ATH_CHECK(m_legacyMuonKey.initialize(!m_legacyMuonKey.empty()));
        ATH_CHECK(m_legacyTrackKey.initialize(!m_legacyMuonKey.empty()));
        ATH_CHECK(m_legacySegmentKey.initialize(!m_legacyMuonKey.empty()));
        ATH_CHECK(m_idTrackKey.initialize(m_isMC && m_storeID));

        ATH_CHECK(detStore()->retrieve(m_detMgr));

        int evOpts{0};

        m_recoSegs = std::make_unique<SegmentVariables>(m_tree, m_recoSegmentKey.key(), "Segments", msgLevel());
        m_recoSegs->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree, 
            "Segments_passSeedQual",[this](const SG::AuxElement* aux){
            const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
            return m_segSelector->passSeedingQuality(Gaudi::Hive::currentContext(),
                                                     *seg); }));
        m_recoSegs->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree, 
                                "Segments_passTrackQual",[this](const SG::AuxElement* aux){
            const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
            return m_segSelector->passTrackQuality(Gaudi::Hive::currentContext(),
                                                   *seg); 
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
            for (auto loc : {Location::Barrel, Location::Endcap}) {
                m_truthSegs->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree,
                    std::format("TruthSegments_has{}Proj", loc), [loc, this](const SG::AuxElement* aux) -> unsigned short {
                    
                    const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
                    const ActsTrk::GeometryContext* gctx{};
                    (void) SG::get(gctx, m_geoCtxKey, Gaudi::Hive::currentContext()).isSuccess();
                    ExpandedSector sector{seg->position().phi()};
                    const Amg::Vector2D projPos{m_seedingTool->expressOnCylinder(gctx->context(), *seg, loc, sector)};
                    return m_seedingTool->withinBounds(projPos, loc);
                }));
            }

            m_tree.addBranch(m_truthSegs);

            m_truthTrks = std::make_unique<IParticleFourMomBranch>(m_tree, "TruthMuons");
            m_truthTrks->addVariable<int>(-1, "truthOrigin");
            m_truthTrks->addVariable<int>(-1, "truthType");
            /// Link the truth segments to the truth partcle
            m_truthTrks->addVariable(
                std::make_unique<GenericPartDecorBranch<xAOD::TruthParticle,
                                                        std::vector<unsigned short>>>(m_tree, 
                std::format("{:}_truthSegLinks", m_truthTrks->name()), [&] (const xAOD::TruthParticle& p){
                    std::vector<unsigned short> idx{};
                    for (const xAOD::MuonSegment* truthSeg: getTruthSegments(p)){
                        idx.push_back(m_truthSegs->push_back(*truthSeg));
                    }
                    return idx;
                }));
            /// Count the number of matched truth segments
            m_truthTrks->addVariable(
                std::make_unique<GenericPartDecorBranch<xAOD::TruthParticle, unsigned short>>(m_tree, 
                std::format("{:}_nTruthSegments", m_truthTrks->name()), [&] (const xAOD::TruthParticle& p) -> unsigned short {
                    return getTruthSegments(p).size();
                }));

            m_truthTrks->addVariable(
                std::make_unique<GenericPartDecorBranch<xAOD::TruthParticle, float>>(m_tree,
                std::format("{:}_eLossInMS", m_truthTrks->name()), [] (const xAOD::TruthParticle& p) -> float {
                    std::vector<const xAOD::MuonSimHit*> allHits{};
                    for (const xAOD::MuonSegment* seg : getTruthSegments(p)){
                        auto hits = getMatchingSimHits(*seg);
                        allHits.insert(allHits.end(), hits.begin(), hits.end());
                    }
                    if (allHits.empty()) {
                        return 0.;
                    }
                    auto [min, max] = std::ranges::minmax(allHits, [](const xAOD::MuonSimHit* a, const xAOD::MuonSimHit* b){
                        return a->kineticEnergy() < b->kineticEnergy();
                    });
                    return (max->kineticEnergy() - min->kineticEnergy()) / Gaudi::Units::GeV;
                }));
            /// Calculate the truth seed length
            auto cone = std::make_shared<VectorBranch<float>>(m_tree,
                                        std::format("{}_seedThetaCone", m_truthTrks->name()));
            auto qTimesP = std::make_shared<VectorBranch<float>>(m_tree,
                                        std::format("{}_qTimesP", m_truthTrks->name()));

            m_tree.addBranch(cone);
            m_tree.addBranch(qTimesP);
            m_truthTrks->addVariable(
                std::make_unique<GenericPartDecorBranch<xAOD::TruthParticle, float>>(m_tree, 
                std::format("{:}_seedLength", m_truthTrks->name()), [cone, qTimesP, this] (const xAOD::TruthParticle& p) -> float {
                    const ActsTrk::GeometryContext* gctx{};
                    const AtlasFieldCacheCondObj* magCache{nullptr};
                    const EventContext& ctx{Gaudi::Hive::currentContext()};
                    (void) SG::get(gctx, m_geoCtxKey, ctx).isSuccess();
                    (void) SG::get(magCache, m_fieldCacheKey, ctx).isSuccess();
                    MagField::AtlasFieldCache magField{};
                    magCache->getInitializedCache(magField);
                    auto truthSeed = makeSeedFromTruth(*gctx, p);
                    if (!truthSeed) {
                        cone->push_back(-1);
                        qTimesP->push_back(0);
                        return -1.;
                    }
                    auto [length, theta] = calcSeedLength(*gctx, *truthSeed);
                    cone->push_back(theta);
                    qTimesP->push_back(m_seedingTool->estimateQtimesP(gctx->context(), *truthSeed, magField) / Gaudi::Units::GeV);

                    return length;
                }));

            m_tree.addBranch(m_truthTrks);
            m_trkTruthLinks.emplace_back(m_truthSegmentKey, "truthParticleLink");
            m_trkTruthLinks.emplace_back(m_truthKey, "truthSegmentLinks");
            m_trkTruthLinks.emplace_back(m_recoSegmentKey, "truthSegmentLink");
            m_trkTruthLinks.emplace_back(m_muonKey, "truthParticleLink");
        }

        m_tree.addBranch(m_recoSegs);
        m_tree.addBranch(std::make_unique<EventInfoBranch>(m_tree, evOpts));

        static const std::vector<std::string> trackSummaries{
                 // Inner
                "innerSmallHits", "innerLargeHits", "innerSmallHoles", "innerLargeHoles",
                "innerClosePrecisionHits",
                // Middle
                "middleSmallHits", "middleLargeHits", "middleSmallHoles",
                "middleLargeHoles", "middleClosePrecisionHits",
                // Outer
                "outerSmallHits", "outerLargeHits", "outerSmallHoles", "outerLargeHoles",
                "outerClosePrecisionHits",
                // Extended
                "extendedSmallHits", "extendedLargeHits", "extendedSmallHoles",
                "extendedLargeHoles", "extendedClosePrecisionHits",
                "innerTriggerEtaHits", "innerTriggerPhiHits", 
                "middleTriggerEtaHits", "middleTriggerPhiHits",
                "outerTriggerEtaHits", "outerTriggerPhiHits", 

                "innerTriggerEtaHoles", "innerTriggerPhiHoles",
                "middleTriggerEtaHoles", "middleTriggerPhiHoles", 
                "outerTriggerEtaHoles", "outerTriggerPhiHoles",
                };
        if(!m_legacyTrackKey.empty()) {
            m_legacyTrks = std::make_unique<IParticleFourMomBranch>(m_tree, "LegacyMSTrks");
            m_legacyTrks->addVariable(std::make_unique<TrackChi2Branch>(*m_legacyTrks));
            if (m_isMC) {
                BilateralLinkerBranch::connectCollections(m_legacyTrks, m_truthTrks, 
                                                          [](const xAOD::IParticle* trk){ 
                                                            return xAOD::TruthHelpers::getTruthParticle(*trk); 
                                                          }, "truth", "LegacyMS");
            }
            for (const auto& summary : trackSummaries) {
                m_legacyTrks->addVariable<uint8_t>(-1, summary); 
            }
            m_tree.addBranch(m_legacyTrks);
            m_legacyRecoSegs = std::make_unique<SegmentVariables>(m_tree, m_legacySegmentKey.key(), "LegacyRecoSegments", msgLevel());
            m_tree.addBranch(m_legacyRecoSegs);
        }

        m_seedSummary = std::make_shared<TrackSummaryModule>(m_tree, "MsTrkSeed", m_summaryTool.get());
        m_muonTrks = std::make_shared<IParticleFourMomBranch>(m_tree, "ActsMuons");
        m_muonTrks->addVariable(std::make_unique<TrackChi2Branch>(*m_muonTrks));
        m_muonTrks->addVariable(std::make_unique<TrackFitIterBranch>(*m_muonTrks));

        using TrkType = xAOD::Muon::TrackParticleType;
        auto dumpTrack = [&](const std::string& trkName, TrkType type) {
            auto trkColl = std::make_shared<IParticleFourMomBranch>(m_tree, std::format("Acts{:}", trkName));
            trkColl->addVariable(std::make_unique<TrackChi2Branch>(*trkColl));
            if (type != TrkType::InnerDetectorTrackParticle) {
                trkColl->addVariable(std::make_unique<TrackFitIterBranch>(*trkColl));
            }
            trkColl->addVariable(std::make_unique<MaterialRecorderBranch>(*trkColl));
            trkColl->addVariable(std::make_unique<EnergyLossBranch>(*trkColl));
            
            trkColl->addVariable<float>("d0");
            trkColl->addVariable<float>("z0");
            if (type != TrkType::InnerDetectorTrackParticle) {
                for (const auto& summary : trackSummaries) {
                    trkColl->addVariable<uint8_t>(-1, summary); 
                }
            }
            m_muonTrks->addVariable(std::make_unique<MuonVal::LinkerBranch>(*m_muonTrks, trkColl, 
                                    [type](const xAOD::IParticle* muonP) -> const xAOD::IParticle* {
                                        const auto* muon = dynamic_cast<const xAOD::Muon*>(muonP);
                                        if (!muon) {
                                            return nullptr;
                                        }
                                        return muon->trackParticle(type);
                                    }, trkName));
            return trkColl;
        };

        dumpTrack("MsTrk", TrkType::MuonSpectrometerTrackParticle);
        dumpTrack("MeTrk", TrkType::ExtrapolatedMuonSpectrometerTrackParticle);

        m_idTracks = dumpTrack("IdTrk", TrkType::InnerDetectorTrackParticle);
        if (m_isMC) {
            BilateralLinkerBranch::connectCollections(m_idTracks, m_truthTrks, 
                [this](const xAOD::IParticle* trk) -> const xAOD::IParticle* { 
                    const xAOD::TruthParticle* truthTrk = xAOD::TruthHelpers::getTruthParticle(*trk);
                    for (const xAOD::IParticle* part : m_truthTrks->getCached()) {
                        if (truthTrk ==  xAOD::TruthHelpers::getTruthParticle(*part)) {
                            return part;
                        }
                    }
                    return nullptr; 
                }, "truth", "IdTrack");
        }
        
        
        m_muonTrks->addVariable<uint16_t>("allAuthors");
        m_muonTrks->addVariable<uint16_t>("author");
        /// Link the reconstructed segments to the muon
        m_muonTrks->addVariable(std::make_unique<GenericPartDecorBranch<xAOD::Muon,
                                                        std::vector<unsigned short>>>(m_tree, 
                    std::format("{:}_segmentLinks", m_muonTrks->name()), [&] (const xAOD::Muon& p){
                    std::vector<unsigned short> idx{};
                    for (unsigned seg = 0 ; seg < p.nMuonSegments(); ++seg) {
                        idx.push_back(m_recoSegs->push_back(*p.muonSegment(seg)));
                    }
                    return idx;
                }));
        /// Link the associated seed
        m_muonTrks->addVariable(std::make_unique<GenericPartDecorBranch<xAOD::Muon, unsigned short>>(m_tree, 
                    std::format("{:}_seedLink", m_muonTrks->name()), [&] (const xAOD::Muon& p) -> unsigned short {
                        using enum xAOD::Muon::TrackParticleType;
                        const xAOD::TrackParticle* msTrack = p.trackParticle(MuonSpectrometerTrackParticle);
                        if (!msTrack) {
                            return -1;
                        }
                        auto actsTrk = ActsTrk::getActsTrack(*msTrack);
                        if (!actsTrk) {
                            THROW_EXCEPTION("Cannot find the associated ms track from the primary track");
                        }
                        return actsTrk->component<std::size_t>("parentSeed");
                }));

        if (m_isMC) {
            BilateralLinkerBranch::connectCollections(m_muonTrks, m_truthTrks, 
                [](const xAOD::IParticle* trk) -> const xAOD::TruthParticle* { 
                    return xAOD::TruthHelpers::getTruthParticle(*trk); 
                }, "truth", "ActsMuon");
        }

        for (const auto& summary : trackSummaries) {
             m_muonTrks->addVariable<uint8_t>(summary); 
        }
        m_tree.addBranch(m_muonTrks);

        m_tree.addBranch(m_seedSummary);
        
        ATH_CHECK(m_trkTruthLinks.initialize());
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode MsTrackTester::dumpLegacyTracks(const EventContext& ctx) {

        //This for now is to be able to retrieve the matching between the legacy segments and tracks ...
        const xAOD::MuonContainer* legacyMuons{nullptr};
        ATH_CHECK(SG::get(legacyMuons, m_legacyMuonKey, ctx));

        if (!legacyMuons) {
            return StatusCode::SUCCESS;
        }

        for (const xAOD::Muon* muon : *legacyMuons) {
            ATH_MSG_VERBOSE("iMuon " << muon->index() << " pT: "<<(muon->pt() *MeVtoGeV)<<" [GeV], eta: "
                    <<muon->eta() <<", phi: "<<toDeg(muon->phi())<<", q: "<<muon->charge()  
                    <<", nSegments: "<<muon->nMuonSegments());
            const xAOD::TrackParticle* track = muon->trackParticle(xAOD::Muon::TrackParticleType::MuonSpectrometerTrackParticle);
            if (!track) {
                continue;
            }
            m_summaryTool->copySummary(m_summaryTool->makeSummary(ctx, *track->track()), *track);
            m_legacyTrks->push_back(track);
            auto trkIdx = m_legacyTrks->find(track);
            for (size_t s = 0; s < muon->nMuonSegments(); ++s) {
                const xAOD::MuonSegment* segment = muon->muonSegment(s);
                auto segIdx = m_legacyRecoSegs->push_back(*segment);
                ATH_MSG_VERBOSE(std::format( "Legacy muon-segment link: segment {:}  @{:}, eta: {:.2f}, phi {:.2f}", 
                                            printID(*segment), Amg::toString(segment->position()),
                                    segment->direction().eta(), toDeg(segment->direction().phi())));
                m_legacySegToTrkLinks[segIdx] = trkIdx;
            }
        }

        //Dump also legacy segments
        const xAOD::MuonSegmentContainer* legacyRecoSegs{nullptr};
        ATH_CHECK(SG::get(legacyRecoSegs, m_legacySegmentKey, ctx));

        if (legacyRecoSegs->size()) {
            m_legacySegToTrkLinks[legacyRecoSegs->size()-1];
            for (const xAOD::MuonSegment* seg : *legacyRecoSegs) {
                //Store all segments
                m_legacyRecoSegs->push_back(*seg);
            }
        }

        const xAOD::TrackParticleContainer* legacyTrks{nullptr};
        ATH_CHECK(SG::get(legacyTrks, m_legacyTrackKey, ctx));

        for (const xAOD::TrackParticle* track : *legacyTrks) {
            m_summaryTool->copySummary(m_summaryTool->makeSummary(ctx, *track->track()), *track);
            m_legacyTrks->push_back(track);
        }

        return StatusCode::SUCCESS;
    }
    StatusCode MsTrackTester::dumpTruthContent(const EventContext& ctx) {
        if (!m_isMC) {
            return StatusCode::SUCCESS;
        }
        const xAOD::MuonSegmentContainer* truthSegs{nullptr};
        ATH_CHECK(SG::get(truthSegs, m_truthSegmentKey, ctx));


        for (const xAOD::MuonSegment* seg : *truthSegs) {
            ATH_MSG_VERBOSE("Dump truth segment "<<printID(*seg)<<" @"<<
            Amg::toString(seg->position())<<", eta: "<<seg->direction().eta()
            <<", phi: "<<toDeg(seg->direction().phi()));
            m_truthSegs->push_back(*seg);
        }
        if (truthSegs->size()) {
            m_truthSegToRecoLink[truthSegs->size()-1];
        }


        const xAOD::TruthParticleContainer* truthMuons{nullptr};
        ATH_CHECK(SG::get(truthMuons, m_truthKey, ctx));
        const std::size_t nT = truthMuons->size() - 1;
        if (!truthMuons->empty()) {
            /// Allocate the memory
            m_truthMuToSeedIdx[nT];
            m_truthMuToSeedCounter[nT];
            m_truthMuRecoSegLinks[nT];
        }

        for (const xAOD::TruthParticle* truth : *truthMuons) {
            ATH_MSG_DEBUG("Truth muon: pT: "<<(truth->pt() *MeVtoGeV)
                <<", eta: "<<truth->eta()<<", phi: "<<toDeg(truth->phi())<<", q: "<<truth->charge());
            m_truthTrks->push_back(*truth);
        }
        
        return StatusCode::SUCCESS;
    }
    StatusCode MsTrackTester::dumpRecoContent(const EventContext& ctx) {
        const xAOD::MuonContainer* muons{nullptr};
        ATH_CHECK(SG::get(muons, m_muonKey, ctx));
        /** Fetch the containers from store gate */
        const xAOD::MuonSegmentContainer* recoSegments{nullptr};
        ATH_CHECK(SG::get(recoSegments, m_recoSegmentKey, ctx));

        const MuonR4::MsTrackSeedContainer* trkSeeds{nullptr};
        ATH_CHECK(SG::get(trkSeeds, m_msTrkSeedKey, ctx));

        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));


        const AtlasFieldCacheCondObj* magCache{nullptr};
        ATH_CHECK(SG::get(magCache, m_fieldCacheKey, ctx));

        const xAOD::TrackParticleContainer* idTracks{nullptr};
        ATH_CHECK(SG::get(idTracks, m_idTrackKey, ctx));

        /// Collect only the ID tracks that belong to a truth muon of interest.
        if (idTracks) {
            for (const xAOD::TrackParticle* idTrk : *idTracks) {
                if (m_truthTrks->find([idTrk](const xAOD::IParticle* p){
                    return xAOD::TruthHelpers::getTruthParticle(*idTrk) == 
                          xAOD::TruthHelpers::getTruthParticle(*p);
                }) < m_truthTrks->size()) {
                    m_idTracks->push_back(idTrk);
                }
            }
        }

        MagField::AtlasFieldCache magField{};
        magCache->getInitializedCache(magField);
        std::unordered_map<const xAOD::TruthParticle*, 
                           std::vector<unsigned>> truthToSeedMatchCounter{};

        if (!trkSeeds->empty()) {
            m_seedTruthLink[trkSeeds->size() -1];
        }

        for (const MuonR4::MsTrackSeed& seed : *trkSeeds) {
            unsigned int seedIdx = m_seedPos.size();
            m_seedPos += seed.position();
            m_seedType+= Acts::toUnderlying(seed.location());
            m_seedSector += seed.sector().sector();
            m_seedSummary->push_back(ctx, seed);

            auto startPars = m_seedingTool->estimateStartParameters(ctx, seed);
            if (startPars.ok()) {
                m_seedDir += (*startPars).direction();
            } else {
                m_seedDir += Amg::Vector3D::UnitZ();
            }
            // m_seedDir
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
            m_seedQP += m_seedingTool->estimateQtimesP(gctx->context(), seed, magField) / Gaudi::Units::GeV; 
            m_seedGood += startPars.ok();
        }
        /** Link the truth muons to the seeds */
        for (auto& [truthMuon, matches] : truthToSeedMatchCounter) {
            const unsigned tIndex = m_truthTrks->find(truthMuon);
            if (tIndex >= m_truthTrks->size()) {
                continue;
            }
            /** Link the seeds by the number of matched segments */
            while (std::count_if(matches.begin(), matches.end(), 
                   [](const unsigned nMatched){
                       return nMatched > 0;
                   })) {
                auto bestMatch = std::ranges::max_element(matches);
                const std::size_t seedIdx = std::distance(matches.begin(), bestMatch);
                m_truthMuToSeedIdx[tIndex].push_back(seedIdx);

                m_seedTruthLink[seedIdx] = tIndex;
                m_truthMuToSeedCounter[tIndex].push_back(*bestMatch);
                (*bestMatch) = 0;
            }
        }

        for (const xAOD::Muon* muon : *muons) {
            m_muonTrks->push_back(muon);
        }

        for (const xAOD::MuonSegment* seg : *recoSegments) {
            m_recoSegs->push_back(*seg);
        }

        return StatusCode::SUCCESS;
    }
    StatusCode MsTrackTester::execute(const EventContext& ctx) {
        ATH_CHECK(dumpTruthContent(ctx));
        ATH_CHECK(dumpLegacyTracks(ctx));
        ATH_CHECK(dumpRecoContent(ctx));

        ATH_CHECK(m_tree.fill(ctx));
        return StatusCode::SUCCESS;
    }
    StatusCode MsTrackTester::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
}
