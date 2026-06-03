/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "MuonSpacePoint/SpacePointHelpers.h"

 #include "AthContainers/ConstDataVector.h"
 #include "xAODTruth/xAODTruthHelpers.h"


namespace {
    constexpr double c_inv = 1. /Gaudi::Units::c_light;

    template <typename SegObj>

    unsigned countMatched(const xAOD::MuonSegment* truthSeg,
                          const SegObj& obj) {
        return truthSeg != nullptr ?
             std::ranges::count_if(getMatchingSimHits(obj), [truthSeg](const xAOD::MuonSimHit* hit) {
                return MuonR4::getMatchedTruthSegment(*hit) == truthSeg;
             }) : 0;
    }
}


namespace MuonValR4 {
    using namespace MuonR4;
    using namespace MuonVal;
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
        ATH_CHECK(m_tree.init(this)); 
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));

        ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));
        ATH_MSG_DEBUG("Succesfully initialised");
        return StatusCode::SUCCESS;
    }


    unsigned int MuonHoughTransformTester::countOnSameSide(const ActsTrk::GeometryContext& gctx,
                                                           const xAOD::MuonSegment& truthSeg,
                                                           const MuonR4::Segment& recoSeg) const{
        unsigned int same{0};
        using namespace SegmentFit;
        const auto[truePos, trueDir] = makeLine(localSegmentPars(truthSeg)); 
        const auto[recoPos, recoDir] = makeLine(localSegmentPars(gctx, recoSeg));
        const std::vector<int> truthSigns = SeedingAux::strawSigns(truePos, trueDir, recoSeg.measurements());
        const std::vector<int> recoSigns = SeedingAux::strawSigns(recoPos, recoDir, recoSeg.measurements());
        for (unsigned int s = 0 ; s < truthSigns.size(); ++s) {
            same += (truthSigns[s] != 0) && truthSigns[s] == recoSigns[s];
        }
        return same;
    }
    std::vector<ObjectMatching> 
            MuonHoughTransformTester::matchWithTruth(const ActsTrk::GeometryContext& gctx,
                                                     const MuonR4::SegmentSeedContainer& seedContainer,
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
            if (truthSeg) {
                assoc_itr = std::ranges::find_if(allAssociations, [truthSeg](const ObjectMatching& obj){
                    return obj.truthSegment == truthSeg;
                });
            }
            if (assoc_itr == allAssociations.end()) {
                ObjectMatching & newObj = allAssociations.emplace_back();
                newObj.chamber = m_detMgr->getSectorEnvelope(recoSeg->chamberIndex(),
                                                             recoSeg->sector(),
                                                             recoSeg->etaIndex());
                newObj.truthSegment = truthSeg;
                assoc_itr = allAssociations.end() -1;
            }
            ObjectMatching& assocObj{*assoc_itr};
            assocObj.matchedSegments.push_back(segment);
            if (!truthSeg) {
                assocObj.matchedSeeds.push_back(segment->parent());
            }
            assocObj.matchedSeedFoundSegment.push_back(1);
            usedSeeds.insert(segment->parent());
        }
        
        if (truthSegments) {
            for (ObjectMatching& assocObj : allAssociations) {
                if (!assocObj.truthSegment) {
                    continue;
                }
                std::ranges::sort(assocObj.matchedSegments, 
                                  [&](const Segment* a, const Segment* b){
                                        return countOnSameSide(gctx,*assocObj.truthSegment, *a) >
                                               countOnSameSide(gctx,*assocObj.truthSegment, *b);
                                  });
                std::ranges::transform(assocObj.matchedSegments, std::back_inserter(assocObj.matchedSeeds),
                                       &MuonR4::Segment::parent);
            }
            
        }
        /// Next loop over all seeds
        for (const SegmentSeed* seed : seedContainer) {
            /// Don't recycle the  used seeds again
            if (usedSeeds.count(seed)) {
                continue;
            }
            /// Find the best matching truth segment
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
    StatusCode MuonHoughTransformTester::execute()  {
        
        const EventContext & ctx = Gaudi::Hive::currentContext();

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
        std::vector<ObjectMatching> objects = matchWithTruth(gctx, *segmentSeeds.asDataVector(), 
                                                             *recoSegments, truthSegments);
        for (const ObjectMatching& obj : objects) {
            fillChamberInfo(obj.chamber);
            fillSeedInfo(obj);
            fillSegmentInfo(gctx, obj);
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
        m_out_hasTruth = true; 

        const Amg::Vector3D segDir{segment->direction()};
        static const SG::ConstAccessor<float> acc_pt{"pt"};
        static const SG::ConstAccessor<float> acc_charge{"charge"};
        // eta is interpreted as the eta-location 
        m_out_gen_Eta = segDir.eta();
        m_out_gen_Phi = segDir.phi();
        m_out_gen_Pt  = acc_pt(*segment);
        m_out_gen_Q = acc_charge(*segment);

        const auto [chamberPos, chamberDir] = SegmentFit::makeLine(SegmentFit::localSegmentPars(*segment));
        m_out_gen_nHits = segment->nPrecisionHits()+segment->nPhiLayers() + segment->nTrigEtaLayers(); 
        using namespace Muon::MuonStationIndex;
        m_out_gen_nMDTHits = segment->nPrecisionHits() * (segment->technology() == TechnologyIndex::MDT); 
        m_out_gen_nNswHits = segment->nPrecisionHits() * (segment->technology() != TechnologyIndex::MDT); 
        m_out_gen_nTGCHits = (segment->nPhiLayers() + segment->nTrigEtaLayers()) * !isBarrel(segment->chamberIndex());
        m_out_gen_nRPCHits = (segment->nPhiLayers() + segment->nTrigEtaLayers()) *  isBarrel(segment->chamberIndex());
        
        unsigned nMMHits{0}, nSTGHits{0};
        for (const xAOD::MuonSimHit* simHit : getMatchingSimHits(*segment)) {
            const TechnologyIndex simIdx = m_idHelperSvc->technologyIndex(simHit->identify());
            nMMHits  += simIdx == TechnologyIndex::MM;
            nSTGHits += simIdx == TechnologyIndex::STGC;
        }
        m_out_gen_nMmHits = nMMHits;
        m_out_gen_nSTGCHits = nSTGHits;

        m_out_gen_tantheta = houghTanBeta(chamberDir); 
        m_out_gen_tanphi   = houghTanAlpha(chamberDir);
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
            const Amg::Transform3D localToChamber = RE->msSector()->globalToLocalTransform(gctx) * RE->localToGlobalTransform(gctx, hash);
            const Amg::Vector3D chamberPos = localToChamber * xAOD::toEigen(hit->localPosition()); 
            minYhit = std::min(chamberPos.y(), minYhit); 
            maxYhit = std::max(chamberPos.y(), maxYhit); 
        }
        m_out_gen_minYhit = minYhit;
        m_out_gen_maxYhit = maxYhit;

        ATH_MSG_DEBUG("A true max on chamber index "<<m_out_chamberIndex.getVariable()<<" side "<<m_out_stationSide.getVariable()<<" phi "<<m_out_stationPhi.getVariable()<<" with "
                    <<m_out_gen_nMDTHits.getVariable()<<" MDT and "<<m_out_gen_nRPCHits.getVariable()+m_out_gen_nTGCHits.getVariable()<< " trigger hits is at "
                    <<m_out_gen_tantheta.getVariable()<<" and "<<m_out_gen_y0.getVariable()); 

        const xAOD::TruthParticle* truthMuon = getTruthMatchedParticle(*segment);
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
        double maxHitGap{-1.f};
        for (auto itr = bucket.begin(); itr != bucket.end(); ++itr){
            if (!(*itr)->measuresEta()) {
                continue;
            }
            auto itr1 = std::find_if(itr+1, bucket.end(),[](const SpacePointBucket::value_type& sp){
                return sp->measuresEta();
            });
            if (itr1 == bucket.end()){
                break;
            }
            maxHitGap = std::max(maxHitGap, std::abs( (*itr)->localPosition().y() - 
                                                      (*itr1)->localPosition().y()));
        }
        m_out_bucketHitGap = maxHitGap;
        m_out_nPrecSpacePoints = std::ranges::count_if(bucket, [](const SpacePointBucket::value_type& sp){
            return isPrecisionHit(*sp);
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
            return isPrecisionHit(*sp) && m_visionTool->isLabeled(*sp);
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
            m_out_seed_nMatchedHits.push_back(countMatched(obj.truthSegment, *seed));
            m_out_seed_y0.push_back(seed->interceptY());
            m_out_seed_tantheta.push_back(seed->tanBeta());
            if (seed->hasPhiExtension()){
                m_out_seed_x0.push_back(seed->interceptX());
                m_out_seed_tanphi.push_back(seed->tanAlpha());
            } else{
                m_out_seed_x0.push_back(-999);
                m_out_seed_tanphi.push_back(-999);
            }
         
            m_out_seed_nHits.push_back(seed->getHitsInMax().size());
            unsigned nMdtSeed{0}, nRpcSeed{0}, nTgcSeed{0}, nMmEtaSeed{0}, nMmStereoSeed{0},
                     nsTgcStripSeed{0}, nsTgcWireSeed{0}, nsTgcPadSeed{0}; 
            unsigned nPrecHits{0}, nEtaHits{0}, nPhiHits{0}, nTrueHits{0}, nTruePrecHits{0}, nTrueEtaHits{0}, nTruePhiHits{0};
            std::vector<unsigned char> treeIdxs{};
          
            for (const HoughHitType & houghSP: seed->getHitsInMax()){                
                if (m_writeSpacePoints){
                    unsigned treeIdx = m_spTester->push_back(*houghSP);
                    treeIdxs.push_back(treeIdx);
                }
                nPrecHits += isPrecisionHit(*houghSP);
                nPhiHits  += houghSP->measuresPhi();
                nEtaHits  += houghSP->measuresEta();

                if (m_visionTool.isEnabled()) {
                    nTrueHits += m_visionTool->isLabeled(*houghSP);
                    nTruePrecHits += m_visionTool->isLabeled(*houghSP) && isPrecisionHit(*houghSP);
                    nTruePhiHits += m_visionTool->isLabeled(*houghSP) && houghSP->measuresPhi();
                    nTrueEtaHits += m_visionTool->isLabeled(*houghSP) && houghSP->measuresEta();
                }
                switch (houghSP->type()) {
                    case xAOD::UncalibMeasType::MdtDriftCircleType: 
                        ++nMdtSeed;
                        break;
                    case xAOD::UncalibMeasType::RpcStripType:
                        nRpcSeed+=houghSP->measuresEta();
                        nRpcSeed+=houghSP->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::TgcStripType:
                        nTgcSeed+=houghSP->measuresEta();
                        nTgcSeed+=houghSP->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::sTgcStripType: {
                        const Identifier sTgc = houghSP->primaryMeasurement()->identify();
                        const Identifier sTgc2 = houghSP->secondaryMeasurement() ? 
                                                houghSP->secondaryMeasurement()->identify() : Identifier{};
                        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
                        const int primType = idHelper.channelType(sTgc);
                        const int secType = idHelper.channelType(sTgc2);
                        nsTgcStripSeed += primType == sTgcIdHelper::sTgcChannelTypes::Strip;
                        nsTgcWireSeed  += primType == sTgcIdHelper::sTgcChannelTypes::Wire;
                        nsTgcPadSeed   += primType == sTgcIdHelper::sTgcChannelTypes::Pad;

                        nsTgcWireSeed += secType == sTgcIdHelper::sTgcChannelTypes::Wire;
                        nsTgcPadSeed += primType != sTgcIdHelper::sTgcChannelTypes::Pad && 
                                        secType == sTgcIdHelper::sTgcChannelTypes::Pad;
                        break;
                    } case xAOD::UncalibMeasType::MMClusterType:{
                        if (m_idHelperSvc->mmIdHelper().isStereo(houghSP->identify())) {
                            ++nMmEtaSeed;
                        } else {
                            ++nMmStereoSeed;
                        }
                        break;
                    }default:
                        ATH_MSG_WARNING("Technology "<<houghSP->identify()  <<" not yet implemented");                        
                }                    
            }
            m_out_seed_nMdt.push_back(nMdtSeed);
            m_out_seed_nRpc.push_back(nRpcSeed);
            m_out_seed_nTgc.push_back(nTgcSeed);

            m_out_seed_nMmEta.push_back(nMmEtaSeed);
            m_out_seed_nMmStereo.push_back(nMmStereoSeed);

            m_out_seed_nsTgcStrip.push_back(nsTgcStripSeed);
            m_out_seed_nsTgcWire.push_back(nsTgcWireSeed);
            m_out_seed_nsTgcPad.push_back(nsTgcPadSeed);

            m_out_seed_nPrecHits.push_back(nPrecHits);
            m_out_seed_nEtaHits.push_back(nEtaHits); 
            m_out_seed_nPhiHits.push_back(nPhiHits);

            m_out_seed_nTrueHits.push_back(nTrueHits);
            m_out_seed_nTruePrecHits.push_back(nTruePrecHits);            
            m_out_seed_nTrueEtaHits.push_back(nTrueEtaHits);
            m_out_seed_nTruePhiHits.push_back(nTruePhiHits);

            m_out_seed_ledToSegment.push_back(obj.matchedSeedFoundSegment.at(iseed));
            if (m_writeSpacePoints) {
                m_spMatchedToPattern[iseed] = std::move(treeIdxs);
                
            } 
        }
    }
    
    void MuonHoughTransformTester::fillSegmentInfo(const ActsTrk::GeometryContext& gctx,
                                                   const ObjectMatching& obj){
        using namespace SegmentFit;

        m_out_segment_n = obj.matchedSegments.size(); 
        for (const Segment* segment : obj.matchedSegments) {
            m_out_segment_hasPhi.push_back(std::ranges::any_of(segment->measurements(), 
                                                [](const auto& meas){  return meas->measuresPhi();}));
            m_out_segment_fitIter.push_back(segment->nFitIterations());
            m_out_segment_truthMatchedHits.push_back(countMatched(obj.truthSegment, *segment));
            m_out_segment_chi2.push_back(segment->chi2());
            m_out_segment_nDoF.push_back(segment->nDoF());
            m_out_segment_hasTimeFit.push_back(segment->hasTimeFit());

            m_out_segment_err_x0.push_back(segment->covariance()(Acts::toUnderlying(ParamDefs::x0), Acts::toUnderlying(ParamDefs::x0)));
            m_out_segment_err_y0.push_back(segment->covariance()(Acts::toUnderlying(ParamDefs::y0), Acts::toUnderlying(ParamDefs::y0)));
            m_out_segment_err_tantheta.push_back(segment->covariance()(Acts::toUnderlying(ParamDefs::theta), Acts::toUnderlying(ParamDefs::theta)));
            m_out_segment_err_tanphi.push_back(segment->covariance()(Acts::toUnderlying(ParamDefs::phi), Acts::toUnderlying(ParamDefs::phi)));
            m_out_segment_err_time.push_back(segment->covariance()(Acts::toUnderlying(ParamDefs::t0), Acts::toUnderlying(ParamDefs::t0)));
            const auto [locPos, locDir] = makeLine(localSegmentPars(gctx, *segment));
            m_out_segment_tanphi.push_back(houghTanAlpha(locDir));
            m_out_segment_tantheta.push_back(houghTanBeta(locDir));
            m_out_segment_y0.push_back(locPos.y());
            m_out_segment_x0.push_back(locPos.x());
            m_out_segment_time.push_back(segment->segementT0() + segment->position().mag() * c_inv);

            unsigned nMdtHits{0}, nRpcEtaHits{0}, nRpcPhiHits{0}, nTgcEtaHits{0}, nTgcPhiHits{0},
                     nMmEtaHits{0}, nMmStereoHits{0}, nStgcStripHits{0},nStgcWireHits{0}, nStgcPadHits{0};
            unsigned nTrueHits{0}, nTruePrecHits{0}, nTrueEtaHits{0}, nTruePhiHits{0};
            
            double minYhit = std::numeric_limits<double>::max();
            double maxYhit = -1 * std::numeric_limits<double>::max();

            std::vector<unsigned char> matched;
            for (const auto & meas : segment->measurements()){
                // skip dummy measurement from beam spot constraint
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" Dump "<<(*meas));
                if (meas->type() == xAOD::UncalibMeasType::Other) {
                    continue;
                }
                minYhit = std::min(meas->localPosition().y(),minYhit); 
                maxYhit = std::max(meas->localPosition().y(),maxYhit);
                if (m_writeSpacePoints) {
                    unsigned treeIdx = m_spTester->push_back(*meas->spacePoint());
                    if (treeIdx >= matched.size()){
                        matched.resize(treeIdx +1);
                    }
                    matched[treeIdx] = true;
                }
                if (m_visionTool.isEnabled()) {
                    nTrueHits += m_visionTool->isLabeled(*meas->spacePoint());
                    nTruePrecHits += isPrecisionHit(*meas) && m_visionTool->isLabeled(*meas->spacePoint());
                    nTrueEtaHits += meas->measuresEta() && m_visionTool->isLabeled(*meas->spacePoint());
                    nTruePhiHits += meas->measuresPhi() && m_visionTool->isLabeled(*meas->spacePoint());
                }
                switch (meas->type()) {
                    case xAOD::UncalibMeasType::MdtDriftCircleType:
                        ++nMdtHits;
                        break;
                    case xAOD::UncalibMeasType::RpcStripType:
                        nRpcEtaHits += meas->measuresEta();
                        nRpcPhiHits += meas->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::TgcStripType:
                        nTgcEtaHits += meas->measuresEta();
                        nTgcPhiHits += meas->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::MMClusterType:{
                        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
                        nMmEtaHits += !idHelper.isStereo(meas->spacePoint()->identify());
                        nMmStereoHits += !idHelper.isStereo(meas->spacePoint()->identify());
                        break;
                    } case xAOD::UncalibMeasType::sTgcStripType: {
                        const auto* prd = static_cast<const xAOD::sTgcMeasurement*>(meas->spacePoint()->primaryMeasurement());
                        const int primType = prd->channelType();
                        prd = dynamic_cast<const xAOD::sTgcMeasurement*>(meas->spacePoint()->secondaryMeasurement());
                        const int secType = (prd != nullptr ? prd->channelType() : -1);
                        nStgcStripHits += primType == sTgcIdHelper::sTgcChannelTypes::Strip;
                        nStgcWireHits  += primType == sTgcIdHelper::sTgcChannelTypes::Wire;
                        nStgcPadHits   += primType == sTgcIdHelper::sTgcChannelTypes::Pad;
                        nStgcWireHits  += secType == sTgcIdHelper::sTgcChannelTypes::Wire;
                        nStgcPadHits   += primType != secType && secType == sTgcIdHelper::sTgcChannelTypes::Pad;
                        break;
                    } default:
                        break;
                }
            }
            m_out_segment_nMdtHits.push_back(nMdtHits);
            m_out_segment_nRpcEtaHits.push_back(nRpcEtaHits);
            m_out_segment_nRpcPhiHits.push_back(nRpcPhiHits);
            m_out_segment_nTgcEtaHits.push_back(nTgcEtaHits);
            m_out_segment_nTgcPhiHits.push_back(nTgcPhiHits);

            m_out_segment_nMmEtaHits.push_back(nMmEtaHits);
            m_out_segment_nMmStereoHits.push_back(nMmStereoHits);
            m_out_segment_nsTgcStripHits.push_back(nStgcStripHits);
            m_out_segment_nsTgcWireHits.push_back(nStgcWireHits);
            m_out_segment_nsTgcPadpHits.push_back(nStgcPadHits);
        

            m_out_segment_nTrueHits.push_back(nTrueHits);
            m_out_segment_nTruePrecHits.push_back(nTruePrecHits);
            m_out_segment_nTruePhiHits.push_back(nTruePhiHits);
            m_out_segment_nTrueEtaHits.push_back(nTrueEtaHits);

            m_out_segment_minYhit.push_back(minYhit);
            m_out_segment_maxYhit.push_back(maxYhit);
            if (m_writeSpacePoints) {
                m_spMatchedToSegment.push_back(std::move(matched));
            }
        }
    }
}  // namespace MuonValR4
