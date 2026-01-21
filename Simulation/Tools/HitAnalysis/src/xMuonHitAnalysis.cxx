
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xMuonHitAnalysis.h"

#include "StoreGate/ReadHandle.h"
#include "xAODMuonViews/ChamberViewer.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include <format>
#include <TH1F.h>
#include <TH2F.h>
namespace MuonValR4{
    xMuonHitAnalysis::HistoSet::HistoSet(const Muon::IMuonIdHelperSvc* idHelperSvc,
                                         const ServiceHandle<ITHistSvc>& histSvc,
                                         const std::string& basePath,
                                         const TechIdx_t techIdx,
                                        const int stIdent):
        identifier{stIdent} {
        
        auto registerHisto = [&](TH1* h) {
            h->Sumw2();
            h->StatOverflows();
            
            std::string stName = "Inclusive";
            if (stIdent >= 0) {
                switch (techIdx) {
                    case TechIdx_t::MDT:
                    case TechIdx_t::RPC: {
                        stName = Muon::MuonStationIndex::stName(static_cast<StIdx_t>(identifier));
                        break;
                    } case TechIdx_t::TGC: {
                        stName = idHelperSvc->tgcIdHelper().stationNameString(stIdent);
                        break;
                    } case TechIdx_t::STGC: {
                        stName = idHelperSvc->stgcIdHelper().stationNameString(stIdent);
                        break;
                    } case TechIdx_t::MM: {
                        stName = idHelperSvc->mmIdHelper().stationNameString(stIdent);
                        break;
                    } default:
                        break;
                }
            }
            const std::string histPath = std::format("/{}/Station_{}/{}",
                                                     basePath, stName, h->GetName());
            histSvc->regHist(histPath, h).ignore();
            return h;
        };
        h_dirTheta = registerHisto(new TH1F("dirTheta", "local direction; #theta [deg]", 180, 0, 180));
        h_dirPhi = registerHisto(new TH1F("dirPhi", "local direction; #phi [deg]", 360, -180, 180));
        h_energy = registerHisto(new TH1F("energy", "kinetic energy; e[GeV]", 150, 0, 150));
        h_deposit = registerHisto(new TH1F("energyDeposit", "energy deposit; e[GeV]", 20, 0, 2));
        h_pdgId = registerHisto(new TH1F("pdgId", "Particle data ID; pdgId",31, -15.5, 15.5));
        
        switch (techIdx) {
            case TechIdx_t::TGC: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",48, 0.5, 48.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",11, -5.5, 5.5));
                h_localHitPos = registerHisto(new TH2F("localHitPosXY", "hit position;x [mm]; y[mm]", 
                                                      240, -1200, 1200, 240, -1200, 1200));
                h_localHitZ = registerHisto(new TH1F("localHitPosZ", "position in gas gap;z [mm]", 20, -5, 5));

                constexpr double r = 12. * Gaudi::Units::m;
                constexpr double z = 14 * Gaudi::Units::m;
                h_globHitXY = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 240, -r,r, 240,-r,r));
                h_globHitRZ  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 1200, 0, r, 2200,-z,z));
                break;
            } case TechIdx_t::RPC: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",8, 0.5, 8.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",17, -8.5, 8.5));
                h_localHitPos = registerHisto(new TH2F("localHitPosXY", "hit position;x [mm]; y[mm]", 
                                                      70, -1400, 1400, 70, -1400, 1400));
                h_localHitZ = registerHisto(new TH1F("localHitPosZ", "position in gas gap;z [mm]", 11, -0.5, 0.5));
                constexpr double r = 12. * Gaudi::Units::m;
                constexpr double z = 12 * Gaudi::Units::m;
                h_globHitXY = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 240, -r,r, 240,-r,r));
                h_globHitRZ  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 1200, 0, r, 600,-z,z));

                break;
            } case TechIdx_t::MDT: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",8, 0.5, 8.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",17, -8.5, 8.5));
                h_localHitPos = registerHisto(new TH1F("localHitPosR", "hit position;r [mm]", 61, -15, 15));
                h_localHitZ = registerHisto(new TH1F("localHitPosZ", "position in gas gap;z [mm]", 310, -3100, 3100));
                constexpr double r = 12. * Gaudi::Units::m;
                constexpr double z = 22 * Gaudi::Units::m;
                h_globHitXY = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 2400, -r,r, 2400,-r,r));
                h_globHitRZ  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 1200, 0, r, 2200,-z,z));

                break;
            } case TechIdx_t::MM: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",8, 0.5, 8.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",5, -2.5, 2.5));
                h_localHitPos = registerHisto(new TH2F("localHitPosXY", "hit position;x [mm]; y[mm]", 
                                                      480, -1200, 1200, 480, -2400, 2400));
                h_localHitZ = registerHisto(new TH1F("localHitPosZ", "position in gas gap;z [mm]", 11, -0.5, 0.5));
                constexpr double r = 5 * Gaudi::Units::m;
                constexpr double z = 8. *Gaudi::Units::m;
                h_globHitXY = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 1000, -r,r, 1000,-r,r));
                h_globHitRZ  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 500, 0, r, 160, -z, z));

                break;
            } case TechIdx_t::STGC: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",8, 0.5, 8.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",7, -3.5, 3.5));
                h_localHitPos = registerHisto(new TH2F("localHitPosXY", "hit position;x [mm]; y[mm]", 
                                                      440, -1100, 1100, 440, -1100, 1100));
                h_localHitZ = registerHisto(new TH1F("localHitPosZ", "position in gas gap;z [mm]", 11, -0.5, 0.5));
                constexpr double r = 5 * Gaudi::Units::m;
                constexpr double z = 8. *Gaudi::Units::m;
                h_globHitXY = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 1000, -r,r, 1000,-r,r));
                h_globHitRZ  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 500, 0, r, 160, -z, z));
                break;
            } default:
                break;
        }
    }


    StatusCode xMuonHitAnalysis::initialize(){
        ATH_CHECK(m_inputKey.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        std::set<int> histIds{-1};
        TechIdx_t techIdx{static_cast<TechIdx_t>(1*m_techIdx)};
        using namespace Muon::MuonStationIndex;
        switch (techIdx) {
            case TechIdx_t::MDT: {
                for (int s = 0 ; s < toInt(StIdx_t::StIndexMax); ++s){
                     histIds.insert(s);
                }
                break;
            } case TechIdx_t::RPC: {
                for (int s = 0 ; s < toInt(StIdx_t::BE); ++s){
                     histIds.insert(s);
                }
                break;
            } case TechIdx_t::TGC: {
                const auto& idHelper{m_idHelperSvc->tgcIdHelper()};
                std::for_each(idHelper.module_begin(), idHelper.module_end(),
                     [&histIds, &idHelper](const Identifier& id) {
                        histIds.insert(idHelper.stationName(id));
                    });
                break;
            } case TechIdx_t::STGC: {
                const auto& idHelper{m_idHelperSvc->stgcIdHelper()};
                std::for_each(idHelper.module_begin(), idHelper.module_end(),
                     [&histIds, &idHelper](const Identifier& id) {
                        histIds.insert(idHelper.stationName(id));
                    });
                break;
            } case TechIdx_t::MM: {
                const auto& idHelper{m_idHelperSvc->mmIdHelper()};
                std::for_each(idHelper.module_begin(), idHelper.module_end(),
                     [&histIds, &idHelper](const Identifier& id) {
                        histIds.insert(idHelper.stationName(id));
                    });
                break;
            } default:
                ATH_MSG_FATAL("Invalid technology index parsed "<<m_techIdx);
                return StatusCode::FAILURE;
        }
        for (int id : histIds) {
            m_histos.emplace_back(m_idHelperSvc.get(), histSvc(), m_path,  techIdx, id);
        }
        return StatusCode::SUCCESS;
    }
    StatusCode xMuonHitAnalysis::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        const xAOD::MuonSimHitContainer* simHits{nullptr};
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(simHits, m_inputKey, ctx));
        if (simHits->empty()) {
            ATH_MSG_DEBUG("No hits recorded in the event "<<m_inputKey.fullKey());
            return StatusCode::SUCCESS;
        }
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        xAOD::ChamberViewer viewer{*simHits, m_idHelperSvc.get(), 
                                   xAOD::ChamberView::Mode::Chamber};
        do{
            const Identifier refId{viewer.at(0)->identify()};
            const TechIdx_t techIdx = m_idHelperSvc->technologyIndex(refId);
            
            const int stIdx = (techIdx == TechIdx_t::TGC || techIdx == TechIdx_t::STGC ||
                               techIdx == TechIdx_t::MM ? m_idHelperSvc->stationName(refId)
                                                        : toInt(m_idHelperSvc->stationIndex(refId)));
            const int stationEta = m_idHelperSvc->stationEta(refId);
            const int stationPhi = m_idHelperSvc->stationPhi(refId);

            std::vector<unsigned> fillMe{};
            for (unsigned h = 0; h<m_histos.size(); ++h) {
                if (m_histos[h].identifier == -1 ||
                    m_histos[h].identifier == stIdx){
                    fillMe.push_back(h);
                }
            }
            for (const xAOD::MuonSimHit* hit : viewer) {
                const Identifier& hitId{hit->identify()};
                const MuonGMR4::MuonReadoutElement* re = m_detMgr->getReadoutElement(hitId);
                const double theta = hit->localDirection().theta();
                const double phi = hit->localDirection().phi();
                double signedR{0.};
                IdentifierHash layHash{re->layerHash(hitId)};
                if (techIdx ==TechIdx_t::MDT) {
                    signedR = Amg::lineDistance<3>(Amg::Vector3D::Zero(),
                                                   Amg::Vector3D::UnitZ(),
                                                   xAOD::toEigen(hit->localPosition()),
                                                   xAOD::toEigen(hit->localDirection()));
                    layHash = re->measurementHash(hitId);
                }
                const Amg::Vector3D globPos{re->localToGlobalTransform(*gctx,layHash)*
                                            xAOD::toEigen(hit->localPosition())};

                for (unsigned h : fillMe) {
                    m_histos[h].h_dirTheta->Fill(theta / Gaudi::Units::deg);
                    m_histos[h].h_dirPhi->Fill(phi / Gaudi::Units::deg);
                    m_histos[h].h_energy->Fill(hit->kineticEnergy()/ Gaudi::Units::GeV);
                    m_histos[h].h_deposit->Fill(hit->energyDeposit() / Gaudi::Units::GeV);
                    m_histos[h].h_pdgId->Fill(hit->pdgId());
                    m_histos[h].h_stationPhi->Fill(stationPhi);
                    m_histos[h].h_stationEta->Fill(stationEta);
                    if (techIdx == TechIdx_t::MDT) {
                      m_histos[h].h_localHitPos->Fill(signedR);
                    } else {
                        m_histos[h].h_localHitPos->Fill(hit->localPosition().x(),
                                                        hit->localPosition().y());
                    }
                    m_histos[h].h_localHitZ->Fill(hit->localPosition().z());
                    m_histos[h].h_globHitXY->Fill(globPos.x(), globPos.y());
                    m_histos[h].h_globHitRZ->Fill(globPos.perp(), globPos.z());
                }
            }
        } while(viewer.next());
        return StatusCode::SUCCESS;
    }

}
