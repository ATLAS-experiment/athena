/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughTransformTester.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonTesterTree/EventInfoBranch.h"

#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "Acts/Utilities/Enumerate.hpp"
#include "GaudiKernel/PhysicalConstants.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"

 #include "AthContainers/ConstDataVector.h"
 #include "xAODTruth/xAODTruthHelpers.h"

 #include "Acts/Utilities/AlgebraHelpers.hpp"
 #include "Acts/Definitions/Units.hpp"

namespace {
    constexpr double c_inv = 1. /Gaudi::Units::c_light;

bool isPrecision(const MuonR4::SpacePoint& hit) {
    using enum xAOD::UncalibMeasType;
    return hit.type() == MdtDriftCircleType || hit.type() == MMClusterType ||
            (hit.type() == sTgcStripType && 
            static_cast<const xAOD::sTgcMeasurement*>(hit.primaryMeasurement())->channelType() ==
            sTgcIdHelper::sTgcChannelTypes::Strip);
}
bool isPrecision(const MuonR4::CalibratedSpacePoint& hit) {
    using enum xAOD::UncalibMeasType;
    if (hit.type() == xAOD::UncalibMeasType::Other){
        return false;
    }
    return isPrecision(*hit.spacePoint());
}

}


namespace MuonValR4 {
    using namespace MuonR4;
    using namespace MuonVal;
    using namespace Muon::MuonStationIndex;
    using namespace Acts::UnitLiterals;


    using ObjectMatching = MuonHoughTransformTester::ObjectMatching;
 
    StatusCode MuonHoughTransformTester::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        
        {
            int infoOpts = 0;
            if (m_isMC) infoOpts = EventInfoBranch::isMC;
            m_tree.addBranch(std::make_unique<EventInfoBranch>(m_tree, infoOpts));  
        }

        ATH_CHECK(m_recoSegKey.initialize());
        for (const std::string& recoLink : m_recoSegLinks) {
            m_truthSegLinkKeys.emplace_back(m_recoSegKey, recoLink);
        }
        ATH_CHECK(m_truthSegmentKey.initialize(!m_truthSegmentKey.empty())); 
        for (const std::string& link: m_truthLinks) {
            m_truthSegLinkKeys.emplace_back(m_truthSegmentKey, link);
        }
        ATH_CHECK(m_truthSegLinkKeys.initialize(!m_truthSegmentKey.empty()));

        /// The collection of readHandle keys should be either 1 or 2
        ATH_CHECK(m_patternSeedKeys.initialize());
        ATH_CHECK(m_spKeys.initialize(m_writeSpacePoints));
        if (m_writeSpacePoints) {
            m_spTester = std::make_unique<SpacePointTesterModule>(m_tree, m_spKeys.front().key(), msgLevel());
            m_tree.addBranch(m_spTester);
        } else {
            m_tree.disableBranch(m_spMatchedToPattern.name());
            m_tree.disableBranch(m_spMatchedToSegment.name());
        }

        for (std::size_t cov = 0 ; cov < m_segmentCov.size(); ++cov){
            using namespace MuonR4::SegmentFit;
            const auto [i, j] = Acts::symMatIndices<Acts::toUnderlying(ParamDefs::nPars)>(cov);
            ParamDefs pI{static_cast<std::uint8_t>(i)},
                      pJ{static_cast<std::uint8_t>(j)};
            std::string brName = (pI!= pJ) ? std::format("segment_cov_{:}_{:}", pI, pJ)
                                         : std::format("segment_cov_{:}", pI);
            m_segmentCov[cov] = std::make_shared<MuonVal::VectorBranch<float>>(m_tree, brName);
            m_tree.addBranch(m_segmentCov[cov]);
        }

        ATH_CHECK(m_tree.init(this)); 
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));

        ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));
        
        return StatusCode::SUCCESS;
    }


    unsigned int MuonHoughTransformTester::countOnSameSide(const xAOD::MuonSegment& truthSeg,
                                                           const xAOD::MuonSegment& recoSeg) const{
        unsigned int same{0};
        using namespace SegmentFit;
        const MuonR4::Segment* detailSeg = detailedSegment(recoSeg);
        const auto[truePos, trueDir] = makeLine(localSegmentPars(truthSeg)); 
        const auto[recoPos, recoDir] = makeLine(localSegmentPars(recoSeg));
        const std::vector<int> truthSigns = SeedingAux::strawSigns(truePos, trueDir, detailSeg->measurements());
        const std::vector<int> recoSigns = SeedingAux::strawSigns(recoPos, recoDir, detailSeg->measurements());
        for (unsigned int s = 0 ; s < truthSigns.size(); ++s) {
            same += (truthSigns[s] != 0) && truthSigns[s] == recoSigns[s];
        }
        return same;
    }
    std::vector<ObjectMatching> 
            MuonHoughTransformTester::matchWithTruth(const MuonR4::SegmentSeedContainer& seedContainer,
                                                     const xAOD::MuonSegmentContainer& segmentContainer,
                                                     const xAOD::MuonSegmentContainer* truthSegments) const {
        std::vector<ObjectMatching> allAssociations{};
        std::unordered_set<const SegmentSeed*> usedSeeds{};
        /// Step 1 Loop over all reconstructed segments && fill them
        /// into the association table
        for (const xAOD::MuonSegment* recoSeg : segmentContainer) {
            const MuonR4::Segment* segment = detailedSegment(*recoSeg);
            assert(segment != nullptr);
            std::vector<ObjectMatching>::iterator assoc_itr = allAssociations.end();
            const xAOD::MuonSegment* truthSeg = getMatchedTruthSegment(*recoSeg);
            /** There is an associated truth egment */
            if (truthSeg) {
                assoc_itr = std::ranges::find_if(allAssociations, [truthSeg](const ObjectMatching& obj){
                    return obj.truthSegment == truthSeg;
                });
            }
            /** Thus far no entry in the association map */
            if (assoc_itr == allAssociations.end()) {
                ObjectMatching& newObj = allAssociations.emplace_back();
                newObj.chamber = m_detMgr->getSectorEnvelope(recoSeg->chamberIndex(),
                                                             recoSeg->sector(),
                                                             recoSeg->etaIndex());
                newObj.truthSegment = truthSeg;
                assoc_itr = allAssociations.end() -1;
            }
            ObjectMatching& assocObj{*assoc_itr};
            assocObj.matchedSegments.push_back(recoSeg);
            assocObj.matchedSeeds.push_back(segment->parent());
            assocObj.matchedSeedFoundSegment.push_back(1);
            usedSeeds.insert(segment->parent());
        }
        
        if (truthSegments) {
            /** Sort the segments according to the drift signs */
            for (ObjectMatching& assocObj : allAssociations) {
                if (!assocObj.truthSegment) {
                    continue;
                }
                std::ranges::sort(assocObj.matchedSegments, 
                                [&](const xAOD::MuonSegment* a, 
                                    const xAOD::MuonSegment* b){
                                    return countOnSameSide(*assocObj.truthSegment, *a) >
                                           countOnSameSide(*assocObj.truthSegment, *b);
                                });
            }
        }
        /** Loop over all remaining seeds and attempt to match them to a segment */
        for (const SegmentSeed* seed : seedContainer) {
            /// Don't recycle the  used seeds again
            if (usedSeeds.count(seed)) {
                continue;
            }
            /** Find the segment that matches best to the seed */
            std::vector<std::pair<const xAOD::MuonSegment*, std::size_t>> segCounts{};
            std::unordered_set<const xAOD::MuonSimHit* > matchedHits = getMatchingSimHits(*seed);
            for (const xAOD::MuonSimHit* hit : matchedHits) {
                const xAOD::MuonSegment* truthSeg = getMatchedTruthSegment(*hit);
                if (!truthSeg) {
                    continue;
                }
                auto count_itr = std::ranges::find_if(segCounts, [truthSeg](const auto& segCounter){
                    return segCounter.first == truthSeg;
                });
                if (count_itr != segCounts.end()) {
                    ++(count_itr->second);
                } else {
                    segCounts.emplace_back(std::make_pair(truthSeg, 1ul));
                }
            }
            /// find the best matching segment
            std::ranges::sort(segCounts, [](const auto& a, const auto& b){
                return a.second > b.second;
            });
            // Add a criterion on the number of counts?
            const xAOD::MuonSegment* truthSeg = segCounts.size() 
                                              ? segCounts.front().first : nullptr;
            if (truthSeg) {
                auto assoc_itr = std::ranges::find_if(allAssociations, 
                    [truthSeg](const ObjectMatching& obj){
                        return obj.truthSegment == truthSeg;
                    });
                if (assoc_itr == allAssociations.end()) {
                    ObjectMatching & newObj = allAssociations.emplace_back();
                    newObj.chamber = seed->msSector();
                    newObj.truthSegment = truthSeg;
                    assoc_itr = allAssociations.end() -1;
                }
                assoc_itr->matchedSeeds.push_back(seed);
                assoc_itr->matchedSeedFoundSegment.push_back(0);
            } else {
                ObjectMatching & newObj = allAssociations.emplace_back();
                newObj.chamber = seed->msSector(); 
                newObj.matchedSeeds.push_back(seed);
                newObj.matchedSeedFoundSegment.push_back(0);
            }
        }
        /// Finally loop over all truth segments and push back the unused ones
        if (truthSegments) {
            for (const xAOD::MuonSegment* truthSeg :  *truthSegments) {
                /// Check that the segment is not used yet
                if (std::ranges::any_of(allAssociations, [truthSeg](const auto& assocObj){
                    return assocObj.truthSegment == truthSeg;
                })) {
                    continue;
                }
                ObjectMatching & newObj = allAssociations.emplace_back();
                newObj.chamber = m_detMgr->getSectorEnvelope(truthSeg->chamberIndex(),
                                                             truthSeg->sector(),
                                                             truthSeg->etaIndex());
                newObj.truthSegment = truthSeg;
            }
        }
        return allAssociations;
    }

    StatusCode MuonHoughTransformTester::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    StatusCode MuonHoughTransformTester::execute(const EventContext& ctx) {
        
        const ActsTrk::GeometryContext* gctxPtr{nullptr};
        ATH_CHECK(SG::get(gctxPtr, m_geoCtxKey, ctx));
        const ActsTrk::GeometryContext& gctx{*gctxPtr};

        ConstDataVector<MuonR4::SegmentSeedContainer> segmentSeeds{SG::VIEW_ELEMENTS};
        for (const SG::ReadHandleKey<SegmentSeedContainer>& key : m_patternSeedKeys) {
            const SegmentSeedContainer* readSegmentSeeds{nullptr};
            ATH_CHECK(SG::get(readSegmentSeeds, key, ctx));
            segmentSeeds.insert(segmentSeeds.end(), readSegmentSeeds->begin(), readSegmentSeeds->end());
        }

        const xAOD::MuonSegmentContainer* truthSegments{nullptr};
        ATH_CHECK(SG::get(truthSegments, m_truthSegmentKey, ctx));

        const xAOD::MuonSegmentContainer* recoSegments{nullptr};
        ATH_CHECK(SG::get(recoSegments, m_recoSegKey, ctx));

        ATH_MSG_DEBUG("Succesfully retrieved input collections. Seeds: "<<segmentSeeds.size()
                    <<", segments: "<<recoSegments->size() 
                    <<", truth segments: "<<(truthSegments? truthSegments->size() : -1)
                    <<".");
        std::vector<ObjectMatching> objects = matchWithTruth(*segmentSeeds.asDataVector(), 
                                                             *recoSegments, truthSegments);
        for (const ObjectMatching& obj : objects) {
            fillChamberInfo(obj.chamber);
            fillSeedInfo(obj);
            fillSegmentInfo(obj);
            if(m_isMC) fillTruthInfo(gctx, obj.truthSegment);
            ATH_CHECK(m_tree.fill(ctx));
        }
        return StatusCode::SUCCESS;
    }
    void MuonHoughTransformTester::fillChamberInfo(const MuonGMR4::SpectrometerSector* msSector){
        m_out_chamberIndex = Acts::toUnderlying(msSector->chamberIndex());
        m_out_stationSide = msSector->side();
        m_out_stationPhi = msSector->stationPhi();
    }                
    void MuonHoughTransformTester:: fillTruthInfo(const ActsTrk::GeometryContext& gctx,
                                                  const xAOD::MuonSegment* segment) {
        if (!segment) {
            return; 
        }
        const xAOD::TruthParticle* truthMuon = getTruthMatchedParticle(*segment);
        m_out_hasTruth = true; 

        const Amg::Vector3D segDir{segment->direction()};
        static const SG::ConstAccessor<float> acc_pt{"pt"};
        static const SG::ConstAccessor<float> acc_charge{"charge"};
        // eta is interpreted as the eta-location 
        m_out_gen_Q = acc_charge(*segment);
       if (truthMuon) {
            m_out_gen_Pt  = truthMuon->pt();
            m_out_gen_Eta = truthMuon->eta();
            m_out_gen_Phi = truthMuon->phi();
            m_out_gen_truthPdgId = truthMuon->pdgId();
            m_out_gen_truthBeta = truthMuon->p4().Beta();
        } else {
            m_out_gen_Pt  = acc_pt(*segment);
            m_out_gen_Eta = segDir.eta();
            m_out_gen_Phi = segDir.phi();
        }
        m_out_gen_deflection = Amg::angle(segDir, segment->position());
        m_out_gen_deflectionEta = segDir.eta() - segment->position().eta();
      
        const auto [chamberPos, chamberDir] = SegmentFit::makeLine(SegmentFit::localSegmentPars(*segment));
        
        ATH_MSG_DEBUG("Number of precision Hits in the truth segment is "<<segment->nPrecisionHits()<<" and number of phi layers is "
                    <<segment->nPhiLayers()<<" and number of trigger eta layers is "<<segment->nTrigEtaLayers());
        m_out_gen_nPrecHits = segment->nPrecisionHits();
        m_out_gen_nTrigEtaHits = segment->nTrigEtaLayers();
        m_out_gen_nTrigPhiHits = segment->nPhiLayers();
        using namespace Acts::detail::LineHelper;
        const Acts::Intersection3D bsExtp = lineIntersect<3>(Amg::Vector3D::Zero(),
                                                             Amg::Vector3D::UnitZ(),
                                                             segment->position(), 
                                                             segment->direction());
        const Amg::Vector3D closePoint = bsExtp.position();
 
        m_out_gen_beamSpotR = closePoint.perp();
        m_out_gen_beamSpotZ = std::abs(closePoint.z());
        unsigned nMmEtaHits{0}, nMmStereoHits{0}, nStgcHits{0};
        for (const xAOD::MuonSimHit* simHit : getMatchingSimHits(*segment)) {
            if (!m_out_gen_truthBeta.isUpdated()) {
                m_out_gen_truthBeta = simHit->beta();
                m_out_gen_truthPdgId = simHit->pdgId();
            }
            nStgcHits += m_idHelperSvc->technologyIndex(simHit->identify()) == TechnologyIndex::STGC;
            if(m_idHelperSvc->technologyIndex(simHit->identify()) != TechnologyIndex::MM) {
                continue;
            }
            const bool isStereo = m_idHelperSvc->mmIdHelper().isStereo(simHit->identify());
            nMmEtaHits += (!isStereo);
            nMmStereoHits += isStereo;
        }
        m_out_gen_nMmEtaHits = nMmEtaHits;
        m_out_gen_nMmStereoHits = nMmStereoHits;
        m_out_gen_nStgcHits = nStgcHits;

        m_out_gen_tanbeta = houghTanBeta(chamberDir); 
        m_out_gen_tanalpha = houghTanAlpha(chamberDir);
        m_out_gen_y0 = chamberPos.y(); 
        m_out_gen_x0 = chamberPos.x(); 
        m_out_gen_time = segment->t0();

        double minYhit = std::numeric_limits<double>::max();
        double maxYhit = -1 * std::numeric_limits<double>::max();
        for (const xAOD::MuonSimHit* hit : getMatchingSimHits(*segment)){
            const Identifier hitId = hit->identify();
            const MuonGMR4::MuonReadoutElement* RE = m_detMgr->getReadoutElement(hitId); 
            const IdentifierHash hash{m_idHelperSvc->isMdt(hitId) ? RE->measurementHash(hitId)
                                                                  : RE->layerHash(hitId) };
            const Acts::Transform3 localToChamber = RE->msSector()->globalToLocalTransform(gctx) * RE->localToGlobalTransform(gctx, hash);
            const Amg::Vector3D chamberPos = localToChamber * xAOD::toEigen(hit->localPosition()); 
            minYhit = std::min(chamberPos.y(), minYhit); 
            maxYhit = std::max(chamberPos.y(), maxYhit); 
        }
        m_out_gen_minYhit = minYhit;
        m_out_gen_maxYhit = maxYhit;

       
        if (truthMuon) {
            using namespace xAOD::TruthHelpers;
            m_out_gen_truthType   = getParticleTruthType(*truthMuon);
            m_out_gen_truthOrigin = getParticleTruthOrigin(*truthMuon);
        }
    }
    void MuonHoughTransformTester::fillBucketInfo(const SpacePointBucket& bucket) {
        m_out_bucketEnd = bucket.coveredMax();
        m_out_bucketStart = bucket.coveredMin();
        m_out_nSpacePoints = bucket.size();

        ATH_MSG_DEBUG("Filling bucket with "<<bucket.size()<<" space points between "<< bucket.coveredMin() <<" and "<< bucket.coveredMax() ); 

        // Because the space points are first ordered by layers and then local y within the layer we need to resort them to compute the max gap in y between consecutive hits in the bucket
        std::vector<const MuonR4::SpacePoint*> etaHits{};
        etaHits.reserve(bucket.size());
        for (const auto& sp : bucket) {
            if (sp->measuresEta()) {
                etaHits.push_back(sp.get());
            }
        }
        std::ranges::sort(etaHits, [](const MuonR4::SpacePoint* a,
                                      const MuonR4::SpacePoint* b) {
            return a->localPosition().y() < b->localPosition().y();
        });


        double maxEtaHitGap{-1.f};
        for (std::size_t i = 1; i < etaHits.size(); ++i) {
            maxEtaHitGap = std::max(maxEtaHitGap,
                                 std::abs(etaHits[i]->localPosition().y()
                                        - etaHits[i - 1]->localPosition().y()));
        }

        m_out_bucketEtaHitGap = maxEtaHitGap;


        m_out_nPrecSpacePoints = std::ranges::count_if(bucket, [](const SpacePointBucket::value_type& sp){
            return isPrecision(*sp);
        });
        m_out_nPhiSpacePoints = std::ranges::count_if(bucket, [](const SpacePointBucket::value_type& sp){
            return sp->measuresPhi();
        });
        if (!m_visionTool.isEnabled()){
            return;
        }
        m_out_nTrueSpacePoints = std::ranges::count_if(bucket,[this](const SpacePointBucket::value_type& sp){
            return m_visionTool->isLabeled(*sp);
        });
        m_out_nTruePrecSpacePoints = std::ranges::count_if(bucket,[this](const SpacePointBucket::value_type& sp){
            return isPrecision(*sp) && m_visionTool->isLabeled(*sp);
        });
        m_out_nTruePhiSpacePoints = std::ranges::count_if(bucket,[this](const SpacePointBucket::value_type& sp){
            return sp->measuresPhi() && m_visionTool->isLabeled(*sp);
        });
    }
    void MuonHoughTransformTester::fillSeedInfo(const ObjectMatching& obj) {

        m_out_seed_n = obj.matchedSeeds.size();
        for (const auto [iseed, seed] : Acts::enumerate(obj.matchedSeeds)){
            if (iseed ==0) {
                fillBucketInfo(*seed->parentBucket());
            }
            double minYhit = m_out_bucketEnd.getVariable();
            double maxYhit = m_out_bucketStart.getVariable();
            for (const SpacePoint* hit : seed->getHitsInMax()){
                minYhit = std::min(hit->localPosition().y(),minYhit); 
                maxYhit = std::max(hit->localPosition().y(),maxYhit); 
            }
            m_out_seed_minYhit.push_back(minYhit);
            m_out_seed_maxYhit.push_back(maxYhit);

            m_out_seed_hasPhiExtension.push_back(seed->hasPhiExtension()); 
            m_out_seed_y0.push_back(seed->interceptY());
            m_out_seed_tanbeta.push_back(seed->tanBeta());
            if (seed->hasPhiExtension()){
                m_out_seed_x0.push_back(seed->interceptX());
                m_out_seed_tanalpha.push_back(seed->tanAlpha());
            } else{
                m_out_seed_x0.push_back(-999);
                m_out_seed_tanalpha.push_back(-999);
            }
            m_out_seed_ledToSegment.push_back(obj.matchedSeedFoundSegment.at(iseed));


            /** Helper lambda to shorten a bit the syntax to count the hits
             *  of a certain kind  */
            auto hitCounter = [seed](auto lambda) {
                return std::count_if(seed->getHitsInMax().begin(),
                                     seed->getHitsInMax().end(), lambda);
            };

            m_out_seed_nPrecHits += hitCounter([](const SpacePoint* sp) {
                                        return isPrecision(*sp);
                                    });
            m_out_seed_nEtaHits += hitCounter([](const SpacePoint* sp) {
                                        return !isPrecision(*sp) && sp->measuresEta();
                                    });
            m_out_seed_nPhiHits += hitCounter([](const SpacePoint* sp) {
                                        return sp->measuresPhi();
                                    });

            m_out_seed_nTruePrecHits += hitCounter([this](const SpacePoint* sp) {
                                        return isPrecision(*sp) &&
                                              (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*sp));
                                    });
            m_out_seed_nTrueEtaHits += hitCounter([this](const SpacePoint* sp) {
                                        return !isPrecision(*sp) && sp->measuresEta() &&
                                              (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*sp));
                                    });
            m_out_seed_nTruePhiHits += hitCounter([this](const SpacePoint* sp) {
                                        return sp->measuresPhi() &&
                                              (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*sp));
                                    });

            /***
             *          Split hit counts for the NSW!
             */
            m_out_seed_nMmEtaHits += hitCounter([this](const SpacePoint* sp){
                return sp->type() == xAOD::UncalibMeasType::MMClusterType &&
                       !m_idHelperSvc->mmIdHelper().isStereo(sp->identify());
            });
            m_out_seed_nMmStereoHits += hitCounter([this](const SpacePoint* sp){
                return sp->type() == xAOD::UncalibMeasType::MMClusterType &&
                       m_idHelperSvc->mmIdHelper().isStereo(sp->identify());
            });

            m_out_seed_nsTgcStripHits+=hitCounter([](const SpacePoint* sp){
                return sp->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       isPrecision(*sp);
            });
            m_out_seed_nsTgcWireHits+=hitCounter([this](const SpacePoint* sp) {
                return sp->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       sp->measuresPhi() &&
                       m_idHelperSvc->stgcIdHelper().channelType(sp->identify()) !=
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            });
            m_out_seed_nsTgcPadHits.push_back(hitCounter([this](const SpacePoint* sp){
                return sp->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       m_idHelperSvc->stgcIdHelper().channelType(sp->identify()) ==
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));

            /** True hit count */
            m_out_seed_nTrueMmEtaHits += hitCounter([this](const SpacePoint* sp){
                return sp->type() == xAOD::UncalibMeasType::MMClusterType &&
                        (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*sp))  &&
                       !m_idHelperSvc->mmIdHelper().isStereo(sp->identify());
            });
            m_out_seed_nTrueMmStereoHits += hitCounter([this](const SpacePoint* sp){
                return sp->type() == xAOD::UncalibMeasType::MMClusterType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*sp)) &&
                       m_idHelperSvc->mmIdHelper().isStereo(sp->identify());
            });

            m_out_seed_nTruesTgcStripHits+=hitCounter([this](const SpacePoint* sp){
                return sp->type() == xAOD::UncalibMeasType::sTgcStripType &&
                        (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*sp)) &&
                       isPrecision(*sp);
            });
            m_out_seed_nTruesTgcWireHits+=hitCounter([this](const SpacePoint* sp) {
                return sp->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*sp)) &&
                       sp->measuresPhi() &&
                       m_idHelperSvc->stgcIdHelper().channelType(sp->identify()) !=
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            });
            m_out_seed_nTruesTgcPadHits.push_back(hitCounter([this](const SpacePoint* sp){
                return sp->type() == xAOD::UncalibMeasType::sTgcStripType &&
                        (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*sp)) &&
                       m_idHelperSvc->stgcIdHelper().channelType(sp->identify()) ==
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));

            if (m_writeSpacePoints) {
                std::vector<unsigned char> treeIdxs{};
                for (const HoughHitType & houghSP: seed->getHitsInMax()){                
                    if (m_writeSpacePoints){
                        unsigned treeIdx = m_spTester->push_back(*houghSP);
                        treeIdxs.push_back(treeIdx);
                    }
                }
                m_spMatchedToPattern[iseed] = std::move(treeIdxs);
            } 
        }
    }
    
    void MuonHoughTransformTester::fillSegmentInfo(const ObjectMatching& obj){
        using namespace SegmentFit;

        m_out_segment_n = obj.matchedSegments.size(); 
        for (const xAOD::MuonSegment* segment : obj.matchedSegments) {

            /** Parameters and covariance  */
            const auto pars = localSegmentPars(*segment);
            const auto cov = localSegmentCov(*segment);
 
            m_out_segment_theta.push_back(pars[Acts::toUnderlying(ParamDefs::theta)]);
            m_out_segment_phi.push_back(pars[Acts::toUnderlying(ParamDefs::phi)]);
            m_out_segment_y0.push_back(pars[Acts::toUnderlying(ParamDefs::y0)]);
            m_out_segment_x0.push_back(pars[Acts::toUnderlying(ParamDefs::x0)]);
            m_out_segment_time.push_back(pars[Acts::toUnderlying(ParamDefs::t0)] + 
                                         segment->position().mag() * c_inv);

            for (std::size_t i =0; i < Acts::toUnderlying(ParamDefs::nPars); ++i) {
                for (std::size_t j = 0; j <=i; ++j) {
                    const std::size_t vecIdx = Acts::vecIdxFromSymMat<Acts::toUnderlying(ParamDefs::nPars)>(i,j);
                    m_segmentCov[vecIdx]->push_back((*cov)(i,j));
                }
            }

            /** Fit quality */
            const MuonR4::Segment* detailSeg = detailedSegment(*segment);
            m_out_segment_fitIter.push_back(detailSeg->nFitIterations());
            m_out_segment_chi2.push_back(segment->chiSquared());
            m_out_segment_nDoF.push_back(segment->numberDoF());
            m_out_segment_hasTimeFit.push_back(detailSeg->hasTimeFit());
            /** Helper lambda to shorten a bit the syntax to count the hits
             *  of a certain kind  */
            auto hitCounter = [detailSeg](auto lambda) {
                return std::count_if(detailSeg->measurements().begin(),
                                     detailSeg->measurements().end(), lambda);
            };

            m_out_segment_nPrecHits +=  segment->nPrecisionHits();
            m_out_segment_nTrigEtaHits += segment->nTrigEtaLayers();
            m_out_segment_nTrigPhiHits += segment->nPhiLayers();
            
            m_out_segment_nPrecOutliers += segment->nPrecisionOutliers();
            m_out_segment_nTrigEtaOutliers += segment->nTriggerEtaOutliers();
            m_out_segment_nTrigPhiOutliers += segment->nTriggerPhiOutliers();
        
            m_out_segment_nPrecHoles += segment->nPrecisionHoles();
            m_out_segment_nTrigEtaHoles += segment->nTriggerEtaHoles();
            m_out_segment_nTrigPhiHoles += segment->nTriggerPhiHoles();
            /** True matched precision hit */
            m_out_segment_nTruePrecHits += hitCounter([this](const auto& meas) {
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       isPrecision(*meas) && (!m_visionTool.isEnabled() || 
                       m_visionTool->isLabeled(*meas->spacePoint()));
            });
            /** True matched trigger eta hit */
            m_out_segment_nTrueTrigEtaHits += hitCounter([this](const auto& meas) {
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       !isPrecision(*meas) && meas->measuresEta() && 
                       meas->spacePoint() &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint()));
            });
            /** True matched trigger phi hit */
            m_out_segment_nTrueTrigPhiHits += hitCounter([this](const auto& meas) {
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       !isPrecision(*meas) && meas->measuresPhi() && 
                       meas->spacePoint() &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint()));
            });
            /** True matched precision outlier */
            m_out_segment_nTruePrecOutliers += hitCounter([this](const auto& meas) {
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       isPrecision(*meas) && (!m_visionTool.isEnabled() || 
                       m_visionTool->isLabeled(*meas->spacePoint()));
            });
            /** True matched trigger eta outlier */
            m_out_segment_nTrueTrigEtaOutliers += hitCounter([this](const auto& meas) {
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       !isPrecision(*meas) && meas->measuresEta() && 
                       meas->spacePoint() &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint()));
            });
            /** True matched trigger phi outlier */
            m_out_segment_nTrueTrigPhiOutliers += hitCounter([this](const auto& meas) {
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       !isPrecision(*meas) && meas->measuresPhi() && 
                       meas->spacePoint() &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint()));
            });

            /***
             *          Split hit counts for the NSW!
             */
            m_out_segment_nMmEtaHits.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::MMClusterType &&
                       !m_idHelperSvc->mmIdHelper().isStereo(meas->spacePoint()->identify());
            }));
            m_out_segment_nMmStereoHits.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::MMClusterType &&
                       m_idHelperSvc->mmIdHelper().isStereo(meas->spacePoint()->identify());
            }));
            m_out_segment_nMmEtaOutliers.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::MMClusterType &&
                       !m_idHelperSvc->mmIdHelper().isStereo(meas->spacePoint()->identify());
            }));
            m_out_segment_nMmStereoOutliers.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::MMClusterType &&
                       m_idHelperSvc->mmIdHelper().isStereo(meas->spacePoint()->identify());
            }));

            m_out_segment_nSTgcStripHits.push_back(hitCounter([](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       isPrecision(*meas);
            }));
            m_out_segment_nSTgcWireHits.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       meas->measuresPhi() &&
                       m_idHelperSvc->stgcIdHelper().channelType(meas->spacePoint()->identify()) !=
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));
            m_out_segment_nSTgcPadHits.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       m_idHelperSvc->stgcIdHelper().channelType(meas->spacePoint()->identify()) ==
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));
            m_out_segment_nSTgcStripOutliers.push_back(hitCounter([](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       isPrecision(*meas);
            }));
            m_out_segment_nSTgcWireOutliers.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       meas->measuresPhi() &&
                       m_idHelperSvc->stgcIdHelper().channelType(meas->spacePoint()->identify()) !=
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));
            m_out_segment_nSTgcPadOutliers.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       m_idHelperSvc->stgcIdHelper().channelType(meas->spacePoint()->identify()) ==
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));
            /**        True NSW hit counts  */
            m_out_segment_nMmTrueEtaHits.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::MMClusterType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       !m_idHelperSvc->mmIdHelper().isStereo(meas->spacePoint()->identify());
            }));
            m_out_segment_nMmTrueStereoHits.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::MMClusterType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       m_idHelperSvc->mmIdHelper().isStereo(meas->spacePoint()->identify());
            }));
            m_out_segment_nMmTrueEtaOutliers.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::MMClusterType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       !m_idHelperSvc->mmIdHelper().isStereo(meas->spacePoint()->identify());
            }));
            m_out_segment_nMmTrueStereoOutliers.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::MMClusterType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       m_idHelperSvc->mmIdHelper().isStereo(meas->spacePoint()->identify());
            }));
            m_out_segment_nSTgcTrueStripHits.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       isPrecision(*meas);
            }));
            m_out_segment_nSTgcTrueWireHits.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       meas->measuresPhi() &&
                       m_idHelperSvc->stgcIdHelper().channelType(meas->spacePoint()->identify()) !=
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));
            m_out_segment_nSTgcTruePadHits.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() == CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       m_idHelperSvc->stgcIdHelper().channelType(meas->spacePoint()->identify()) ==
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));
            m_out_segment_nSTgcTrueStripOutliers.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       isPrecision(*meas);
            }));
            m_out_segment_nSTgcTrueWireOutliers.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       meas->measuresPhi() &&
                       m_idHelperSvc->stgcIdHelper().channelType(meas->spacePoint()->identify()) !=
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));
            m_out_segment_nSTgcTruePadOutliers.push_back(hitCounter([this](const auto& meas){
                return meas->fitState() != CalibratedSpacePoint::State::Valid &&
                       meas->type() == xAOD::UncalibMeasType::sTgcStripType &&
                       (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) &&
                       m_idHelperSvc->stgcIdHelper().channelType(meas->spacePoint()->identify()) ==
                       sTgcIdHelper::sTgcChannelTypes::Pad;
            }));

            double minYhit = 1._km;
            double maxYhit = -1._km;
            double minYTruehit = 1._km;
            double maxYTruehit = -1._km;
  
            std::vector<unsigned char> matched{};
            for (const auto & meas : detailSeg->measurements()){
                // skip dummy measurement from beam spot constraint
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" Dump "<<(*meas));
                if (meas->type() == xAOD::UncalibMeasType::Other) {
                    continue;
                }
                minYhit = std::min(meas->localPosition().y(),minYhit); 
                maxYhit = std::max(meas->localPosition().y(),maxYhit);
                if (!m_visionTool.isEnabled() || m_visionTool->isLabeled(*meas->spacePoint())) {
                    minYTruehit = std::min(meas->localPosition().y(), minYTruehit); 
                    maxYTruehit = std::max(meas->localPosition().y(), maxYTruehit);
                }
                if (m_writeSpacePoints) {
                    unsigned treeIdx = m_spTester->push_back(*meas->spacePoint());
                    if (treeIdx >= matched.size()){
                        matched.resize(treeIdx +1);
                    }
                    matched[treeIdx] = true;
                }
            }
   
            m_out_segment_minYhit += minYhit;
            m_out_segment_maxYhit += maxYhit;
            m_out_segment_minTrueYhit += minYTruehit;
            m_out_segment_maxTrueYhit += maxYTruehit;
            if (m_writeSpacePoints) {
                m_spMatchedToSegment.push_back(std::move(matched));
            }
        }
    }
}  // namespace MuonValR4
