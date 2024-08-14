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

namespace MuonValR4 {
    MuonHoughTransformTester::MuonHoughTransformTester(const std::string& name,
                                ISvcLocator* pSvcLocator)
        : AthHistogramAlgorithm(name, pSvcLocator) {}


    StatusCode MuonHoughTransformTester::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_inSimHitKeys.initialize());
        ATH_CHECK(m_spacePointKey.initialize());
        ATH_CHECK(m_inHoughSegmentSeedKey.initialize());
        ATH_CHECK(m_truthSegmentKey.initialize());
        ATH_CHECK(m_rh_truthSegmentSimHitLink.initialize());
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

   
    void MuonHoughTransformTester::matchSeedToTruth(const EventContext & ctx, const MuonR4::SegmentSeed* seed, 
                                                    chamberLevelObjects & objs ) const{
        double bestTruthFrac{0.}; 
        const xAOD::MuonSegment* bestMatch = nullptr; 
        SG::ReadDecorHandle<xAOD::MuonSegmentContainer,SimHitLinkVec> simHitsFromTruth(m_rh_truthSegmentSimHitLink, ctx);
        SG::ConstAccessor<ElementLink<xAOD::MuonSimHitContainer>> simHitAcc("simHitLink"); 

        for (auto & [ truthsegment, truthQuantities] : objs.truthMatching) {
            unsigned int nRecFound{0}; 
            const SimHitLinkVec & simHitsForSegment = simHitsFromTruth(*truthsegment); 

            for (const MuonR4::HoughHitType& spacePoint : seed->getHitsInMax()) {
                if (!simHitAcc.isAvailable(*spacePoint->primaryMeasurement())) continue; 
                auto simHitMatch = simHitAcc(*spacePoint->primaryMeasurement()); 
                if (!simHitMatch.isValid() || *simHitMatch == nullptr){
                    continue;
                }
                auto found = std::ranges::find_if( simHitsForSegment, [&simHitMatch](const ElementLink<xAOD::MuonSimHitContainer> & el){
                    return (el == simHitMatch);
                });
                if (found != simHitsForSegment.end()){
                    ++nRecFound; 
                }
            }
            double truthFraction = (1.*nRecFound) / (1.*seed->getHitsInMax().size()); 
            if (truthFraction > bestTruthFrac) {
                bestMatch = truthsegment;
                bestTruthFrac = truthFraction; 
            }
        }
        if (!bestMatch) return;
        /** Map the seed to the truth particle */
        chamberLevelObjects::SeedMatchQuantites& seedMatch = objs.seedMatching[seed];
        seedMatch.matchProb = bestTruthFrac;
        seedMatch.truthsegment = bestMatch;
        /** Back mapping of the best truth -> seed */
        objs.truthMatching[bestMatch].assocSeeds.push_back(seed);
    }
  
    void MuonHoughTransformTester::matchSeedsToTruth(const EventContext & ctx,chamberLevelObjects & objs) const {        
        for (auto & [ seed, matchObj] : objs.seedMatching) {
            matchSeedToTruth(ctx, seed, objs);
            ATH_MSG_VERBOSE("Truth matching probability "<<matchObj.matchProb);           
        }
    }
          
    void MuonHoughTransformTester::fillChamberInfo(const MuonGMR4::MuonChamber* chamber){
        m_out_stationName = chamber->stationName();
        m_out_stationEta = chamber->stationEta();
        m_out_stationPhi = chamber->stationPhi();
    }                
    void MuonHoughTransformTester::fillTruthInfo(const EventContext & ctx, const xAOD::MuonSegment* segment,const ActsGeometryContext & gctx){
        if (!segment) return; 
        m_out_hasTruth = true; 
        Amg::Vector3D segPos{
            segment->x(),
            segment->y(),
            segment->z(),
        }; 
        Amg::Vector3D segDir{
            segment->px(),
            segment->py(),
            segment->pz(),
        };
        // eta is interpreted as the eta-location 
        m_out_gen_Eta   = segDir.eta();
        m_out_gen_Phi= segDir.phi();
        m_out_gen_Pt= segDir.perp();

        SG::ReadDecorHandle<xAOD::MuonSegmentContainer,SimHitLinkVec> simHitsFromTruth(m_rh_truthSegmentSimHitLink, ctx);
        const SimHitLinkVec & simHits = simHitsFromTruth(*segment);         
        const xAOD::MuonSimHit* firstSimHit = *(simHits.front()); 
        const Identifier ID = firstSimHit->identify();
        const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(ID); 
        //transform from local (w.r.t tube's frame) to global (ATLAS frame) and then to chamber's frame
        const MuonGMR4::MuonChamber* muonChamber = reElement->getChamber();
        auto toChamber = muonChamber->globalToLocalTrans(gctx);
        const Amg::Vector3D chamberPos{toChamber * segPos};
        Amg::Vector3D chamberDir = toChamber.linear() * segDir;
        
        m_out_gen_nHits = segment->nPrecisionHits()+segment->nPhiLayers() + segment->nTrigEtaLayers(); 
       
        m_out_gen_nMDTHits = (segment->technology() == Muon::MuonStationIndex::MDT ? segment->nPrecisionHits() : 0); 
        m_out_gen_nNswHits = (segment->technology() != Muon::MuonStationIndex::MDT ? segment->nPrecisionHits() : 0); 
        m_out_gen_nTGCHits = (segment->chamberIndex() > Muon::MuonStationIndex::ChIndex::BEE ? segment->nPhiLayers() + segment->nTrigEtaLayers() : 0);
        m_out_gen_nRPCHits = (segment->chamberIndex() <= Muon::MuonStationIndex::ChIndex::BEE ? segment->nPhiLayers() + segment->nTrigEtaLayers() : 0);


        m_out_gen_tantheta = (std::abs(chamberDir.z()) > 1.e-8 ? chamberDir.y()/chamberDir.z() : 1.e10); 
        m_out_gen_tanphi = (std::abs(chamberDir.z()) > 1.e-8 ? chamberDir.x()/chamberDir.z() : 1.e10); 
        m_out_gen_y0 = chamberPos.y(); 
        m_out_gen_x0 = chamberPos.x(); 
        ATH_MSG_DEBUG("A true max on "<<m_out_stationName.getVariable()<<" eta "<<m_out_stationEta.getVariable()<<" phi "<<m_out_stationPhi.getVariable()<<" with "<<m_out_gen_nMDTHits.getVariable()<<" MDT and "<<m_out_gen_nRPCHits.getVariable()+m_out_gen_nTGCHits.getVariable()<< " trigger hits is at "<<m_out_gen_tantheta.getVariable()<<" and "<<m_out_gen_y0.getVariable()); 
    }
    void MuonHoughTransformTester::fillSeedInfo(const MuonR4::SegmentSeed* foundMax, 
                                                double matchProb) {
        if (!foundMax) return; 
        m_out_hasMax = true; 
        m_out_max_hasPhiExtension = foundMax->hasPhiExtension();
        m_out_max_matchFraction = matchProb; 
        m_out_max_tantheta = foundMax->tanTheta();
        m_out_max_y0 = foundMax->interceptY();
        if (m_out_max_hasPhiExtension.getVariable()){
            m_out_max_tanphi = foundMax->tanPhi();
            m_out_max_x0 = foundMax->interceptX(); 
        }
        m_out_max_nHits = foundMax->getHitsInMax().size(); 
        m_out_max_nEtaHits = std::accumulate(foundMax->getHitsInMax().begin(), foundMax->getHitsInMax().end(),0,
                                             [](int i, const MuonR4::HoughHitType & h){i += h->measuresEta();return i;}); 
        m_out_max_nPhiHits = std::accumulate(foundMax->getHitsInMax().begin(), foundMax->getHitsInMax().end(),0,
                                            [](int i, const MuonR4::HoughHitType & h){i += h->measuresPhi();return i;}); 
        unsigned int nMdtMax{0}, nRpcMax{0}, nTgcMax{0}, nMmMax{0}, nsTgcMax{0}; 
        for (const MuonR4::HoughHitType & houghSP: foundMax->getHitsInMax()){
            m_sacePointOnSeed[m_out_SP->push_back(*houghSP)] = true; 
            const xAOD::UncalibratedMeasurement* meas = houghSP->primaryMeasurement();
            switch (meas->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: 
                        ++nMdtMax;
                    break;
                case xAOD::UncalibMeasType::RpcStripType:
                    ++nRpcMax;
                    break;
                case xAOD::UncalibMeasType::TgcStripType:
                    ++nTgcMax;
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
                                                   const MuonR4::Segment* segment, double matchProb){
        using namespace MuonR4::SegmentFit;
        if (!segment) return; 
        m_out_hasSegment = true; 
        m_out_segment_matchFraction = matchProb; 
        m_out_segment_chi2 = segment->chi2();
        m_out_segment_nDoF = segment->nDoF();

        m_out_segment_err_x0 = segment->covariance()[toInt(AxisDefs::x0)];
        m_out_segment_err_y0 = segment->covariance()[toInt(AxisDefs::y0)];
        m_out_segment_err_tantheta= segment->covariance()[toInt(AxisDefs::tanTheta)];
        m_out_segment_err_tanphi = segment->covariance()[toInt(AxisDefs::tanPhi)];

        const Amg::Transform3D trf{segment->chamber()->globalToLocalTrans(gctx)};
        for (const double c2 : segment->chi2PerMeasurement()){
            m_out_segment_chi2_measurement.push_back(c2); 
        }
        const Amg::Vector3D locPos = trf * segment->position();
        const Amg::Vector3D locDir = trf.linear()* segment->direction();
        m_out_segment_tanphi = locDir.x() / locDir.z();
        m_out_segment_tantheta = locDir.y() / locDir.z();
        m_out_segment_y0 = locPos.y();
        m_out_segment_x0 = locPos.x();
    }
    StatusCode MuonHoughTransformTester::execute()  {
        
        const EventContext & ctx = Gaudi::Hive::currentContext();
        const ActsGeometryContext* gctxPtr{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctxPtr));
        const ActsGeometryContext& gctx{*gctxPtr};

        // retrieve the two input collections
        
        const MuonR4::SegmentSeedContainer* readSegmentSeeds{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_inHoughSegmentSeedKey, readSegmentSeeds));
        
        const MuonR4::SegmentContainer* readMuonSegments{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_inSegmentKey, readMuonSegments));
        
        const xAOD::MuonSegmentContainer* readTruthSegments{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_truthSegmentKey, readTruthSegments));
        
        SG::ReadDecorHandle<xAOD::MuonSegmentContainer, SimHitLinkVec> readSimHits{m_rh_truthSegmentSimHitLink, ctx}; 

        ATH_MSG_DEBUG("Succesfully retrieved input collections");

        // map the drift circles to identifiers. 
        // The fast digi should only generate one circle per tube. 
        std::map<const MuonGMR4::MuonChamber*, chamberLevelObjects> allObjectsPerChamber; 

        for (const xAOD::MuonSegment* truthSegment : *readTruthSegments){
            const SimHitLinkVec & simHits = readSimHits(*truthSegment); 
            const Identifier simHitId = (*simHits.front())->identify();
            const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(simHitId); 
            const MuonGMR4::MuonChamber* id{reElement->getChamber()};
            chamberLevelObjects & theObjects  = allObjectsPerChamber[id];
            theObjects.truthMatching[truthSegment].truthsegment = truthSegment; 
        }

        // Populate the seeds first
        for (const MuonR4::SegmentSeed* max : *readSegmentSeeds) {
            // use adventurous minion syntax to add a key with a default value
            allObjectsPerChamber[max->chamber()].seedMatching[max];
        }
        if (readMuonSegments) {
            for (const MuonR4::Segment* segment : *readMuonSegments){
                chamberLevelObjects&  thechamber = allObjectsPerChamber[segment->chamber()];
                chamberLevelObjects::SeedMatchMap& recoOnChamber = thechamber.seedMatching;
                recoOnChamber[segment->parent()].segment = segment; 
            }
        }

        for (auto & [chamber, chamberLevelObjects] : allObjectsPerChamber){
            matchSeedsToTruth(ctx, chamberLevelObjects);            
            /// Step 1: Fill the matched pairs 
            for (auto & [truthSegment, assocInfo] : chamberLevelObjects.truthMatching) {                
                if (assocInfo.assocSeeds.empty()) {
                    fillChamberInfo(chamber); 
                    fillTruthInfo(ctx, truthSegment, gctx);
                    if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
                    continue;
                }
                for (const MuonR4::SegmentSeed* seed : assocInfo.assocSeeds) {
                    fillChamberInfo(chamber); 
                    fillTruthInfo(ctx, truthSegment, gctx);
                    auto& seedMatch = chamberLevelObjects.seedMatching[seed];
                    m_out_SP->push_back(*seed->parentBucket());
                    fillSeedInfo(seed, seedMatch.matchProb);
                    if (seedMatch.segment) {
                        fillSegmentInfo(gctx, seedMatch.segment, seedMatch.matchProb);
                    }
                    if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
                }
            }
            // also fill the reco not matched to any truth 
            for (auto & [ seed, assocInfo ] : chamberLevelObjects.seedMatching) {
                if (assocInfo.truthsegment) continue;
                fillChamberInfo(chamber);
                m_out_SP->push_back(*seed->parentBucket());
                fillSeedInfo(seed, 0.); 
                if (assocInfo.segment) {
                    fillSegmentInfo(gctx, assocInfo.segment, 0.);
                }
                if (!m_tree.fill(ctx)) return StatusCode::FAILURE; 
            }
        } // end loop over chambers

        return StatusCode::SUCCESS;
    }
}  // namespace MuonValR4
