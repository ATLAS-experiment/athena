/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughTransformTester.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "GaudiKernel/PhysicalConstants.h"

namespace {
    constexpr double c_inv = 1. /Gaudi::Units::c_light;
}
namespace MuonValR4 {
    using namespace MuonR4;
    using TruthHitCol = std::unordered_set<const xAOD::MuonSimHit*>;
    unsigned int countMatched(const TruthHitCol& truthHits,
                              const TruthHitCol& recoHits) {
        unsigned int matched{0};
        for (const xAOD::MuonSimHit* reco : recoHits) {
            matched += truthHits.count(reco);
        }
        return matched;
    }
    std::vector<MuonHoughTransformTester::ObjectMatching> 
            MuonHoughTransformTester::matchWithTruth(const xAOD::MuonSegmentContainer* truthSegments,
                                                     const SegmentSeedContainer* seedContainer,
                                                     const SegmentContainer* segmentContainer) const {
        std::vector<ObjectMatching> assocObj{}, assocObjMultMatch{};
        std::unordered_set<const SegmentSeed*> usedSeeds{};
        std::unordered_set<const Segment*> usedSegs{};
        std::vector<TruthHitCol> truthHitsVec{}, seedHitsVec{}, segmentHitsVec{};

        if (truthSegments) {
            for (const SegmentSeed* seed: *seedContainer) {
                seedHitsVec.emplace_back(getTruthMatchedHits(*seed));
            }
            for (const Segment* segment: *segmentContainer){
                segmentHitsVec.emplace_back(getTruthMatchedHits(*segment));
            }

            for (const xAOD::MuonSegment* truth: *truthSegments) {
                const TruthHitCol& truthHits{truthHitsVec.emplace_back(getTruthMatchedHits(*truth))};
                ObjectMatching matchTempl{};
                matchTempl.truthSegment = truth;
                matchTempl.chamber = m_r4DetMgr->getChamber((*truthHits.begin())->identify());
                int seedIdx{-1};
                for (const SegmentSeed* seed : *seedContainer){
                    ++seedIdx;
                    if (seed->chamber() != matchTempl.chamber) {
                        continue;
                    }
                    const TruthHitCol& seedHits{seedHitsVec[seedIdx]};
                    unsigned int matchedHits = countMatched(truthHits, seedHits);
                    if (!matchedHits) {
                        continue;
                    }
                    double matchFracion{1.*matchedHits / (1.*seed->getHitsInMax().size())};
                    if (matchFracion > matchTempl.matchFracSeed) {
                        matchTempl.matchFracSeed = matchFracion;
                        matchTempl.matchedSeed = seed;
                        matchTempl.nTruthMatchedMax = matchedHits;
                    } 
                }
                int segmentIdx{-1};
                /** Match segments */
                std::vector<const Segment*> matchedSegs{};
                for (const Segment* segment : *segmentContainer) {
                    ++segmentIdx;
                    if (segment->chamber() != matchTempl.chamber) {
                        continue;
                    }
                    const TruthHitCol& segmentHits{segmentHitsVec[segmentIdx]};
                    unsigned int matchedHits = countMatched(truthHits, segmentHits);
                    if (!matchedHits) {
                        continue;
                    }

                    double matchFracion{1.*matchedHits / (1.*truthHits.size())};
                    if (matchFracion > matchTempl.matchFracSegment) {
                        matchTempl.matchFracSegment = matchFracion;
                        matchTempl.nTruthMatchedSeg = matchedHits;
                        matchTempl.matchedSegment = segment;
                        matchedSegs.clear();
                    /// Same number of hits but maybe different solution?
                    } else if (matchTempl.nTruthMatchedSeg == matchedHits) {
                        matchedSegs.push_back(segment);
                    }
                }
                usedSegs.insert(matchTempl.matchedSegment);
                usedSeeds.insert(matchTempl.matchedSeed);
                assocObj.push_back(matchTempl);
                for (const Segment* matched : matchedSegs) {
                    ObjectMatching matching{matchTempl};
                    matching.matchedSeed = matched->parent();
                    matching.matchedSegment = matched;
                    usedSeeds.insert(matching.matchedSeed);
                    usedSegs.insert(matching.matchedSegment);
                    assocObjMultMatch.push_back(std::move(matching));
                }                
            }
        }
        int segIdx{-1};
        for (const Segment* seg: *segmentContainer) {
            ++segIdx;
            if (usedSegs.count(seg)) {
                continue;
            }
            ObjectMatching match{};
            match.chamber = seg->chamber();
            match.matchedSegment = seg;
            match.matchedSeed = seg->parent();
            for (unsigned int truthIdx = 0 ; truthIdx < truthHitsVec.size(); ++truthIdx){
                if (assocObj[truthIdx].chamber != match.chamber) {
                    continue;
                }
                unsigned int matches = countMatched(truthHitsVec[truthIdx], segmentHitsVec[segIdx]);
                if (!matches) continue;
                match.bestTruthMatch = false;
                const double matchFrac = 1. * matches / (1. *seg->measurements().size());
                if (matchFrac > match.matchFracSegment) {
                    match.matchFracSeed = matchFrac;
                    match.truthSegment = truthSegments->at(truthIdx);
                    match.nTruthMatchedSeg = matches;
                }
            }
            usedSeeds.insert(match.matchedSeed);
            assocObj.push_back(match);
        }
        int seedIdx{-1};
        for (const SegmentSeed* seed: *seedContainer) {
            ++seedIdx;
            if (usedSeeds.count(seed)) {
                continue;
            }
            ObjectMatching match{};
            match.chamber = seed->chamber();
            match.matchedSeed = seed;
            for (unsigned int truthIdx = 0 ; truthIdx < truthHitsVec.size(); ++truthIdx){
                if (assocObj[truthIdx].chamber != match.chamber) {
                    continue;
                }
                unsigned int matches = countMatched(truthHitsVec[truthIdx], seedHitsVec[seedIdx]);
                if (!matches) continue;
                match.bestTruthMatch = false;
                const double matchFrac = 1. * matches / (1. *seed->getHitsInMax().size());
                if (matchFrac > match.matchFracSeed) {
                    match.matchFracSeed = matchFrac;
                    match.truthSegment = truthSegments->at(truthIdx);
                    match.nTruthMatchedMax = matches;
                }
            }
            assocObj.push_back(match);
        }
        assocObj.insert(assocObj.end(), std::make_move_iterator(assocObjMultMatch.begin()),
                                        std::make_move_iterator(assocObjMultMatch.end()));
        return assocObj;
    }


    MuonHoughTransformTester::MuonHoughTransformTester(const std::string& name, ISvcLocator* pSvcLocator): 
        AthHistogramAlgorithm(name, pSvcLocator) {}


    StatusCode MuonHoughTransformTester::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_spacePointKey.initialize());
        ATH_CHECK(m_inHoughSegmentSeedKey.initialize());
        ATH_CHECK(m_truthSegmentKey.initialize(!m_truthSegmentKey.empty()));
        ATH_CHECK(m_inSegmentKey.initialize(!m_inSegmentKey.empty()));
        m_tree.addBranch(std::make_shared<MuonVal::EventInfoBranch>(m_tree,0));
        m_out_SP = std::make_shared<MuonValR4::SpacePointTesterModule>(m_tree, m_spacePointKey.key()); 
        m_tree.addBranch(m_out_SP); 
        ATH_CHECK(m_tree.init(this));
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_r4DetMgr));
        ATH_MSG_DEBUG("Succesfully initialised");
        return StatusCode::SUCCESS;
    }
   template <class ContainerType>
        StatusCode MuonHoughTransformTester::retrieveContainer(const EventContext& ctx, 
                                                               const SG::ReadHandleKey<ContainerType>& key,
                                                               const ContainerType*& contToPush) const {
            contToPush = nullptr;
            if (key.empty()) {
                ATH_MSG_VERBOSE("No key has been parsed for object "<< typeid(ContainerType).name());
                return StatusCode::SUCCESS;
            }
            SG::ReadHandle<ContainerType> readHandle{key, ctx};
            ATH_CHECK(readHandle.isPresent());
            contToPush = readHandle.cptr();
            return StatusCode::SUCCESS;
        }

    StatusCode MuonHoughTransformTester::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }

    Amg::Transform3D MuonHoughTransformTester::toChamberTrf(const ActsGeometryContext& gctx,
                                                         const Identifier& hitId) const {
        const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(hitId); 
        //transform from local (w.r.t tube's frame) to global (ATLAS frame) and then to chamber's frame
        const MuonGMR4::MuonChamber* muonChamber = reElement->getChamber();
        /// Mdt tubes have all their own transform while for the strip detectors there's one transform per layer
        const IdentifierHash trfHash = reElement->detectorType() == ActsTrk::DetectorType::Mdt ?
                                       reElement->measurementHash(hitId) : reElement->layerHash(hitId);            
        return muonChamber->globalToLocalTrans(gctx) * reElement->localToGlobalTrans(gctx, trfHash);
    }
          
    void MuonHoughTransformTester::fillChamberInfo(const MuonGMR4::MuonChamber* chamber){
        m_out_stationName = chamber->stationName();
        m_out_stationEta = chamber->stationEta();
        m_out_stationPhi = chamber->stationPhi();
    }                
    void MuonHoughTransformTester:: fillTruthInfo(const ActsGeometryContext& gctx,
                                                  const MuonGMR4::MuonChamber* muonChamber, 
                                                  const xAOD::MuonSegment* segment) {
        if (!segment) return; 
        m_out_hasTruth = true; 
        Amg::Vector3D segPos{segment->position()}; 
        Amg::Vector3D segDir{segment->direction()};
        static const SG::Accessor<float> acc_pt{"pt"};
        // eta is interpreted as the eta-location 
        m_out_gen_Eta = segDir.eta();
        m_out_gen_Phi = segDir.phi();
        m_out_gen_Pt  = acc_pt(*segment);

        //transform from local (w.r.t tube's frame) to global (ATLAS frame) and then to chamber's frame
        auto toChamber = muonChamber->globalToLocalTrans(gctx);
        const Amg::Vector3D chamberPos{toChamber * segPos};
        Amg::Vector3D chamberDir = toChamber.linear() * segDir;
        
        m_out_gen_nHits = segment->nPrecisionHits()+segment->nPhiLayers() + segment->nTrigEtaLayers(); 
       
        m_out_gen_nMDTHits = (segment->technology() == Muon::MuonStationIndex::MDT ? segment->nPrecisionHits() : 0); 
        m_out_gen_nNswHits = (segment->technology() != Muon::MuonStationIndex::MDT ? segment->nPrecisionHits() : 0); 
        m_out_gen_nTGCHits = (segment->chamberIndex() > Muon::MuonStationIndex::ChIndex::BEE ? segment->nPhiLayers() + segment->nTrigEtaLayers() : 0);
        m_out_gen_nRPCHits = (segment->chamberIndex() <= Muon::MuonStationIndex::ChIndex::BEE ? segment->nPhiLayers() + segment->nTrigEtaLayers() : 0);


        m_out_gen_tantheta = (std::abs(chamberDir.z()) > 1.e-8 ? chamberDir.y()/chamberDir.z() : 1.e10); 
        m_out_gen_tanphi   = (std::abs(chamberDir.z()) > 1.e-8 ? chamberDir.x()/chamberDir.z() : 1.e10); 
        m_out_gen_y0 = chamberPos.y(); 
        m_out_gen_x0 = chamberPos.x(); 
        m_out_gen_time = segment->t0();
        ATH_MSG_DEBUG("A true max on "<<m_out_stationName.getVariable()<<" eta "<<m_out_stationEta.getVariable()<<" phi "<<m_out_stationPhi.getVariable()<<" with "<<m_out_gen_nMDTHits.getVariable()<<" MDT and "<<m_out_gen_nRPCHits.getVariable()+m_out_gen_nTGCHits.getVariable()<< " trigger hits is at "<<m_out_gen_tantheta.getVariable()<<" and "<<m_out_gen_y0.getVariable()); 
    }
    void MuonHoughTransformTester::fillSeedInfo(const ObjectMatching& obj) {
        const SegmentSeed* foundMax = obj.matchedSeed;
        if (!foundMax) return; 
        m_out_hasMax = true; 
        m_out_max_hasPhiExtension = foundMax->hasPhiExtension();
        m_out_max_matchFraction = obj.nTruthMatchedMax; 
        m_out_max_tantheta = foundMax->tanTheta();
        m_out_max_y0 = foundMax->interceptY();
        if (m_out_max_hasPhiExtension.getVariable()){
            m_out_max_tanphi = foundMax->tanPhi();
            m_out_max_x0 = foundMax->interceptX(); 
        }
        m_out_max_nHits = foundMax->getHitsInMax().size(); 
        m_out_max_nEtaHits = std::accumulate(foundMax->getHitsInMax().begin(), foundMax->getHitsInMax().end(),0,
                                             [](int i, const HoughHitType & h){i += h->measuresEta();return i;}); 
        m_out_max_nPhiHits = std::accumulate(foundMax->getHitsInMax().begin(), foundMax->getHitsInMax().end(),0,
                                            [](int i, const HoughHitType & h){i += h->measuresPhi();return i;}); 
        unsigned int nMdtMax{0}, nRpcMax{0}, nTgcMax{0}, nMmMax{0}, nsTgcMax{0}; 
        for (const HoughHitType & houghSP: foundMax->getHitsInMax()){
            m_spacePointOnSeed[m_out_SP->push_back(*houghSP)] = true; 
            switch (houghSP->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: 
                    ++nMdtMax;
                    break;
                case xAOD::UncalibMeasType::RpcStripType:
                    nRpcMax+=houghSP->measuresEta();
                    nRpcMax+=houghSP->measuresPhi();
                    break;
                case xAOD::UncalibMeasType::TgcStripType:
                    nTgcMax+=houghSP->measuresEta();
                    nTgcMax+=houghSP->measuresPhi();
                    break;
                case xAOD::UncalibMeasType::sTgcStripType:
                    ++nsTgcMax;
                    break;
                case xAOD::UncalibMeasType::MMClusterType:
                    ++nMmMax;
                    break;
                default:
                    ATH_MSG_WARNING("Technology "<<m_idHelperSvc->toString(houghSP->identify())
                                <<" not yet implemented");                        
            }                    
        }
        m_out_max_nMdt = nMdtMax;
        m_out_max_nRpc = nRpcMax;
        m_out_max_nTgc = nTgcMax;
        m_out_max_nsTgc = nsTgcMax;
        m_out_max_nMm = nMmMax;
    }
    
    void MuonHoughTransformTester::fillSegmentInfo(const ActsGeometryContext& gctx,
                                                   const ObjectMatching& obj){
        
        const Segment* segment = obj.matchedSegment;
        using namespace SegmentFit;
        if (!segment) return; 
        m_out_hasSegment = true; 
        m_out_segment_hasPhi = std::ranges::find_if(segment->measurements(), [](const auto& meas){  return meas->measuresPhi();}) 
                            !=segment->measurements().end();
        m_out_segment_fitIter = segment->nFitIterations();
        m_out_segment_matchFraction = obj.matchFracSegment; 
        m_out_segment_truthMatchedHits = obj.nTruthMatchedSeg;
        m_out_segment_chi2 = segment->chi2();
        m_out_segment_nDoF = segment->nDoF();
        m_out_segment_hasTimeFit = segment->hasTimeFit();

        m_out_segment_err_x0 = segment->covariance()(toInt(ParamDefs::x0), toInt(ParamDefs::x0));
        m_out_segment_err_y0 = segment->covariance()(toInt(ParamDefs::y0), toInt(ParamDefs::y0));
        m_out_segment_err_tantheta = segment->covariance()(toInt(ParamDefs::theta), toInt(ParamDefs::theta));
        m_out_segment_err_tanphi   = segment->covariance()(toInt(ParamDefs::phi), toInt(ParamDefs::phi));
        m_out_segment_err_time = segment->covariance()(toInt(ParamDefs::time), toInt(ParamDefs::time));
        const Amg::Transform3D trf{segment->chamber()->globalToLocalTrans(gctx)};
        for (const double c2 : segment->chi2PerMeasurement()){
            m_out_segment_chi2_measurement.push_back(c2); 
        }
        const Amg::Vector3D locPos = trf * segment->position();
        const Amg::Vector3D locDir = trf.linear()* segment->direction();
        m_out_segment_tanphi   = locDir.x() / locDir.z();
        m_out_segment_tantheta = locDir.y() / locDir.z();
        m_out_segment_y0 = locPos.y();
        m_out_segment_x0 = locPos.x();
        m_out_segment_time = segment->segementT0() + segment->position().mag() * c_inv;

        unsigned int nMdtHits{0}, nRpcEtaHits{0}, nRpcPhiHits{0}, nTgcEtaHits{0}, nTgcPhiHits{0};
        for (const auto & meas : segment->measurements()){
            switch (meas->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType:
                    m_spacePointOnSegment[m_out_SP->push_back(*meas->spacePoint())] = true;
                    ++nMdtHits;
                    break;
                case xAOD::UncalibMeasType::RpcStripType:
                    m_spacePointOnSegment[m_out_SP->push_back(*meas->spacePoint())] = true;
                    nRpcEtaHits += meas->measuresEta();
                    nRpcPhiHits += meas->measuresPhi();
                    break;
                case xAOD::UncalibMeasType::TgcStripType:
                    m_spacePointOnSegment[m_out_SP->push_back(*meas->spacePoint())] = true;
                    nTgcEtaHits += meas->measuresEta();
                    nTgcPhiHits += meas->measuresPhi();
                    break;
                default:
                    break;
            }
        }
        m_out_segment_nMdtHits = nMdtHits;
        m_out_segment_nRpcEtaHits= nRpcEtaHits;
        m_out_segment_nRpcPhiHits= nRpcPhiHits;
        m_out_segment_nTgcEtaHits= nTgcEtaHits;
        m_out_segment_nTgcPhiHits= nTgcPhiHits;
    }
    StatusCode MuonHoughTransformTester::execute()  {
        
        const EventContext & ctx = Gaudi::Hive::currentContext();
        const ActsGeometryContext* gctxPtr{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctxPtr));
        const ActsGeometryContext& gctx{*gctxPtr};

        // retrieve the two input collections
        
        const SegmentSeedContainer* readSegmentSeeds{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_inHoughSegmentSeedKey, readSegmentSeeds));
        
        const SegmentContainer* readMuonSegments{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_inSegmentKey, readMuonSegments));
        
        const xAOD::MuonSegmentContainer* readTruthSegments{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_truthSegmentKey, readTruthSegments));
        

        ATH_MSG_DEBUG("Succesfully retrieved input collections");

        std::vector<ObjectMatching> objects = matchWithTruth(readTruthSegments, readSegmentSeeds, readMuonSegments);
        for (const ObjectMatching& obj : objects) {
            fillChamberInfo(obj.chamber);
            m_out_gen_bestMatch = obj.bestTruthMatch;
            fillTruthInfo(gctx, obj.chamber, obj.truthSegment);
            fillSeedInfo(obj);
            fillSegmentInfo(gctx, obj);
            ATH_CHECK(m_tree.fill(ctx));
        }

        return StatusCode::SUCCESS;
    }
}  // namespace MuonValR4
