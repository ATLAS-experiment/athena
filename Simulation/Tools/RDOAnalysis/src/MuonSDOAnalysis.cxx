/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSDOAnalysis.h"
#include "StoreGate/ReadHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <algorithm>
#include <format>

#include <TH1F.h>
#include <TH2F.h>
namespace MuonVal{
    using namespace Muon::MuonStationIndex;

    MuonSDOAnalysis::HistoSet::HistoSet(const Muon::IMuonIdHelperSvc* idHelperSvc,
                                        const ServiceHandle<ITHistSvc>& histSvc,
                                        const std::string& basePath,
                                        const TechIdx_t techIdx,
                                        const int stIdent):
        identifier{stIdent}{

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
      
        h_sdoWord = registerHisto(new TH1F("sdoWord", "sdoWord", 100, 0, 10));
        h_eventIndex = registerHisto(new TH1F("eventIndex", "Event index (SDO)", 100, 0, 1000));

        switch (techIdx) {
            case TechIdx_t::TGC: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",48, 0.5, 48.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",11, -5.5, 5.5));
                constexpr double r = 12. * Gaudi::Units::m;
                constexpr double z = 14 * Gaudi::Units::m;
                h_xyPos = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 240, -r,r, 240,-r,r));
                h_rzPos  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 1200, 0, r, 2200,-z,z));
                break;
            } case TechIdx_t::RPC: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",8, 0.5, 8.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",17, -8.5, 8.5));
                constexpr double r = 12. * Gaudi::Units::m;
                constexpr double z = 12 * Gaudi::Units::m;
                h_xyPos = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 240, -r,r, 240,-r,r));
                h_rzPos  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 1200, 0, r, 600,-z,z));

                break;
            } case TechIdx_t::MDT: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",8, 0.5, 8.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",17, -8.5, 8.5));
                constexpr double r = 12. * Gaudi::Units::m;
                constexpr double z = 22 * Gaudi::Units::m;
                h_xyPos = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 2400, -r,r, 2400,-r,r));
                h_rzPos  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 1200, 0, r, 2200,-z,z));

                break;
            } case TechIdx_t::MM: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",8, 0.5, 8.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",5, -2.5, 2.5));
                constexpr double r = 5 * Gaudi::Units::m;
                constexpr double z = 8. *Gaudi::Units::m;
                h_xyPos = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 1000, -r,r, 1000,-r,r));
                h_rzPos  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 500, 0, r, 160, -z, z));

                break;
            } case TechIdx_t::STGC: {
                h_stationPhi = registerHisto(new TH1F("stationPhi", "StationPhi;stationPhi",8, 0.5, 8.5));
                h_stationEta = registerHisto(new TH1F("stationEta", "StationEta;stationEta",7, -3.5, 3.5));
                constexpr double r = 5 * Gaudi::Units::m;
                constexpr double z = 8. *Gaudi::Units::m;
                h_xyPos = registerHisto(new TH2F("hitXY", "hitXY;x [mm]; y[mm];entries", 1000, -r,r, 1000,-r,r));
                h_rzPos  = registerHisto(new TH2F("hitRZ", "hitRz;r [mm]; z[mm];entries", 500, 0, r, 160, -z, z));
                break;
            } default:
                break;
        }
        
        h_radius = registerHisto(new TH1F("h_radius", "Radius (SDO)", 100, -15, 15));
        h_localZ = registerHisto(new TH1F("h_localZ", "Local z-position (SDO)", 100, -3200, 3200));
    }

    StatusCode MuonSDOAnalysis::initialize() {
  
        ATH_CHECK(m_inputKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
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

    StatusCode MuonSDOAnalysis::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        const MuonSimDataCollection* sdoCont{nullptr};
        ATH_CHECK(SG::get(sdoCont, m_inputKey, ctx));

        for (const auto&[sdoID, sdo]: *sdoCont) {
            std::vector<unsigned> fillMe{};
            const TechIdx_t techIdx = m_idHelperSvc->technologyIndex(sdoID);
            
            const int stIdx = (techIdx == TechIdx_t::TGC || techIdx == TechIdx_t::STGC ||
                               techIdx == TechIdx_t::MM ? m_idHelperSvc->stationName(sdoID)
                                                        : toInt(m_idHelperSvc->stationIndex(sdoID)));
            const int stationEta = m_idHelperSvc->stationEta(sdoID);
            const int stationPhi = m_idHelperSvc->stationPhi(sdoID);


            for (unsigned h = 0 ; h < m_histos.size(); ++h) {
                if (m_histos[h].identifier ==  -1 || 
                    m_histos[h].identifier == stIdx) {
                    fillMe.push_back(h);
                }
            }

            const int sdoWord =  sdo.word();
            const Amg::Vector3D gPos = sdo.globalPosition();
            const float xPos = gPos.x();
            const float yPos = gPos.y();
            const float zPos = gPos.z();
            for (unsigned h : fillMe) {
                m_histos[h].h_sdoWord->Fill(sdoWord);
                m_histos[h].h_xyPos->Fill(xPos, yPos);
                m_histos[h].h_rzPos->Fill(std::hypot(xPos, yPos), zPos);
                m_histos[h].h_stationPhi->Fill(stationPhi);
                m_histos[h].h_stationEta->Fill(stationEta);
                
            }
            for (const auto& [particleLink, data]: sdo.getdeposits()) {
                const int eventIx = particleLink.eventIndex();
                const float radius = data.firstEntry();
                const float localZ = data.secondEntry();
                for (unsigned h : fillMe) {
                    m_histos[h].h_eventIndex->Fill(eventIx);
                    m_histos[h].h_radius->Fill(radius);
                    m_histos[h].h_localZ->Fill(localZ);
                }
            }
        }
        return StatusCode::SUCCESS;
    }
}
 
