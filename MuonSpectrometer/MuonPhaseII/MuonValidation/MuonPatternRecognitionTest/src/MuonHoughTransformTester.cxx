/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughTransformTester.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "StoreGate/ReadCondHandle.h"
#include "GeoModelHelpers/throwExcept.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TArrow.h"
#include "TMarker.h"
#include "TEllipse.h"
#include "TLatex.h"



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

    StatusCode MuonHoughTransformTester::execute()  {
        
        const EventContext & ctx = Gaudi::Hive::currentContext();
        SG::ReadHandle<ActsGeometryContext> gctxHandle{m_geoCtxKey, ctx};
        ATH_CHECK(gctxHandle.isPresent());
        const ActsGeometryContext& gctx{*gctxHandle};

        // retrieve the two input collections

        auto simHitCollections = m_inSimHitKeys.makeHandles(ctx);
        
        SG::ReadHandle<MuonR4::StationHoughSegmentSeedContainer> readSegmentSeeds(m_inHoughSegmentSeedKey, ctx);
        ATH_CHECK(readSegmentSeeds.isPresent());        
        
        SG::ReadHandle<MuonR4::MuonSegmentContainer> readMuonSegments(m_inSegmentKey, ctx);
        ATH_CHECK(readMuonSegments.isPresent());        

        ATH_MSG_DEBUG("Succesfully retrieved input collections");

        std::set<const MuonR4::HoughSegmentSeed*> matchedSeeds{};
        // map the drift circles to identifiers. 
        // The fast digi should only generate one circle per tube. 
        std::map<std::pair<const MuonGMR4::MuonChamber*, HepMC::ConstGenParticlePtr>, std::vector<const xAOD::MuonSimHit*>> simHitMap{};
        for (auto & collection : simHitCollections){
            ATH_CHECK(collection.isPresent());
            for (const xAOD::MuonSimHit* simHit : *collection) {
                const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(simHit->identify()); 
                const MuonGMR4::MuonChamber* id{reElement->getChamber()}; 
                auto genLink = simHit->genParticleLink();
                HepMC::ConstGenParticlePtr genParticle = nullptr; 
                if (genLink.isValid()){
                    genParticle = genLink.cptr(); 
                }
                /// skip empty truth matches for now
                if (genParticle == nullptr) continue;
                simHitMap[std::make_pair(id,genParticle)].push_back(simHit);
            }
        }
        
        std::map<const MuonGMR4::MuonChamber*, std::vector<MuonR4::HoughSegmentSeed>> houghPeakMap{};
        for (const MuonR4::StationHoughSegmentSeeds  & max : *readSegmentSeeds){
            houghPeakMap.emplace(max.chamber(),max.getMaxima());
        }   

        for (auto & [stationAndParticle, hits] : simHitMap){
            HepMC::ConstGenParticlePtr genParticlePtr = stationAndParticle.second;
            const xAOD::MuonSimHit* simHit = hits.front();
            const Identifier ID = simHit->identify();
            const MuonGMR4::MuonChamber* chamber = m_r4DetMgr->getReadoutElement(ID)->getChamber();
                       
            const Amg::Transform3D toChamber{toChamberTrf(gctx, ID)};
            const Amg::Vector3D localPos{toChamber * xAOD::toEigen(simHit->localPosition())};
            Amg::Vector3D chamberDir = toChamber.linear() * xAOD::toEigen(simHit->localDirection());
            
            /// Express the simulated hit in the center of the chamber
            const std::optional<double> lambda = Amg::intersect<3>(localPos, chamberDir, Amg::Vector3D::UnitZ(), 0.);
            Amg::Vector3D chamberPos = localPos + (*lambda)*chamberDir;

            m_out_stationName = chamber->stationName();
            m_out_stationEta = chamber->stationEta();
            m_out_stationPhi = chamber->stationPhi();
            /// Global coordinates
            m_out_gen_Eta   = genParticlePtr->momentum().eta();
            m_out_gen_Phi= genParticlePtr->momentum().phi();
            m_out_gen_Pt= genParticlePtr->momentum().perp();
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
            m_out_gen_z0 = chamberPos.y(); 
            m_out_gen_x0 = chamberPos.x(); 

            const std::vector<MuonR4::HoughSegmentSeed>& houghMaxima = houghPeakMap[chamber];
            if (houghMaxima.empty()){
                if (!m_tree.fill(ctx)) {
                    return StatusCode::FAILURE;
                }
                if (m_drawEvtDisplayFailure) {
                    ATH_CHECK(drawEventDisplay(ctx, hits, nullptr));
                }
                continue;
            }
            const MuonR4::HoughSegmentSeed* foundMax = nullptr; 
            // find the best hough maximum
            size_t max_hits{0}, max_etaHits{0}, max_phiHits{0};
            for (const MuonR4::HoughSegmentSeed & max : houghMaxima){                
                size_t nFound{0}, nEta{0}, nPhi{0}; 
                for (const xAOD::MuonSimHit* simHit : hits) {

                    for (const MuonR4::HoughHitType & hitOnMax : max.getHitsInMax()) {
                        if(m_idHelperSvc->isMdt(simHit->identify()) &&
                            hitOnMax->identify() == simHit->identify()){
                            ++nFound;
                            break;
                        } 
                        //// Sim hits are expressed w.r.t to the gas gap Id. Check whether
                        ///  the hit is in the same gas gap
                        else if (m_idHelperSvc->gasGapId(hitOnMax->identify()) == simHit->identify()) {
                            ++nFound;
                            if (hitOnMax->measuresEta()) ++nEta; 
                            if (hitOnMax->measuresPhi()) ++nPhi; 
                        }
                    }
                }
                if (nFound > max_hits){
                    max_hits = nFound;
                    max_etaHits = nEta;
                    max_phiHits = nPhi; 
                    foundMax = &max; 
                }
            }
            /// Maximum could be associated to the hit
            if (foundMax != nullptr){
                matchedSeeds.insert(foundMax);
                fillMaximum(foundMax);
                m_out_max_nHits = max_hits; 
                m_out_max_nEtaHits = max_etaHits; 
                m_out_max_nPhiHits = max_phiHits; 

                bool foundSegment = false; 
                const MuonR4::MuonSegment* theSegment = nullptr; 
                max_hits =  max_etaHits = max_phiHits; 
                for (const MuonR4::MuonSegment & segment : *readMuonSegments){             
                    size_t nFound{0}, nEta{0}, nPhi{0}; 
                    for (const xAOD::MuonSimHit* simHit : hits) {
                        for (const xAOD::UncalibratedMeasurement* hitOnMax : segment.measurements()) {
                            if(hitOnMax->type() == xAOD::UncalibMeasType::MdtDriftCircleType &&
                                hitOnMax->identifier() == simHit->identify().get_compact()){
                                ++nFound;
                                break;
                            } 
                            //// Sim hits are expressed w.r.t to the gas gap Id. Check whether
                            ///  the hit is in the same gas gap
                            else if (m_idHelperSvc->gasGapId(Identifier(Identifier::value_type(hitOnMax->identifier()))) == simHit->identify()) {
                                ++nFound;
                                if (m_idHelperSvc->measuresPhi(Identifier(Identifier::value_type(hitOnMax->identifier())))) ++nPhi;
                                else ++nEta; 
                            }
                        }
                    }
                    if (nFound > max_hits){
                        max_hits = nFound;
                        max_etaHits = nEta;
                        max_phiHits = nPhi; 
                        foundSegment = true;
                        theSegment = &segment; 
                    }
                }
                if (foundSegment){
                    m_out_hasSegment = true; 
                    m_out_segment_chi2 = theSegment->chi2();
                    for (auto & c2 : theSegment->chi2PerMeasurement()){
                        m_out_segment_chi2_measurement.push_back(c2); 
                    }
                    m_out_segment_tanphi = theSegment->tanPhi();
                    m_out_segment_tantheta = theSegment->tanTheta();
                    m_out_segment_z0 = theSegment->y0();
                    m_out_segment_x0 = theSegment->x0();
                }

                if (m_drawEvtDisplaySuccess) {
                    ATH_CHECK(drawEventDisplay(ctx, hits, foundMax));
                    ATH_CHECK(drawChi2(ctx, hits, foundMax, theSegment ,"ValidMin"));
                   

                }
            } else if (m_drawEvtDisplayFailure) {
                ATH_CHECK(drawEventDisplay(ctx, hits, nullptr));          
            }
            if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
        }
        ATH_CHECK(dumpUnMatched(ctx, *readSegmentSeeds, matchedSeeds));
        return StatusCode::SUCCESS;
    }
    void MuonHoughTransformTester::fillMaximum(const MuonR4::HoughSegmentSeed* foundMax) {
        if (!foundMax) return;
        m_out_hasMax = true; 
        m_out_max_hasPhiExtension = foundMax->hasPhiExtension(); 
        m_out_max_tantheta = foundMax->tanTheta();
        m_out_max_z0 = foundMax->interceptY();
        if (m_out_max_hasPhiExtension.getVariable()){
            m_out_max_tanphi = foundMax->tanPhi();
            m_out_max_x0 = foundMax->interceptX(); 
        }
        unsigned int nMdtMax{0}, nRpcMax{0}, nTgcMax{0}, nMmMax{0}, nsTgcMax{0}; 
        for (const MuonR4::HoughHitType & houghSP: foundMax->getHitsInMax()){
            /// Skip all space points that don' contain any phi measurement
            
            const xAOD::UncalibratedMeasurement* meas = houghSP->primaryMeasurement();
            switch (meas->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: 
                    m_max_driftCircleId.push_back(houghSP->identify());
                    m_max_driftCircleTubePos.push_back(houghSP->positionInChamber());
                    m_max_driftCirclRadius.push_back(houghSP->driftRadius());
                    m_max_driftCircleDriftUncert.push_back(houghSP->uncertainty()[0]);
                    m_max_driftCircleTubeLength.push_back(houghSP->uncertainty()[1]);
                        ++nMdtMax;
                    break;
                case xAOD::UncalibMeasType::RpcStripType:
                    m_max_rpcHitId.push_back(houghSP->identify());
                    m_max_rpcHitPos.push_back(houghSP->positionInChamber());
                    m_max_rpcHitHasPhiMeas.push_back(houghSP->measuresPhi());
                    m_max_rpcHitErrorX.push_back(houghSP->uncertainty()[0]);
                    m_max_rpcHitErrorY.push_back(houghSP->uncertainty()[1]);
                    ++nRpcMax;
                    break;
                case xAOD::UncalibMeasType::TgcStripType:
                    m_max_tgcHitId.push_back(houghSP->identify());
                    m_max_tgcHitPos.push_back(houghSP->positionInChamber());
                    m_max_tgcHitHasPhiMeas.push_back(houghSP->measuresPhi());
                    m_max_tgcHitErrorX.push_back(houghSP->uncertainty()[0]);
                    m_max_tgcHitErrorY.push_back(houghSP->uncertainty()[1]);
                    ++nTgcMax;
                    break;
                case xAOD::UncalibMeasType::sTgcStripType:
                    m_max_stgcHitId.push_back(houghSP->identify());
                    m_max_stgcHitPos.push_back(houghSP->positionInChamber());
                    m_max_stgcHitHasPhiMeas.push_back(houghSP->measuresPhi());
                    m_max_stgcHitErrorX.push_back(houghSP->uncertainty()[0]);
                    m_max_stgcHitErrorY.push_back(houghSP->uncertainty()[1]);
                    ++nsTgcMax;
                    break;
                case xAOD::UncalibMeasType::MMClusterType:
                    m_max_MmHitId.push_back(houghSP->identify());
                    m_max_MmHitPos.push_back(houghSP->positionInChamber());
                    m_max_MmHitIsStero.push_back(m_idHelperSvc->mmIdHelper().isStereo(houghSP->identify()));
                    m_max_MmHitErrorX.push_back(houghSP->uncertainty()[0]);
                    m_max_MmHitErrorY.push_back(houghSP->uncertainty()[1]);
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
    StatusCode MuonHoughTransformTester::dumpUnMatched(const EventContext& ctx,
                                                       const MuonR4::StationHoughSegmentSeedContainer& seedContainer,
                                                       const std::set<const MuonR4::HoughSegmentSeed*>& matchedSeeds) {
        if (!m_dumpUnmatchedSeeds){
            return StatusCode::SUCCESS;
        }
        for (const auto& maxInStation : seedContainer) {            
            
            const MuonGMR4::MuonChamber* chamber = maxInStation.chamber();
            for (const MuonR4::HoughSegmentSeed& dumpMeMayBe : maxInStation.getMaxima()) {
               if (matchedSeeds.count(&dumpMeMayBe)) {
                  ATH_MSG_VERBOSE("Do not dump the seed twice");
                  continue;
               }
               fillMaximum(&dumpMeMayBe);
               m_out_gen_Eta = -666;
               m_out_gen_Phi = -666;
               m_out_gen_Pt = -1;
               m_out_gen_nHits = 0;
               m_out_stationName = chamber->stationName();
               m_out_stationEta = chamber->stationEta();
               m_out_stationPhi = chamber->stationPhi();

               if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
            }
        }
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

        double zmin{1.e9}, zmax{-1.e9}, ymin{1.e9}, ymax{-1.e9}; 
        std::vector<std::pair<double,double>> shPos{}, shDir{};

        const MuonGMR4::MuonChamber* refChamber = m_r4DetMgr->getChamber(simHits[0]->identify());
        
        for (const xAOD::MuonSimHit* thisHit: simHits){
            const Identifier thisID = thisHit->identify();
            const Amg::Transform3D toChamber = toChamberTrf(gctx, thisID);

            const Amg::Vector3D localPos{toChamber * xAOD::toEigen(thisHit->localPosition())};
            const Amg::Vector3D chamberDir = toChamber.linear() * xAOD::toEigen(thisHit->localDirection());

            shPos.push_back(std::make_pair(localPos.y(), localPos.z())); 
            shDir.push_back(std::make_pair(chamberDir.y(), chamberDir.z())); 
            zmin = std::min(localPos.z(), zmin);
            zmax = std::max(localPos.z(), zmax);
            ymin = std::min(localPos.y(), ymin);
            ymax = std::max(localPos.y(), ymax);
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
                
        std::vector<std::unique_ptr<TObject>> primitives; 
        for (size_t h = 0; h < shPos.size(); ++h){
            auto  l = std::make_unique<TArrow>(shPos.at(h).first, shPos.at(h).second,shPos.at(h).first + 0.3 * frameWidth * shDir.at(h).first, shPos.at(h).second + 0.3 * frameWidth * shDir.at(h).second); 
            l->SetLineStyle(kDotted);
            l->Draw();
            primitives.emplace_back(std::move(l));
            auto m = std::make_unique<TMarker>(shPos.at(h).first, shPos.at(h).second,kFullDotLarge); 
            m->Draw(); 
            primitives.emplace_back(std::move(m));
        }
        
        for (auto spbucket : *readSpacePoints) {            
            if (spbucket->muonChamber() != refChamber) continue;
            std::set<MuonR4::HoughHitType> hitsOnMax{};
            if (foundMax){
                hitsOnMax.insert(foundMax->getHitsInMax().begin(), foundMax->getHitsInMax().end()); 
            } 
            for (auto & hit : *spbucket){
                ATH_MSG_DEBUG( "         HIT @ "<<Amg::toString(hit->positionInChamber())<<"  "<<m_idHelperSvc->toString(hit->identify())<<" with r = "<<hit->driftRadius()); 
                /// Space point is a mdt space point
                if (hit->driftRadius() > 1e-6) {
                    auto  ell = std::make_unique<TEllipse>(hit->positionInChamber().y(), 
                                                          hit->positionInChamber().z(),
                                                          hit->driftRadius());
                    ell->SetLineColor(kRed);
                    ell->SetFillColor(kRed);
                    if (hitsOnMax.count(hit)) {
                        ell->SetFillStyle(1001);
                        ell->SetFillColorAlpha(ell->GetFillColor(), 0.8); 
                    }else{
                        ell->SetFillStyle(0);
                    }
                    ell->Draw(); 
                    primitives.emplace_back(std::move(ell));
                } else {
                    auto  m = std::make_unique<TEllipse>(hit->positionInChamber().y(), 
                                                         hit->positionInChamber().z(), 
                                                         hit->uncertainty().y());
                    m->SetLineColor(kBlue); 
                    m->SetFillColor(kBlue); 
                    m->SetFillStyle(0);
                    
                    if (hit->measuresPhi() && hit->measuresEta()) {
                        m->SetLineColor(kViolet); 
                        m->SetFillColor(kViolet);
                    } else if (hit->measuresPhi())  {
                        m->SetLineColor(kGreen); 
                        m->SetFillColor(kGreen);
                    }
                    /// Fill the ellipse if the hit is on the hough maximum
                    if (hitsOnMax.count(hit)) {
                        m->SetFillStyle(1001);
                        m->SetFillColorAlpha(m->GetFillColor(), 0.8); 
                    }
                    m->Draw();
                    primitives.emplace_back(std::move(m));
                }
            }
        }
            
        std::stringstream legendLabel{};
        legendLabel<<"Evt "<<ctx.evt()<<" station: "<<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName());
        legendLabel<<"eta: "<<refChamber->stationEta()<<", phi: "<<refChamber->stationPhi();
        legendLabel<<", found maximum: "<<( foundMax ? "si" : "no");


        TLatex tl( 0.15,0.8,legendLabel.str().c_str());
        tl.SetNDC();
        tl.SetTextFont(53); 
        tl.SetTextSize(18); 
        tl.Draw();
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
        static std::atomic<unsigned int> pdfCounter{0};
        std::stringstream pdfName{};
        pdfName<<"HoughEvt_"<<ctx.evt()<<"_"<<(++pdfCounter)
                <<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName())<<"_"
                <<refChamber->stationEta()<<"_"<<refChamber->stationPhi()<<".pdf"; 
        
        myCanvas.SaveAs(pdfName.str().c_str());
        myCanvas.SaveAs(m_allCanName.value().c_str());
        primitives.clear();
        return StatusCode::SUCCESS;
    }    
    
    double evalX2(const double* pars,  std::function<double(double, double, const ActsGeometryContext &, const std::vector<const xAOD::MuonSimHit*>&, const MuonR4::HoughSegmentSeed*)> fcn, const ActsGeometryContext & gctx, const std::vector<const xAOD::MuonSimHit*>& simHits,  const MuonR4::HoughSegmentSeed* seed){
        return fcn(pars[0],pars[1],gctx,simHits,seed);
    }
    #define XXX std::cout << __LINE__ << std::endl; 
    StatusCode MuonHoughTransformTester::drawChi2(const EventContext& ctx,
                                const std::vector<const xAOD::MuonSimHit*>& simHits,
                                const MuonR4::HoughSegmentSeed* foundMax,
                                const MuonR4::MuonSegment* foundSegment,
                                const std::string & label) const{
        
        if (simHits.size() < 4) return StatusCode::SUCCESS;
        std::vector<std::pair<double,double>> shPos{}, shDir{};

        const MuonGMR4::MuonChamber* refChamber = m_r4DetMgr->getChamber(simHits[0]->identify());
        std::cout <<" ref chamber = "<<refChamber<<std::endl;

        double true_y0 = m_out_gen_z0.getVariable(); 
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
                hChi2.Fill(y,t,MuonR4::SegmentFitHelpers::segmentChiSquare(pars.data(),foundMax->getHitsInMax(),chi2PerLayer));
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
            
        std::stringstream legendLabel{};
        legendLabel<<"Method "<<label<<" Evt "<<ctx.evt()<<" station: "<<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName());
        legendLabel<<"eta: "<<refChamber->stationEta()<<", phi: "<<refChamber->stationPhi();
        legendLabel<<", found maximum: "<<( foundMax ? "si" : "no");


        TLatex tl( 0.15,0.8,legendLabel.str().c_str());
        tl.SetNDC();
        tl.SetTextFont(53); 
        tl.SetTextSize(18); 
        tl.Draw();
        static std::atomic<unsigned int> pdfCounter{0};
        std::stringstream pdfName{};
        pdfName<<"Chi2_Eta_"<<label<<"_"<<ctx.evt()<<"_"<<(++pdfCounter)
                <<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName())<<"_"
                <<refChamber->stationEta()<<"_"<<refChamber->stationPhi()<<".pdf"; 
        
        std::cout << "try to save "<<pdfName.str()<<", this might go poof"<<std::endl;
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
                hChi3.Fill(y,t,MuonR4::SegmentFitHelpers::segmentChiSquare(pars.data(),foundMax->getHitsInMax(),chi2PerLayer));
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
            
        std::stringstream legendLabel2{};
        legendLabel2<<"Method "<<label<<" Evt "<<ctx.evt()<<" station: "<<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName());
        legendLabel2<<"eta: "<<refChamber->stationEta()<<", phi: "<<refChamber->stationPhi();
        legendLabel2<<", found maximum: "<<( foundMax ? "si" : "no");


        TLatex tl2( 0.15,0.8,legendLabel2.str().c_str());
        tl2.SetNDC();
        tl2.SetTextFont(53); 
        tl2.SetTextSize(18); 
        tl2.Draw();
        std::stringstream pdfName2{};
        pdfName2<<"Chi2_Phi_"<<label<<"_"<<ctx.evt()<<"_"<<(++pdfCounter)
                <<m_idHelperSvc->mdtIdHelper().stationNameString(refChamber->stationName())<<"_"
                <<refChamber->stationEta()<<"_"<<refChamber->stationPhi()<<".pdf"; 
        
        std::cout << "try to save "<<pdfName2.str()<<", this might go poof"<<std::endl;
        myCanvas.SaveAs(pdfName2.str().c_str());
        myCanvas.SaveAs(m_allCanName.value().c_str());


        return StatusCode::SUCCESS;

    }

}  // namespace MuonValR4
