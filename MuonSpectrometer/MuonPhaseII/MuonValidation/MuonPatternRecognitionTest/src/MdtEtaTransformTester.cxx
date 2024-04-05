/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtEtaTransformTester.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"
#include "StoreGate/ReadCondHandle.h"
#include "GeoModelHelpers/throwExcept.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TArrow.h"
#include "TMarker.h"
#include "TEllipse.h"
#include "TLatex.h"



namespace MuonValR4 {
    


    MdtEtaTransformTester::MdtEtaTransformTester(const std::string& name,
                                ISvcLocator* pSvcLocator)
        : AthHistogramAlgorithm(name, pSvcLocator) {}


    StatusCode MdtEtaTransformTester::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_inSimHitKeys.initialize());
        ATH_CHECK(m_inHoughMaximaKey.initialize());
        ATH_CHECK(m_spacePointKey.initialize());
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

    StatusCode MdtEtaTransformTester::finalize() {
        ATH_CHECK(m_tree.write());
        if (m_allCan) m_allCan->SaveAs(Form("%s]", m_allCanName.value().c_str()));
        return StatusCode::SUCCESS;
    }

    Amg::Transform3D MdtEtaTransformTester::toChamberTrf(const ActsGeometryContext& gctx,
                                                         const Identifier& hitId) const {
        const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(hitId); 
        //transform from local (w.r.t tube's frame) to global (ATLAS frame) and then to chamber's frame
        const MuonGMR4::MuonChamber* muonChamber = reElement->getChamber();
        /// Mdt tubes have all their own transform while for the strip detectors there's one transform per layer
        const IdentifierHash trfHash = reElement->detectorType() == ActsTrk::DetectorType::Mdt ?
                                       reElement->measurementHash(hitId) : reElement->layerHash(hitId);            
        return muonChamber->globalToLocalTrans(gctx) * reElement->localToGlobalTrans(gctx, trfHash);
    }

    StatusCode MdtEtaTransformTester::execute()  {
        
        const EventContext & ctx = Gaudi::Hive::currentContext();
        SG::ReadHandle<ActsGeometryContext> gctxHandle{m_geoCtxKey, ctx};
        ATH_CHECK(gctxHandle.isPresent());
        const ActsGeometryContext& gctx{*gctxHandle};

        // retrieve the two input collections

        auto simHitCollections = m_inSimHitKeys.makeHandles(ctx);
        
        SG::ReadHandle<MuonR4::StationHoughMaxContainer> readHoughPeaks(m_inHoughMaximaKey, ctx);
        ATH_CHECK(readHoughPeaks.isPresent());        

        ATH_MSG_DEBUG("Succesfully retrieved input collections");

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
        
        std::map<const MuonGMR4::MuonChamber*, std::vector<MuonR4::HoughMaximum>> houghPeakMap{};
        for (const MuonR4::StationHoughMaxima  & max : *readHoughPeaks){
            houghPeakMap.emplace(max.chamber(),max.getMaxima());
        }   

        for (auto & [stationAndParticle, hits] : simHitMap){
            HepMC::ConstGenParticlePtr genParticlePtr = stationAndParticle.second;
            const xAOD::MuonSimHit* simHit = hits.front();
            const Identifier ID = simHit->identify();
            const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(ID);
                       
            const Amg::Transform3D toChamber{toChamberTrf(gctx, ID)};
            const Amg::Vector3D localPos{toChamber * xAOD::toEigen(simHit->localPosition())};
            Amg::Vector3D chamberDir = toChamber.linear() * xAOD::toEigen(simHit->localDirection());
            
            /// Express the simulated hit in the center of the chamber
            const std::optional<double> lambda = Amg::intersect<3>(localPos, chamberDir, Amg::Vector3D::UnitZ(), 0.);
            Amg::Vector3D chamberPos = localPos + (*lambda)*chamberDir;

            m_evtNumber = ctx.eventID().event_number();
            m_out_stationName = reElement->stationName();
            m_out_stationEta = reElement->stationEta();
            m_out_stationPhi = reElement->stationPhi();
            /// Global coordinates
            m_out_gen_Eta   = genParticlePtr->momentum().eta();
            m_out_gen_Phi= genParticlePtr->momentum().phi();
            m_out_gen_Pt= genParticlePtr->momentum().perp();
            m_out_gen_nHits = hits.size(); 
            unsigned int nMdt{0}, nRpc{0}, nTgc{0}; 
            for (const xAOD::MuonSimHit* hit : hits){
                nMdt += m_idHelperSvc->isMdt(hit->identify()); 
                nRpc += m_idHelperSvc->isRpc(hit->identify()); 
                nTgc += m_idHelperSvc->isTgc(hit->identify()); 
            }
            m_out_gen_nRPCHits = nRpc; 
            m_out_gen_nMDTHits = nMdt; 
            m_out_gen_nTGCHits = nTgc;

            m_out_gen_tantheta = (std::abs(chamberDir.z()) > 1.e-8 ? chamberDir.y()/chamberDir.z() : 1.e10); 
            m_out_gen_z0 = chamberPos.y(); 

            const std::vector<MuonR4::HoughMaximum>& houghMaxima = houghPeakMap[reElement->getChamber()];
            if (houghMaxima.empty()){
                if (!m_tree.fill(ctx)) {
                    return StatusCode::FAILURE;
                }
                if (m_drawEvtDisplayFailure) {
                    ATH_CHECK(drawEventDisplay(ctx, hits, nullptr));
                }
                continue;
            }
            const MuonR4::HoughMaximum* foundMax = nullptr; 
            // find the best hough maximum
            size_t max_hits{0};
            for (const MuonR4::HoughMaximum & max : houghMaxima){                
                size_t nFound{0}; 
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
                        }
                    }
                }
                if (nFound > max_hits){
                    max_hits = nFound;
                    foundMax = &max; 
                }
            }
            /// Maximum could be associated to the hit
            if (foundMax != nullptr){
                m_out_hasMax = true; 
                m_out_max_tantheta = foundMax->getX();
                m_out_max_z0 = foundMax->getY();
                m_out_max_nHits = max_hits; 
                unsigned int nMdt{0}, nRpc{0}, nTgc{0}; 
                for (const MuonR4::HoughHitType & houghSP: foundMax->getHitsInMax()){
                    /// Skip all space points that don' contain any phi measurement
                    if (!houghSP->measuresEta()) continue;
                    
                    const xAOD::UncalibratedMeasurement* meas = houghSP->primaryMeasurement();
                    switch (meas->type()) {
                        case xAOD::UncalibMeasType::MdtDriftCircleType: 
                            m_max_driftCircleId.push_back(houghSP->identify());
                            m_max_driftCircleTubePos.push_back(houghSP->positionInChamber());
                            m_max_driftCirclRadius.push_back(houghSP->driftRadius());
                            m_max_driftCircleDriftUncert.push_back(houghSP->uncertainty()[0]);
                            m_max_driftCircleTubeLength.push_back(houghSP->uncertainty()[1]);
                             ++nMdt;
                            break;
                        case xAOD::UncalibMeasType::RpcStripType:
                            m_max_rpcHitId.push_back(houghSP->identify());
                            m_max_rpcHitPos.push_back(houghSP->positionInChamber());
                            m_max_rpcHitHasPhiMeas.push_back(houghSP->measuresPhi());
                            m_max_rpcHitErrorX.push_back(houghSP->uncertainty()[0]);
                            m_max_rpcHitErrorY.push_back(houghSP->uncertainty()[1]);
                            ++nRpc;
                            break;
                        case xAOD::UncalibMeasType::TgcStripType:
                            m_max_tgcHitId.push_back(houghSP->identify());
                            m_max_tgcHitPos.push_back(houghSP->positionInChamber());
                            m_max_tgcHitHasPhiMeas.push_back(houghSP->measuresPhi());
                            m_max_tgcHitErrorX.push_back(houghSP->uncertainty()[0]);
                            m_max_tgcHitErrorY.push_back(houghSP->uncertainty()[1]);
                            ++nTgc;
                            break;
                        default:
                            ATH_MSG_WARNING("Technology "<<m_idHelperSvc->toString(houghSP->identify())
                                        <<" not yet implemented");                        
                    }                    
                }
                m_out_max_nMdt = nMdt;
                m_out_max_nRpc = nRpc;
                m_out_max_nTgc = nTgc;
                if (m_drawEvtDisplaySuccess) {
                    ATH_CHECK(drawEventDisplay(ctx, hits, foundMax));
                }
            } else if (m_drawEvtDisplayFailure) {
                ATH_CHECK(drawEventDisplay(ctx, hits, nullptr));          
            }
            if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MdtEtaTransformTester::drawEventDisplay(const EventContext& ctx,
                                                       const std::vector<const xAOD::MuonSimHit*>& simHits,
                                                       const MuonR4::HoughMaximum* foundMax) const {
        
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
            auto  mrk = std::make_unique<TMarker>( foundMax->getY(), 0., kFullTriangleUp);
            mrk->SetMarkerSize(1);
            mrk->SetMarkerColor(kOrange-3); 
            mrk->Draw();
            primitives.emplace_back(std::move(mrk));
            auto trajectory = std::make_unique<TArrow>( foundMax->getY(), 0., foundMax->getY() +  0.3 * frameWidth * foundMax->getX(), 0.3 * frameWidth);
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

}  // namespace MuonValR4
