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
#include "StoreGate/ReadCondHandle.h"
#include "GeoModelHelpers/throwExcept.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TArrow.h"
#include "TMarker.h"
#include "TLegend.h"


std::unique_ptr<TLatex> getTLatex(const std::string & label){
    auto tl = std::make_unique<TLatex>( 0.15,0.8,label.c_str());
    tl->SetNDC();
    tl->SetTextFont(53); 
    tl->SetTextSize(18); 
    return tl;
}

std::unique_ptr<TEllipse> getDriftCircleShape(double x, double y, double r, bool maxHit){
    auto ell = std::make_unique<TEllipse>(x, y, r);
    if (r>1e-6) {
        ell->SetLineColor(kRed);
        ell->SetFillColor(kRed);
    } else {
        ell->SetLineColor(kBlue);
        ell->SetFillColor(kBlue);
    }
    ell->SetFillStyle(0);
    if (maxHit) {
        ell->SetFillStyle(1001);
        ell->SetFillColorAlpha(ell->GetFillColor(), 0.8); 
    }
    return ell;
}

std::unique_ptr<TBox> getBoxShape(double x1, double x2, double y1, double y2, bool isRPC, bool maxHit){
    auto box = std::make_unique<TBox>(x1, y1, x2, y2);
    if (isRPC) {
        box->SetLineColor(kViolet); 
        box->SetFillColor(kViolet);
    } else {
        box->SetLineColor(kGreen); 
        box->SetFillColor(kGreen);
    }
    box->SetFillStyle(0);
    if (maxHit) {
        box->SetFillStyle(1001);
        box->SetFillColorAlpha(box->GetFillColor(), 0.8); 
    }
    return box;
}

namespace MuonValR4 {
    


    MuonHoughTransformTester::MuonHoughTransformTester(const std::string& name,
                                ISvcLocator* pSvcLocator)
        : AthHistogramAlgorithm(name, pSvcLocator) {}


    StatusCode MuonHoughTransformTester::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_inSimHitKeys.initialize());
        ATH_CHECK(m_inHoughSegmentSeedKey.initialize());
        ATH_CHECK(m_inSegmentKey.initialize());
        ATH_CHECK(m_spacePointKey.initialize());
        m_tree.addBranch(std::make_shared<MuonVal::EventInfoBranch>(m_tree,0));
        m_out_SP = std::make_shared<MuonValR4::SpacePointTesterModule>(m_tree, m_spacePointKey.key()); 
        m_tree.addBranch(m_out_SP); 
        ATH_CHECK(m_tree.init(this));
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_r4DetMgr));
        ATH_MSG_DEBUG("Succesfully initialised");
        if (m_drawEvtDisplayFailure || m_drawEvtDisplaySuccess) {
            m_allCan = std::make_unique<TCanvas>("AllDisplays", "AllDisplays", 800, 600);
            m_allCan->SaveAs(Form("%s[", m_allCanName.value().c_str()));
        }
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
        if (m_allCan) m_allCan->SaveAs(Form("%s]", m_allCanName.value().c_str()));
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

   
    void MuonHoughTransformTester::matchSeedToTruth(const MuonR4::HoughSegmentSeed* seed, 
                                                    chamberLevelObjects & objs ) const{
        double bestTruthFrac{0.}; 
        HepMC::ConstGenParticlePtr bestMatch = nullptr; 
        for (auto & [ genParticle, truthQuantities] : objs.truthMatching) {
            unsigned int nRecFound{0}; 
            for (const MuonR4::HoughHitType& spacePoint : seed->getHitsInMax()) {
                for (const xAOD::MuonSimHit* simHit : truthQuantities.detectorHits) {
                    if(m_idHelperSvc->isMdt(simHit->identify()) &&
                        spacePoint->identify() == simHit->identify()){
                        ++nRecFound;
                        break;
                    } 
                    //// Sim hits are expressed w.r.t to the gas gap Id. Check whether
                    ///  the hit is in the same gas gap
                    else if (m_idHelperSvc->gasGapId(spacePoint->identify()) == simHit->identify()) {
                        ++nRecFound;
                        // break; // should we... ? 
                    }
                }
            }
            double truthFraction = (1.*nRecFound) / (1.*seed->getHitsInMax().size()); 
            if (truthFraction > bestTruthFrac) {
                bestMatch = genParticle;
                bestTruthFrac = truthFraction; 
            }
        }
        if (!bestMatch) return;
        /** Map the seed to the truth particle */
        chamberLevelObjects::SeedMatchQuantites& seedMatch = objs.seedMatching[seed];
        seedMatch.matchProb = bestTruthFrac;
        seedMatch.truthParticle = bestMatch;
        /** Back mapping of the best truth -> seed */
        objs.truthMatching[bestMatch].assocSeeds.push_back(seed);
    }
  
    void MuonHoughTransformTester::matchSeedsToTruth(chamberLevelObjects & objs) const {        
        for (auto & [ seed, matchObj] : objs.seedMatching) {
            matchSeedToTruth(seed, objs);
            ATH_MSG_VERBOSE("Truth matching probability "<<matchObj.matchProb);           
        }
    }
          
    void MuonHoughTransformTester::fillChamberInfo(const MuonGMR4::MuonChamber* chamber){
        m_out_stationName = chamber->stationName();
        m_out_stationEta = chamber->stationEta();
        m_out_stationPhi = chamber->stationPhi();
    }                
    void MuonHoughTransformTester::fillTruthInfo(const HepMC::ConstGenParticlePtr genParticlePtr, const std::vector<const xAOD::MuonSimHit*> & hits,const ActsGeometryContext & gctx){
        if (!genParticlePtr) return; 
        m_out_hasTruth = true; 
        m_out_gen_Eta   = genParticlePtr->momentum().eta();
        m_out_gen_Phi= genParticlePtr->momentum().phi();
        m_out_gen_Pt= genParticlePtr->momentum().perp();
        
        const xAOD::MuonSimHit* simHit = hits.front(); 
        const Identifier ID = simHit->identify();
                
        const Amg::Transform3D toChamber{toChamberTrf(gctx, ID)};
        const Amg::Vector3D localPos{toChamber * xAOD::toEigen(simHit->localPosition())};
        Amg::Vector3D chamberDir = toChamber.linear() * xAOD::toEigen(simHit->localDirection());
        
        /// Express the simulated hit in the center of the chamber
        const std::optional<double> lambda = Amg::intersect<3>(localPos, chamberDir, Amg::Vector3D::UnitZ(), 0.);
        Amg::Vector3D chamberPos = localPos + (*lambda)*chamberDir;
        m_out_gen_nHits = hits.size(); 
        unsigned int nMdt{0}, nRpc{0}, nTgc{0}, nMm{0}, nsTgc{0}; 
        for (const xAOD::MuonSimHit* hit : hits){
            nMdt += m_idHelperSvc->isMdt(hit->identify()); 
            nRpc += m_idHelperSvc->isRpc(hit->identify()); 
            nTgc += m_idHelperSvc->isTgc(hit->identify()); 
            nMm  += m_idHelperSvc->isMM(hit->identify());
            nsTgc += m_idHelperSvc->issTgc(hit->identify());
        }
        m_out_gen_nRPCHits = nRpc; 
        m_out_gen_nMDTHits = nMdt; 
        m_out_gen_nTGCHits = nTgc;
        m_out_gen_nMMits = nMm;
        m_out_gen_nsTGCHits = nsTgc;

        m_out_gen_tantheta = (std::abs(chamberDir.z()) > 1.e-8 ? chamberDir.y()/chamberDir.z() : 1.e10); 
        m_out_gen_tanphi = (std::abs(chamberDir.z()) > 1.e-8 ? chamberDir.x()/chamberDir.z() : 1.e10); 
        m_out_gen_y0 = chamberPos.y(); 
        m_out_gen_x0 = chamberPos.x(); 
    }
    void MuonHoughTransformTester::fillSeedInfo(const MuonR4::HoughSegmentSeed* foundMax, 
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
    
    void MuonHoughTransformTester::fillSegmentInfo(const MuonR4::MuonSegment* segment, double matchProb){
        if (!segment) return; 
        m_out_hasSegment = true; 
        m_out_segment_matchFraction = matchProb; 
        m_out_segment_chi2 = segment->chi2();
        for (const double c2 : segment->chi2PerMeasurement()){
            m_out_segment_chi2_measurement.push_back(c2); 
        }
        m_out_segment_tanphi = segment->tanPhi();
        m_out_segment_tantheta = segment->tanTheta();
        m_out_segment_z0 = segment->y0();
        m_out_segment_x0 = segment->x0();
    }
    StatusCode MuonHoughTransformTester::execute()  {
        
        const EventContext & ctx = Gaudi::Hive::currentContext();
        const ActsGeometryContext* gctxPtr{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctxPtr));
        const ActsGeometryContext& gctx{*gctxPtr};

        // retrieve the two input collections
        
        const MuonR4::StationHoughSegmentSeedContainer* readSegmentSeeds{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_inHoughSegmentSeedKey, readSegmentSeeds));
        
        const MuonR4::MuonSegmentContainer* readMuonSegments{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_inSegmentKey, readMuonSegments));
        const MuonR4::MuonSpacePointContainer* spacePoints{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_spacePointKey, spacePoints));

        ATH_MSG_DEBUG("Succesfully retrieved input collections");

        // map the drift circles to identifiers. 
        // The fast digi should only generate one circle per tube. 
        std::map<const MuonGMR4::MuonChamber*, chamberLevelObjects> allObjectsPerChamber; 

        for (const SG::ReadHandleKey<xAOD::MuonSimHitContainer>& key : m_inSimHitKeys){
            const xAOD::MuonSimHitContainer* collection{nullptr};
            ATH_CHECK(retrieveContainer(ctx, key, collection));
            for (const xAOD::MuonSimHit* simHit : *collection) {
                const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(simHit->identify()); 
                const MuonGMR4::MuonChamber* id{reElement->getChamber()};
                chamberLevelObjects & theObjects  = allObjectsPerChamber[id];
                auto genLink = simHit->genParticleLink();
                HepMC::ConstGenParticlePtr genParticle = nullptr; 
                if (genLink.isValid()){
                    genParticle = genLink.cptr(); 
                }
                /// skip empty truth matches for now
                if (!genParticle) continue;
                theObjects.truthMatching[genParticle].detectorHits.push_back(simHit); 
            }
        }

        // Populate the seeds first
        for (const MuonR4::StationHoughSegmentSeeds  & max : *readSegmentSeeds) {
            chamberLevelObjects&  thechamber = allObjectsPerChamber[max.chamber()];
            chamberLevelObjects::SeedMatchMap& recoOnChamber = thechamber.seedMatching; 
            for (const MuonR4::HoughSegmentSeed & max : max.getMaxima()) {
               const MuonR4::MuonSpacePointBucket* parentBucket{nullptr};
               /// Find the space point bucket that has all the hits in the maximum
               for (const MuonR4::MuonSpacePointBucket* bucket : *spacePoints) {
                    if (std::accumulate(bucket->begin(), bucket->end(),0u, 
                            [&max](unsigned int i, const MuonR4::MuonSpacePointBucket::value_type & h){
                                return i + (std::find(max.getHitsInMax().begin(),max.getHitsInMax().end(), h) != 
                                            max.getHitsInMax().end());
                            }) == max.getHitsInMax().size()){
                        parentBucket = bucket;
                        break;
                    }
               }
               recoOnChamber[&max].bucket = parentBucket;
               
            }
        }
        for (const MuonR4::MuonSegment& segment : *readMuonSegments){
            chamberLevelObjects&  thechamber = allObjectsPerChamber[segment.chamber()];
            chamberLevelObjects::SeedMatchMap& recoOnChamber = thechamber.seedMatching;
            recoOnChamber[segment.parent()].segment = &segment; 
        }

        for (auto & [chamber, chamberLevelObjects] : allObjectsPerChamber){
            matchSeedsToTruth(chamberLevelObjects);            
            /// Step 1: Fill the matched pairs 
            for (auto & [genParticlePtr, assocInfo] : chamberLevelObjects.truthMatching) {                
                if (assocInfo.assocSeeds.empty()) {
                    fillChamberInfo(chamber); 
                    fillTruthInfo(genParticlePtr, assocInfo.detectorHits, gctx);
                    if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
                    continue;
                }
                for (const MuonR4::HoughSegmentSeed* seed : assocInfo.assocSeeds) {
                    fillChamberInfo(chamber); 
                    fillTruthInfo(genParticlePtr, assocInfo.detectorHits, gctx);
                    auto& seedMatch = chamberLevelObjects.seedMatching[seed];
                    m_out_SP->push_back(*seedMatch.bucket);
                    fillSeedInfo(seed, seedMatch.matchProb);
                    if (seedMatch.segment) {
                        fillSegmentInfo(seedMatch.segment, seedMatch.matchProb);
                    }
                    if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
                }
            }
            // also fill the reco not matched to any truth 
            for (auto & [ seed, assocInfo ] : chamberLevelObjects.seedMatching) {
                if (assocInfo.truthParticle) continue;
                fillChamberInfo(chamber);
                m_out_SP->push_back(*assocInfo.bucket);
                fillSeedInfo(seed, 0.); 
                    if (assocInfo.segment) {
                    fillSegmentInfo(assocInfo.segment, 0.);
                }
                if (!m_tree.fill(ctx)) return StatusCode::FAILURE; 
            }
        } // end loop over chambers

        return StatusCode::SUCCESS;
    }

    StatusCode MuonHoughTransformTester::drawEventDisplay(const EventContext& ctx,
                                                          const std::vector<const xAOD::MuonSimHit*>& simHits,
                                                          const MuonR4::HoughSegmentSeed* foundMax) const {
        
        if (simHits.size() < 4) return StatusCode::SUCCESS;

        SG::ReadHandle<ActsGeometryContext> gctxHandle{m_geoCtxKey, ctx};
        ATH_CHECK(gctxHandle.isPresent());
        const ActsGeometryContext& gctx{*gctxHandle};

        auto readSpacePoints = SG::makeHandle(m_spacePointKey, ctx);  
        ATH_CHECK(readSpacePoints.isPresent()); 

        double zmin{1.e9}, zmax{-1.e9}, ymin{1.e9}, ymax{-1.e9}, xmin{1.e9}, xmax{-1.e9}; 
        std::vector<std::pair<double,double>> yzPos{}, yzDir{};
        std::vector<std::pair<double,double>> xzPos{}, xzDir{};

        const MuonGMR4::MuonChamber* refChamber = m_r4DetMgr->getChamber(simHits[0]->identify());
        
        for (const xAOD::MuonSimHit* thisHit: simHits){
            const Identifier thisID = thisHit->identify();
            const Amg::Transform3D toChamber = toChamberTrf(gctx, thisID);

            const Amg::Vector3D localPos{toChamber * xAOD::toEigen(thisHit->localPosition())};
            const Amg::Vector3D chamberDir = toChamber.linear() * xAOD::toEigen(thisHit->localDirection());

            yzPos.push_back(std::make_pair(localPos.y(), localPos.z())); 
            yzDir.push_back(std::make_pair(chamberDir.y(), chamberDir.z())); 
            xzPos.push_back(std::make_pair(localPos.x(), localPos.z())); 
            xzDir.push_back(std::make_pair(chamberDir.x(), chamberDir.z())); 
            zmin = std::min(localPos.z(), zmin);
            zmax = std::max(localPos.z(), zmax);
            ymin = std::min(localPos.y(), ymin);
            ymax = std::max(localPos.y(), ymax);
            xmin = std::min(localPos.x(), xmin);
            xmax = std::max(localPos.x(), xmax);
        }
                
        TCanvas myCanvas("can","can",800,600); 
        myCanvas.cd();

        double width = (ymax - ymin)*1.1;
        double height = (zmax - zmin)*1.1;
        if (height > width) width = height; 
        else height = width;

        double y0 = 0.5 * (ymax + ymin) - 0.5 * width;  
        double z0 = 0.5 * (zmax + zmin) - 0.5 * height;  
        double y1 = 0.5 * (ymax + ymin) + 0.5 * width;  
        double z1 = 0.5 * (zmax + zmin) + 0.5 * height;  
        auto frame = myCanvas.DrawFrame(y0,z0,y1,z1); 
        double frameWidth = frame->GetXaxis()->GetXmax() - frame->GetXaxis()->GetXmin(); 
        frame->GetXaxis()->SetTitle("y [mm]");
        frame->GetYaxis()->SetTitle("z [mm]");
 
        std::vector<std::unique_ptr<TObject>> primitives; 
        // archery with spacepoints
        for (size_t h = 0; h < yzPos.size(); ++h){
            auto  l = std::make_unique<TArrow>(yzPos.at(h).first, yzPos.at(h).second,yzPos.at(h).first + 0.3 * frameWidth * yzDir.at(h).first, yzPos.at(h).second + 0.3 * frameWidth * yzDir.at(h).second); 
            l->SetLineStyle(kDotted);
            l->Draw();
            primitives.emplace_back(std::move(l));
            auto m = std::make_unique<TMarker>(yzPos.at(h).first, yzPos.at(h).second,kFullDotLarge); 
            m->Draw(); 
            primitives.emplace_back(std::move(m));
        }
        if (foundMax) {
            auto  mrk = std::make_unique<TMarker>( foundMax->interceptY(), 0., kFullTriangleUp);
            mrk->SetMarkerSize(1);
            mrk->SetMarkerColor(kOrange-3); 
            mrk->Draw();
            primitives.emplace_back(std::move(mrk));
            auto trajectory = std::make_unique<TArrow>( foundMax->interceptY(), 0., foundMax->interceptY() +  0.3 * frameWidth * foundMax->tanTheta(), 0.3 * frameWidth);
            trajectory->SetLineColor(kOrange-3); 
            trajectory->Draw();
            primitives.push_back(std::move(trajectory));
        }
 
        // shapes with spacepoints
        for (auto spbucket : *readSpacePoints) {
            if (spbucket->muonChamber() != refChamber) continue;
            std::set<MuonR4::HoughHitType> hitsOnMax{};
            if (foundMax){
                hitsOnMax.insert(foundMax->getHitsInMax().begin(), foundMax->getHitsInMax().end()); 
            } 
            for (auto & hit : *spbucket){
                ATH_MSG_DEBUG( "         HIT @ "<<Amg::toString(hit->positionInChamber())<<"  "<<m_idHelperSvc->toString(hit->identify())<<" with r = "<<hit->driftRadius()); 
                /// Space point is a mdt space point
                // for circle making 
                double x, y;
                x = hit->positionInChamber().y();
                y = hit->positionInChamber().z();  // confusing i know
                double r = hit->driftRadius();
                // for box making 
                double bx1, bx2, by1, by2;
                bx1 = hit->positionInChamber().y() - 3*hit->uncertainty().y();
                bx2 = hit->positionInChamber().y() + 3*hit->uncertainty().y();
                by1 = hit->positionInChamber().z() - 0.01*height;
                by2 = hit->positionInChamber().z() + 0.01*height;
                bool isMaxHit = hitsOnMax.count(hit);
                if (r>1e-6) {
                    auto ell = getDriftCircleShape(x, y, r, isMaxHit);
                    ell->Draw();
                    primitives.emplace_back(std::move(ell));
                } else {
                    // RPC and TGC
                    if (hit->measuresPhi()) {
                        auto box = getBoxShape(bx1, bx2, by1, by2, hit->measuresPhi() && hit->measuresEta(), isMaxHit);
                        box->Draw();
                        primitives.emplace_back(std::move(box));
                    } else {
                        auto ell = getDriftCircleShape(x, y, r, isMaxHit);
                        ell->Draw();
                        primitives.emplace_back(std::move(ell));
                    }
                }
            }
        }
            
        std::stringstream legendLabel{};
        legendLabel<<"Evt "<<ctx.evt()<<" station: "<<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName());
        legendLabel<<"eta: "<<refChamber->stationEta()<<", phi: "<<refChamber->stationPhi();
        legendLabel<<", found maximum: "<<( foundMax ? "si" : "no");

        std::unique_ptr<TLatex> tl = getTLatex(legendLabel.str());
        tl->Draw();
        static std::atomic<unsigned int> pdfCounter{0};
        std::stringstream pdfName{};
        pdfName<<"HoughEvt_YZ_"<<ctx.evt()<<"_"<<(++pdfCounter)
                <<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName())<<"_"
                <<refChamber->stationEta()<<"_"<<refChamber->stationPhi()<<".pdf"; 
        
        myCanvas.SaveAs(pdfName.str().c_str());
        myCanvas.SaveAs(m_allCanName.value().c_str());
        primitives.clear();

        //////////////////////////////////////////  
        /// BEEG WALL OF COPYING STARTING HERE ///
        // XZ plot
        TCanvas xzCanvas("can","can",800,600); 
        xzCanvas.cd();

        width = (xmax - xmin)*1.1;
        if (height > width) width = height; 
        else height = width;

        double x0 = 0.5 * (xmax + xmin) - 0.5 * width;  
        double x1 = 0.5 * (xmax + xmin) + 0.5 * width;  
        auto xzframe = xzCanvas.DrawFrame(x0,z0,x1,z1); 
        frameWidth = xzframe->GetXaxis()->GetXmax() - frame->GetXaxis()->GetXmin(); 
        xzframe->GetXaxis()->SetTitle("x [mm]");
        xzframe->GetYaxis()->SetTitle("z [mm]");
 
        // archery with spacepoints
        for (size_t h = 0; h < xzPos.size(); ++h){
            auto  l = std::make_unique<TArrow>(xzPos.at(h).first, xzPos.at(h).second,xzPos.at(h).first + 0.3 * frameWidth * xzDir.at(h).first, xzPos.at(h).second + 0.3 * frameWidth * xzDir.at(h).second); 
            l->SetLineStyle(kDotted);
            l->Draw();
            primitives.emplace_back(std::move(l));
            auto m = std::make_unique<TMarker>(xzPos.at(h).first, xzPos.at(h).second,kFullDotLarge); 
            m->Draw(); 
            primitives.emplace_back(std::move(m));
        }
        if (foundMax) {
            auto  mrk = std::make_unique<TMarker>( foundMax->interceptX(), 0., kFullTriangleUp);
            mrk->SetMarkerSize(1);
            mrk->SetMarkerColor(kOrange-3); 
            mrk->Draw();
            primitives.emplace_back(std::move(mrk));
            auto trajectory = std::make_unique<TArrow>( foundMax->interceptX(), 0., foundMax->interceptX() +  0.3 * frameWidth * foundMax->tanTheta(), 0.3 * frameWidth);
            trajectory->SetLineColor(kOrange-3); 
            trajectory->Draw();
            primitives.push_back(std::move(trajectory));
        }
 
        // shapes with spacepoints
        for (auto spbucket : *readSpacePoints) {
            if (spbucket->muonChamber() != refChamber) continue;
            std::set<MuonR4::HoughHitType> hitsOnMax{};
            if (foundMax){
                hitsOnMax.insert(foundMax->getHitsInMax().begin(), foundMax->getHitsInMax().end()); 
            } 
            for (auto & hit : *spbucket){
                ATH_MSG_DEBUG( "         HIT @ "<<Amg::toString(hit->positionInChamber())<<"  "<<m_idHelperSvc->toString(hit->identify())<<" with r = "<<hit->driftRadius()); 
                /// Space point is a mdt space point
                // for circle making 
                double r = hit->driftRadius();
                // for box making 
                double bx1, bx2, by1, by2;
                bx1 = hit->positionInChamber().x() - 3*hit->uncertainty().x();
                bx2 = hit->positionInChamber().x() + 3*hit->uncertainty().x();
                by1 = hit->positionInChamber().z() - 0.01*height;
                by2 = hit->positionInChamber().z() + 0.01*height;
                bool isMaxHit = hitsOnMax.count(hit);
                // for xz plot, ignore MDT completely
                if (r<1e-6 && hit->measuresPhi()) {
                    auto box = getBoxShape(bx1, bx2, by1, by2, hit->measuresPhi() && hit->measuresEta(), isMaxHit);
                    box->Draw();
                    primitives.emplace_back(std::move(box));
                }
            }
        }
            
        std::unique_ptr<TLatex> tl2 = getTLatex(legendLabel.str());
        tl2->Draw();
        std::stringstream xzpdfName{};
        xzpdfName<<"HoughEvt_XZ_"<<ctx.evt()<<"_"<<(++pdfCounter)
                <<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName())<<"_"
                <<refChamber->stationEta()<<"_"<<refChamber->stationPhi()<<".pdf"; 
        
        xzCanvas.SaveAs(xzpdfName.str().c_str());
        xzCanvas.SaveAs(m_allCanName.value().c_str());
        return StatusCode::SUCCESS;
    }    
    
    StatusCode MuonHoughTransformTester::drawChi2(const EventContext& ctx,
                                const std::vector<const xAOD::MuonSimHit*>& simHits,
                                const MuonR4::HoughSegmentSeed* foundMax,
                                const MuonR4::MuonSegment* foundSegment,
                                const std::string & label, const ActsGeometryContext & gctx) const{
        
        if (simHits.size() < 4) return StatusCode::SUCCESS;
        std::vector<std::pair<double,double>> yzPos{}, yzDir{};

        const MuonGMR4::MuonChamber* refChamber = m_r4DetMgr->getChamber(simHits[0]->identify());

        double true_y0 = m_out_gen_y0.getVariable(); 
        double true_x0 = m_out_gen_x0.getVariable(); 
        double true_tanTheta = m_out_gen_tantheta.getVariable(); 
        double true_tanPhi = m_out_gen_tanphi.getVariable(); 

        TCanvas myCanvas("can","can",800,600); 
        myCanvas.cd();


        TH2D hChi2("chi2","chi2;y0;tan(theta)",200,true_y0 - 100,true_y0 + 100 , 200,true_tanTheta - 0.2,true_tanTheta + 0.2); 

        std::vector<double>pars(4,0.); 
        std::vector<double> chi2PerLayer(foundMax->getHitsInMax().size()); 
        pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::x0] = true_x0;
        pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::y0] = true_y0;
        pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::tanPhi] = true_tanPhi;
        pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::tanTheta] = true_tanTheta;
        for (int bx =1 ; bx < hChi2.GetXaxis()->GetNbins()+1; ++bx){
            for (int by =1 ; by < hChi2.GetYaxis()->GetNbins()+1; ++by){
                double y = hChi2.GetXaxis()->GetBinCenter(bx); 
                double t = hChi2.GetYaxis()->GetBinCenter(by); 
                pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::y0] = y;
                pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::tanTheta] = t;
                hChi2.Fill(y,t,MuonR4::SegmentFitHelpers::segmentChiSquare(pars.data(),foundMax->getHitsInMax(),chi2PerLayer, gctx, m_doBeamspotConstraint.value()));
            }
        }
        int mx,my,mz = 0;
        auto mbin = hChi2.GetMinimumBin(mx, my, mz); 
        double minChi2 = hChi2.GetBinContent(mbin); 
        double ygx2 = hChi2.GetXaxis()->GetBinCenter(mx); 
        double tgx2 = hChi2.GetYaxis()->GetBinCenter(my); 
        std::vector<double> contourLevels{minChi2 + 0.1,minChi2 + 1.0,minChi2 + 4.0,minChi2 + 9.0,minChi2 + 50.}; 
        myCanvas.SetLogz();
        hChi2.SetMinimum(1.e-1);
        hChi2.SetContour(100000);
        hChi2.GetXaxis()->SetTitle("y0[mm]");
        hChi2.GetYaxis()->SetTitle("tan #Theta");
        hChi2.Draw("COLZ");
        hChi2.SetStats(0);
        std::shared_ptr<TH2D> hClone (dynamic_cast<TH2D*>(hChi2.Clone("leeeeeak"))); 
        hClone->SetContour(5, contourLevels.data()); 
        hClone->SetFillStyle(0);
        hClone->Draw("CONT2SAME");

        TMarker mark(true_y0, true_tanTheta, kFullDotLarge);
        mark.SetMarkerColor(kAzure+10);
        mark.SetMarkerSize(1);
        mark.Draw();

        
        TMarker houghMin(foundMax->interceptY(), foundMax->tanTheta(), kOpenSquare);
        houghMin.SetMarkerColor(kOrange-3);
        houghMin.SetMarkerSize(1);
        houghMin.Draw();

        
        TMarker th2min(ygx2, tgx2, kOpenDiamond);
        th2min.SetMarkerColor(kGray+1);
        th2min.SetMarkerSize(1);
        th2min.Draw();
        TMarker gx2Min(-99999,-9999999, kFullDiamond);
        gx2Min.SetMarkerColor(kRed+1);
        gx2Min.SetMarkerSize(1);
        if (foundSegment){
            gx2Min.SetX(foundSegment->y0());
            gx2Min.SetY(foundSegment->tanTheta());
            gx2Min.Draw();
        }
        
        // adding a legend for all the dots we have! 
        TLegend leg(0.65, 0.38, 0.90, 0.15);
        leg.AddEntry(&mark, "True parameters", "P");
        leg.AddEntry(&houghMin, "Hough maximum", "P");
        leg.AddEntry(&th2min, "#chi2 TH2 minimum", "P");
        leg.AddEntry(&gx2Min, "#chi2 Minuit minimum", "P");
        leg.Draw();
            
        std::stringstream legendLabel{};
        legendLabel<<"Method "<<label<<" Evt "<<ctx.evt()<<" station: "<<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName());
        legendLabel<<"eta: "<<refChamber->stationEta()<<", phi: "<<refChamber->stationPhi();
        legendLabel<<", found maximum: "<<( foundMax ? "si" : "no");


        std::unique_ptr<TLatex> tl = getTLatex(legendLabel.str());
        tl->Draw();

        static std::atomic<unsigned int> pdfCounter{0};
        std::stringstream pdfName{};
        pdfName<<"Chi2_Eta_"<<label<<"_"<<ctx.evt()<<"_"<<(++pdfCounter)
                <<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName())<<"_"
                <<refChamber->stationEta()<<"_"<<refChamber->stationPhi()<<".pdf"; 
        
        myCanvas.SaveAs(pdfName.str().c_str());
        myCanvas.SaveAs(m_allCanName.value().c_str());
        // primitives.clear();
        hChi2.Reset();
        myCanvas.Clear();
        TH2D hChi3("chi3","chi2;x0;tan(phi)",500,true_x0 - 200,true_x0 + 200 , 500,true_tanPhi - 0.3,true_tanPhi + 0.3); 
        pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::x0] = true_x0;
        pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::y0] = true_y0;
        pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::tanPhi] = true_tanPhi;
        pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::tanTheta] = true_tanTheta;
        for (int bx =1 ; bx < hChi3.GetXaxis()->GetNbins()+1; ++bx){
            for (int by =1 ; by < hChi3.GetYaxis()->GetNbins()+1; ++by){
                double y = hChi3.GetXaxis()->GetBinCenter(bx); 
                double t = hChi3.GetYaxis()->GetBinCenter(by); 
                pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::x0] = y;
                pars[(int)MuonR4::MuonSegmentFitterEventData::parameterIndices::tanPhi] = t;
                hChi3.Fill(y,t,MuonR4::SegmentFitHelpers::segmentChiSquare(pars.data(),foundMax->getHitsInMax(),chi2PerLayer, gctx, m_doBeamspotConstraint.value()));
            }
        }
        mbin = hChi3.GetMinimumBin(mx, my, mz); 
        minChi2 = hChi3.GetBinContent(mbin); 
        ygx2 = hChi3.GetXaxis()->GetBinCenter(mx); 
        tgx2 = hChi3.GetYaxis()->GetBinCenter(my); 
        myCanvas.SetLogz();
        hChi3.SetMinimum(1.e-1);
        hChi3.GetXaxis()->SetTitle("x0[mm]");
        hChi3.GetYaxis()->SetTitle("tan #Phi");
        hChi3.SetContour(100000);
        hChi3.SetStats(0);
        hChi3.Draw("COLZ");
        hClone.reset (dynamic_cast<TH2D*>(hChi3.Clone("leeeeeak"))); 
        hClone->SetContour(5, contourLevels.data()); 
        hClone->SetFillStyle(0);
        hClone->Draw("CONT2SAME");

        TMarker mark2(true_x0, true_tanPhi, kFullDotLarge);
        mark2.SetMarkerColor(kAzure+10);
        mark2.SetMarkerSize(1);
        mark2.Draw();

        
        TMarker houghMin2(foundMax->interceptX(), foundMax->tanPhi(), kOpenSquare);
        houghMin2.SetMarkerColor(kOrange-3);
        houghMin2.SetMarkerSize(1);
        houghMin2.Draw();

        
        TMarker th2min2(ygx2, tgx2, kOpenDiamond);
        th2min2.SetMarkerColor(kGray+1);
        th2min2.SetMarkerSize(1);
        th2min2.Draw();
        TMarker gx2Min2(-99999,-9999999, kFullDiamond);
        gx2Min2.SetMarkerColor(kRed+1);
        gx2Min2.SetMarkerSize(1);
        if (foundSegment){
            gx2Min2.SetX(foundSegment->x0());
            gx2Min2.SetY(foundSegment->tanPhi());
            gx2Min2.Draw();
        }
            
        // adding a legend for all the dots we have! 
        TLegend leg2(0.65, 0.38, 0.90, 0.15);
        leg.AddEntry(&mark, "True parameters", "P");
        leg2.AddEntry(&houghMin, "Hough maximum", "P");
        leg2.AddEntry(&th2min, "#chi2 TH2 minimum", "P");
        leg2.AddEntry(&gx2Min, "#chi2 Minuit minimum", "P");
        leg2.Draw();

        std::stringstream legendLabel2{};
        legendLabel2<<"Method "<<label<<" Evt "<<ctx.evt()<<" station: "<<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName());
        legendLabel2<<"eta: "<<refChamber->stationEta()<<", phi: "<<refChamber->stationPhi();
        legendLabel2<<", found maximum: "<<( foundMax ? "si" : "no");


        std::unique_ptr<TLatex> tl2 = getTLatex(legendLabel2.str());
        tl2->SetNDC();
        tl2->SetTextFont(53); 
        tl2->SetTextSize(18); 
        tl2->Draw();
        std::stringstream pdfName2{};
        pdfName2<<"Chi2_Phi_"<<label<<"_"<<ctx.evt()<<"_"<<(++pdfCounter)
                <<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName())<<"_"
                <<refChamber->stationEta()<<"_"<<refChamber->stationPhi()<<".pdf"; 
        
        myCanvas.SaveAs(pdfName2.str().c_str());
        myCanvas.SaveAs(m_allCanName.value().c_str());


        return StatusCode::SUCCESS;

    }

}  // namespace MuonValR4
