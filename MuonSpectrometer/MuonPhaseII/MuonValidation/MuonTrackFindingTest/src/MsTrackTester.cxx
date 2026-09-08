/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MsTrackTester.h"

#include "StoreGate/ReadHandle.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonTrackEvent/HitSummary.h"
#include "MuonTrackEvent/TrackMatchingUtils.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonPRDTestR4/TrackContainerModule.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"
#include "xAODTruth/xAODTruthHelpers.h"

#include "ActsEvent/Decoration.h"
#include "ActsEvent/CaloExtension.h"
#include "ActsInterop/Logger.h"

#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "FourMomUtils/P4Helpers.h"

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

    const MuonR4::MuonTag* findTagMatchingToId(const xAOD::TrackParticle& idTrack,
                                               const SG::ReadHandleKey<MuonR4::MuonTagContainer>& key,
                                               const EventContext& ctx) {
        const MuonR4::MuonTagContainer* muTagCont{nullptr};
        if(!SG::get(muTagCont, key, ctx).isSuccess()){
            THROW_EXCEPTION("Failed to retrieve "<<key.fullKey());
        }
        if (!muTagCont) {
            return nullptr;
        }
        for (const MuonR4::MuonTag* muTag : *muTagCont) {
            if (muTag->idTrack() == &idTrack){
                return muTag;
            }
        }
        return nullptr;
    }

    bool isOnCaloExit(const Acts::Surface& surf) {
        return surf.geometryId().volume() == ActsTrk::detail::GeoVolIds::s_caloEnvelopeID;
    }

    bool isOnCaloExit(const Acts::BoundTrackParameters& pars) {
        return isOnCaloExit(pars.referenceSurface());
    }
    
}

namespace MuonValR4 {
    std::optional<MsTrackSeed> MsTrackTester::makeSeedFromTruth(const Acts::GeometryContext& tgContext,
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
        const auto [barrelLength, barrelTheta] = calcSeedLength(tgContext, barrelSeed);
        const auto [endcapLength, endcapTheta] = calcSeedLength(tgContext, endcapSeed);
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

    std::pair<double, double> MsTrackTester::calcSeedLength(const Acts::GeometryContext& tgContext,
                                                            const MuonR4::MsTrackSeed& seed) const {
        double maxL{-1.*Gaudi::Units::km}, minL{1.*Gaudi::Units::km},
               maxTheta{-181.}, minTheta{181};
        for (const xAOD::MuonSegment* seg : seed.segments()) {
            const Amg::Vector2D projPos{m_seedingTool->expressOnCylinder(tgContext, *seg, seed.location(), seed.sector())};
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

        ATH_CHECK(m_legacyMuonKey.initialize(!m_legacyMuonKey.empty()));
        ATH_CHECK(m_legacyTrackKey.initialize(!m_legacyMuonKey.empty()));
        ATH_CHECK(m_legacySegmentKey.initialize(!m_legacyMuonKey.empty()));
        ATH_CHECK(m_idTrackKey.initialize(m_isMC && m_storeID));
        ATH_CHECK(m_idTagKey.initialize(!m_idTrackKey.empty()));
        ATH_CHECK(m_segTagKey.initialize(!m_idTrackKey.empty()));
        
        ATH_CHECK(m_ctxProvider.initialize());
        ATH_CHECK(m_extrapolationTool.retrieve(EnableTool{m_storeID}));
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        m_logger = makeActsAthenaLogger(this, name());

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
                    const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(Gaudi::Hive::currentContext());
                    ExpandedSector sector{seg->position().phi()};
                    const Amg::Vector2D projPos{m_seedingTool->expressOnCylinder(tgContext, *seg, loc, sector)};
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
                    return (min->kineticEnergy() - max->kineticEnergy()) / Gaudi::Units::GeV;
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
                    
                    const EventContext& ctx{Gaudi::Hive::currentContext()};
                    const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
                    const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
                    MagField::AtlasFieldCache magField{};
                    mfContext.get<const AtlasFieldCacheCondObj*>()->getInitializedCache(magField);
                    auto truthSeed = makeSeedFromTruth(tgContext, p);
                    if (!truthSeed) {
                        cone->push_back(-1);
                        qTimesP->push_back(0);
                        return -1.;
                    }
                    auto [length, theta] = calcSeedLength(tgContext, *truthSeed);
                    cone->push_back(theta);
                    qTimesP->push_back(m_seedingTool->estimateQtimesP(tgContext, *truthSeed, magField) / Gaudi::Units::GeV);
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

        m_seedSummary = std::make_shared<TrackSummaryModule>(m_tree, "TrkSeed", m_summaryTool.get());
        m_muonTrks = std::make_shared<IParticleFourMomBranch>(m_tree, "ActsMuons");
        m_muonTrks->addVariable(std::make_unique<TrackChi2Branch>(*m_muonTrks));
        m_muonTrks->addVariable<float>(4.5, "segmentDeltaEta");
        m_muonTrks->addVariable<float>(std::numbers::pi, "segmentDeltaPhi");
        m_muonTrks->addVariable<float>(-1., "segmentChi2OverDoF");

        using TrkType = xAOD::Muon::TrackParticleType;
        auto dumpTrack = [&](const std::string& trkName, TrkType type) {
            auto trkColl = std::make_shared<IParticleFourMomBranch>(m_tree, trkName);
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
            BilateralLinkerBranch::connectCollections(m_muonTrks, trkColl, 
                    [type](const xAOD::IParticle* muonP) -> const xAOD::IParticle* {
                        const auto* muon = dynamic_cast<const xAOD::Muon*>(muonP);
                        if (!muon) {
                            return nullptr;
                        }
                        return muon->trackParticle(type);
                    }, trkName, "muon");

            return trkColl;
        };

        auto msTracks = dumpTrack("MsTrk", TrkType::MuonSpectrometerTrackParticle);
        dumpTrack("MeTrk", TrkType::ExtrapolatedMuonSpectrometerTrackParticle);
        m_idTracks = dumpTrack("IdTrk", TrkType::InnerDetectorTrackParticle);

        /** Dump whether the id track is selected and dump the parameters at calo exit */
        {
            auto& l0{m_tree.newVector<float>(std::format("{:}_caloExitLoc0", m_idTracks->name()))};
            auto& l1{m_tree.newVector<float>(std::format("{:}_caloExitLoc1", m_idTracks->name()))};
            auto& theta{m_tree.newVector<float>(std::format("{:}_caloExitTheta", m_idTracks->name()))};
            auto& phi{m_tree.newVector<float>(std::format("{:}_caloExitPhi", m_idTracks->name()))};
            auto& mom{m_tree.newVector<float>(std::format("{:}_caloExitMom", m_idTracks->name()))};
            
            m_idTracks->addVariable(std::make_unique<GenericPartDecorBranch<xAOD::TrackParticle, std::uint8_t>>(m_tree, 
                std::format("{:}_isCandidate", m_idTracks->name()), 
                [this, &l0, &l1, &theta, &phi, &mom] (const xAOD::TrackParticle& idTrack) -> std::uint8_t {
                    const MuonR4::MuonTag* tag = findBaseIdTag(idTrack, Gaudi::Hive::currentContext());
                    std::optional<Acts::BoundTrackParameters> trkPars = tag ? tag->extrapolatedParsID(Acts::hashString("@CaloExit"))
                                                                            : std::nullopt;
                    const std::size_t idx = l0.size();
                    using namespace MuonCombinedR4;
                    l0[idx] = trkPars ? longitudinalParam(*trkPars, logger()) : 1._km;
                    l1[idx] = trkPars ? toDeg(localPolarAngle(*trkPars, logger())) : 360.;
                    
                    theta[idx] = trkPars ? toDeg(trkPars->get<Acts::eBoundTheta>()) : 360.;
                    phi[idx] = trkPars ? toDeg(trkPars->get<Acts::eBoundPhi>()) : 360.;
                    mom[idx] = trkPars ? trkPars->absoluteMomentum() : -1._GeV;
                    return trkPars != std::nullopt;
                }));
        }
        {
            auto& dX0{m_tree.newMatrix<float>(std::format("{:}_segTagDx0", m_idTracks->name()))};
            auto& dY0{m_tree.newMatrix<float>(std::format("{:}_segTagDy0", m_idTracks->name()))};
            auto& dTheta{m_tree.newMatrix<float>(std::format("{:}_segTagDtheta", m_idTracks->name()))};
            auto& dPhi{m_tree.newMatrix<float>(std::format("{:}_segTagDphi", m_idTracks->name()))};
            auto& segments{m_tree.newMatrix<std::uint8_t>(std::format("{:}_segTagSegments", m_idTracks->name()))};
            auto& goodExtp{m_tree.newMatrix<std::uint8_t>(std::format("{:}_segTagExtpGood",m_idTracks->name()))};
            auto& taggedSeg{m_tree.newMatrix<std::uint8_t>(std::format("{:}_segTagOnTag", m_idTracks->name()))};
            m_idTracks->addVariable(
            std::make_unique<GenericPartDecorBranch<xAOD::TrackParticle, std::vector<float>>>(m_tree, 
                std::format("{:}_segTagScore", m_idTracks->name()), 
                    [&dX0, &dY0, &dTheta, &dPhi, &segments, 
                     &goodExtp, &taggedSeg,this] (const xAOD::TrackParticle& idTrack) -> std::vector<float> {
                    auto result = calcSegTagVariables(idTrack, Gaudi::Hive::currentContext());

                    segments.push_back(std::move(result.recoSegs));
                    dX0.push_back(std::move(result.deltaX0));
                    dY0.push_back(std::move(result.deltaY0));
                    dTheta.push_back(std::move(result.deltaTheta));
                    dPhi.push_back(std::move(result.deltaPhi));
                    goodExtp.push_back(std::move(result.goodExtp));
                    taggedSeg.push_back(std::move(result.taggedSeg));
                    return std::move(result.matchScores);
            }));
        }
        if (m_isMC) {
            BilateralLinkerBranch::connectCollections(m_idTracks, m_truthTrks, 
                [this](const xAOD::IParticle* trk) { 
                    return truthTreeParticle(*trk); 
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
        msTracks->addVariable(std::make_unique<GenericPartDecorBranch<xAOD::TrackParticle, unsigned short>>(m_tree, 
                    std::format("{:}_seedLink", msTracks->name()), [&] (const xAOD::TrackParticle& p) -> unsigned short {
                        auto actsTrk = ActsTrk::getActsTrack(p);
                        if (!actsTrk) {
                            THROW_EXCEPTION("Cannot find the associated ms track from the primary track");
                        }
                        return actsTrk->component<std::size_t>("parentSeed");
                }));

        msTracks->addVariable(std::make_unique<GenericPartDecorBranch<xAOD::TrackParticle, float>>(m_tree, 
                    std::format("{:}_loc0", msTracks->name()), [&] (const xAOD::TrackParticle& p){
                        auto actsTrk = ActsTrk::getActsTrack(p);
                        if (!actsTrk) {
                            THROW_EXCEPTION("Cannot find the associated ms track from the primary track");
                        }
                        auto refPars = actsTrk->createParametersAtReference();
                        return isOnCaloExit(refPars) ? MuonCombinedR4::longitudinalParam(refPars, logger())
                                                     : refPars.get<Acts::eBoundLoc0>();
                }));
        msTracks->addVariable(std::make_unique<GenericPartDecorBranch<xAOD::TrackParticle, float>>(m_tree, 
                    std::format("{:}_loc1", msTracks->name()), [&] (const xAOD::TrackParticle& p){
                        auto actsTrk = ActsTrk::getActsTrack(p);
                        if (!actsTrk) {
                            THROW_EXCEPTION("Cannot find the associated ms track from the primary track");
                        }
                        auto refPars = actsTrk->createParametersAtReference();
                        return isOnCaloExit(refPars) ? toDeg(MuonCombinedR4::localPolarAngle(refPars, logger()))
                                                     : refPars.get<Acts::eBoundLoc0>(); 
                }));
        msTracks->addVariable(std::make_unique<GenericPartDecorBranch<xAOD::TrackParticle, std::uint8_t>>(m_tree, 
                    std::format("{:}_isOnCaloExit", msTracks->name()), [&] (const xAOD::TrackParticle& p){
                        auto actsTrk = ActsTrk::getActsTrack(p);
                        if (!actsTrk) {
                            THROW_EXCEPTION("Cannot find the associated ms track from the primary track");
                        }
                        return isOnCaloExit(actsTrk->referenceSurface());
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

    const xAOD::TruthParticle* MsTrackTester::truthTreeParticle(const xAOD::IParticle& part) const {
        const xAOD::TruthParticle* truthTrk = xAOD::TruthHelpers::getTruthParticle(part);
        if (!truthTrk || !m_truthTrks) {
            return nullptr;
        }
        for (const xAOD::IParticle* thisPart : m_truthTrks->getCached()) {
            if (thisPart == truthTrk || truthTrk ==  xAOD::TruthHelpers::getTruthParticle(*thisPart)) {
                return dynamic_cast<const xAOD::TruthParticle*>(thisPart);
            }
        }
        return nullptr;
    }

    const MuonR4::MuonTag* MsTrackTester::findBaseIdTag(const xAOD::TrackParticle& idTrack,
                                                        const EventContext& ctx) const {
        return findTagMatchingToId(idTrack, m_idTagKey, ctx);
    }
    const MuonR4::MuonTag* MsTrackTester::findMuTagIMO(const xAOD::TrackParticle& idTrack,
                                                        const EventContext& ctx) const {
        return findTagMatchingToId(idTrack, m_segTagKey, ctx);
    }

    std::vector<const xAOD::MuonSegment*> 
        MsTrackTester::getAssociatedSegments(const xAOD::TrackParticle& idTrack,
                                             const EventContext& ctx) const {
        std::vector<const xAOD::MuonSegment*> compatibleSegs{};
        if (const xAOD::TruthParticle* truthPart = truthTreeParticle(idTrack); truthPart != nullptr) {
            std::vector<const xAOD::MuonSegment*> truthSegs = getTruthSegments(*truthPart);
            const xAOD::MuonSegmentContainer* recoSegs{nullptr};
            SG::get(recoSegs, m_recoSegmentKey, ctx).ignore();
            std::copy_if(recoSegs->begin(), recoSegs->end(), std::back_inserter(compatibleSegs), 
                        [&truthSegs](const xAOD::MuonSegment* recoSeg){
                             return Acts::rangeContainsValue(truthSegs, getMatchedTruthSegment(*recoSeg));
                        });
            if (!compatibleSegs.empty()) {
                return compatibleSegs;
            }
        }
        
        const xAOD::MuonContainer* muons{nullptr};
        SG::get(muons,m_muonKey, ctx).ignore();
        for (const xAOD::Muon* muon : *muons) {
            if (muon->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle) == &idTrack) {
                for (std::size_t s = 0; s < muon->nMuonSegments() ; ++s) {
                    compatibleSegs.push_back(muon->muonSegment(s));
                }
                break;
            }
        }
        return compatibleSegs;
    }

    const MuonGMR4::SpectrometerSector* MsTrackTester::getEnvelope(const xAOD::MuonSegment& segment) const {
        return m_detMgr->getSectorEnvelope(segment.chamberIndex(), segment.sector(), segment.etaIndex()); 
    }
        
    MsTrackTester::SegmentTagVariables 
        MsTrackTester::calcSegTagVariables(const xAOD::TrackParticle& idTrack,
                                           const EventContext& ctx)  {

        SegmentTagVariables result{};
        /// No associated segments found
        const std::vector<const xAOD::MuonSegment*> compatibleSegs = getAssociatedSegments(idTrack, ctx);
        if (compatibleSegs.empty()) {
            return result;
        } 
        /// Calo extension does not work
        const ActsTrk::CaloExtension* caloExt = ActsTrk::getCaloExtension(idTrack);
        if (!caloExt) {
            return result; 
        }
        std::optional<Acts::BoundTrackParameters> startPars = caloExt->lastParameters();
        if (!startPars) {
            return result;
        }

        const MuonR4::MuonTag* muTag = findMuTagIMO(idTrack, ctx);
    
        const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

        auto nextParameters = [&](const xAOD::MuonSegment& segment) {
            /// In case that there is already a 
            const MuonGMR4::SpectrometerSector* msSector = getEnvelope(segment);
            const Acts::Surface& target{msSector->surface()};
            if (target.geometryId() == startPars->referenceSurface().geometryId()) {
                return true;
            }
            if (muTag) {
                auto surfPars = muTag->extrapolatedParsID(Acts::toUnderlying(segment.chamberIndex()));
                if (surfPars) {
                    startPars = surfPars;
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Recycle cached parameters from muTag\n"
                                    <<(*startPars));
                    return true;
                }
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Extrapolation to "<<printID(segment)<<" failed somehow");
            }

            auto extpPars = m_extrapolationTool->propagate(ctx, *startPars, target);
            if (!extpPars.ok()) {
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Extrapolation keeps failing");
                return false;
            }
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Updated parameters to \n"<<(*extpPars));
            startPars = *extpPars;
            return true;
        };

        const xAOD::MuonSegment* bestSeg{nullptr};
        float bestMatchScore{std::numeric_limits<float>::max()};
        Acts::BoundVector bestPars{Acts::BoundVector::Zero()};

        auto dumpBestSeg = [&]() {
            if (!bestSeg) {
                return;
            }
            result.recoSegs.push_back(m_recoSegs->push_back(*bestSeg));
            result.matchScores.push_back(bestMatchScore);
            result.deltaPhi.push_back(toDeg(bestPars[Acts::eBoundPhi]));
            result.deltaTheta.push_back(toDeg(bestPars[Acts::eBoundTheta]));
            result.deltaY0.push_back(bestPars[Acts::eBoundLoc1]);
            result.deltaX0.push_back(bestPars[Acts::eBoundLoc0]);
            result.goodExtp.push_back(1);
            result.taggedSeg.push_back(muTag && Acts::rangeContainsValue(muTag->segments(), bestSeg));
            bestSeg = nullptr;
            bestMatchScore = std::numeric_limits<float>::max();
            bestPars = Acts::BoundVector::Zero();
        };

        for (auto segItr = compatibleSegs.begin(); segItr != compatibleSegs.end(); ) {
            const xAOD::MuonSegment* testMe{*segItr};
            /// Store in the tree that the segment cannot be associated due to extrapolation failure
            if (!nextParameters(*testMe)) {
                result.recoSegs.push_back(m_recoSegs->push_back(*testMe));
                result.matchScores.push_back(-1.f);
                result.deltaX0.push_back(1._m);
                result.deltaY0.push_back(1._m);
                result.deltaTheta.push_back(180.);
                result.deltaPhi.push_back(180.);
                result.goodExtp.push_back(0);
                result.taggedSeg.push_back(0);
                segItr = std::find_if(std::next(segItr),compatibleSegs.end(), [&](const xAOD::MuonSegment* assocSeg){
                    return getEnvelope(*testMe) != getEnvelope(*assocSeg);
                });
            } else {
                ++segItr;
            } 
            /// If there is a best segment dump the best segment to the tree
            if (bestSeg && getEnvelope(*testMe) != getEnvelope(*bestSeg)) {
                dumpBestSeg();
            }
            const Acts::BoundTrackParameters segmentPars{MuonR4::SegmentFit::boundSegmentPars(tgContext, *m_detMgr, *testMe)};
            Acts::BoundVector dPars = segmentPars.parameters() - startPars->parameters();
            /** The segment does not measure phi. Reset anything in loc0 and non-precision direction */
            if (!testMe->nPhiLayers()) {
                dPars[Acts::eBoundPhi] = dPars[Acts::eBoundLoc0] = 0.;
            } else {
                dPars[Acts::eBoundPhi] = P4Helpers::deltaPhi(dPars[Acts::eBoundPhi], 0.);
            }
            
            dPars[Acts::eBoundQOverP] = dPars[Acts::eBoundTime] = 0.;
            Acts::BoundMatrix covariance{Acts::BoundMatrix::Identity()};
            covariance(Acts::eBoundLoc0, Acts::eBoundLoc0) = Acts::square(m_toleranceX0.value());
            covariance(Acts::eBoundLoc1, Acts::eBoundLoc1) = Acts::square(m_toleranceY0.value());
            covariance(Acts::eBoundTheta, Acts::eBoundTheta) = Acts::square(m_toleranceTheta.value());
            covariance(Acts::eBoundPhi, Acts::eBoundPhi) = Acts::square(m_tolerancePhi.value());
            if (startPars->covariance()) {
                covariance += (*startPars->covariance());
            }
            if (segmentPars.covariance()) {
                covariance += (*segmentPars.covariance());
            }
            const float chi2 = dPars.dot(covariance.inverse()*dPars);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Difference: "<<Acts::toString(dPars)
                        <<", covariance: \n"<<Acts::toString(covariance)<<",\nchi2: "<<chi2);
            if (chi2 < bestMatchScore) {
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Best parameters on surface thus far");
                bestMatchScore = chi2;
                bestSeg = testMe;
                bestPars = std::move(dPars);
            }
        }
        dumpBestSeg();
        return result;
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

        const xAOD::TrackParticleContainer* idTracks{nullptr};
        ATH_CHECK(SG::get(idTracks, m_idTrackKey, ctx));

        const MuonR4::MuonTagContainer* idTags{nullptr};
        ATH_CHECK(SG::get(idTags, m_idTagKey, ctx));

        const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
        const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);

        MagField::AtlasFieldCache magField{};
        mfContext.get<const AtlasFieldCacheCondObj*>()->getInitializedCache(magField);

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
            const auto[seedLength, theta] = calcSeedLength(tgContext, seed);
            m_seedLength+= seedLength;
            m_seedThetaCone+=theta;
            m_seedQP += m_seedingTool->estimateQtimesP(tgContext, seed, magField) / Gaudi::Units::GeV; 
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

        /// Collect only the ID tracks that belong to a truth muon of interest.
        if (idTracks) {
            m_nIdTracks = idTracks->size();
            for (const xAOD::TrackParticle* idTrk : *idTracks) {
                if (m_truthTrks->find([idTrk](const xAOD::IParticle* p){
                    return xAOD::TruthHelpers::getTruthParticle(*idTrk) == 
                          xAOD::TruthHelpers::getTruthParticle(*p);
                }) < m_truthTrks->size()) {
                    m_idTracks->push_back(idTrk);
                }
            }
        }
        if (idTags) {
            m_nIdTags = idTags->size();
            for (const MuonR4::MuonTag* idTag : *idTags) {
                /// Select only the id tags that are extrapolated to the calo exit
                if (idTag->extrapolatedParsID(Acts::hashString("@CaloExit"))){
                    m_idTracks->push_back(idTag->idTrack());
                }
            }
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
