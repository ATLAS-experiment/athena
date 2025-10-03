/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TgcRDOAnalysis.h"
#include "StoreGate/ReadHandle.h"

#include <format>

namespace MuonVal{
     TgcRDOAnalysis::HistoSet::HistoSet(const TgcIdHelper& idHelper,
                                        const ServiceHandle<ITHistSvc>& histSvc,
                                        const std::string& basePath,
                                        const int stName):
        stationName{stName}{
        auto createHisto = [&](const std::string& hName,
                              const std::string& hTitle,
                              const unsigned nBins,
                              const float xLow,
                              const float xHigh) {
            TH1* h = new TH1F(hName.c_str(), hTitle.c_str(), nBins, xLow, xHigh);
            h->Sumw2();
            histSvc->regHist(std::format("/{}/RDO/Station_{}/{}",
                                        basePath, 
                                        stName > 0 ? idHelper.stationNameString(stName) : "Inclusive",
                                        hName), h).ignore();
            return h;
        };
        h_tgcID = createHisto("tgcID", "TGC ID", 100, 0, 25);
        h_tgcSubDetID = createHisto("tgcSubDetID", "TGC sub-detector ID", 100, 0, 110);
        h_tgcRodID = createHisto("tgcRodID", "TGC ROD ID", 100, 0, 15);
        h_tgcTrigType = createHisto("tgcTrigType", "TGC trigger type", 100, 0, 2);
        h_tgcBcID = createHisto("tgcBcID", "TGC BCID", 100, 0, 2);
        h_tgcL1ID = createHisto("tgcL1ID", "TGC L1ID", 100, 0, 2);
        h_bcTag = createHisto("bcTag", "BC Tag", 100, 0, 5);
        h_subDetID = createHisto("subDetID", "Sub-detector ID", 100, 0, 110);
        h_rodID = createHisto("rodID", "ROD ID", 100, 0, 15);        
        h_sswID = createHisto("sswID", "SSW ID", 100, 0, 10);        
        h_slbID = createHisto("slbID", "SLB ID", 100, 0, 30);        
        h_bcID = createHisto("bcID", "BCID", 100, 0, 10);        
        h_l1ID = createHisto("l1ID", "L1ID", 100, 0, 10);        
        h_type = createHisto("type", "type (hit or coinc)", 100, 0, 10);        
        h_slbType = createHisto("slbType", "SLB type", 100, 0, 10);        
        h_bitPos = createHisto("bitPos", "Bitmap position", 100, 0, 220);        
        h_track = createHisto("track", "tracklet", 100, 0, 10);        
        h_adj = createHisto("adj", "Adjacent", 100, 0, 2);
    }


    StatusCode TgcRDOAnalysis::initialize() {
        ATH_CHECK(m_inputKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());

        std::set<int> stationNames{-1};
        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        std::for_each(idHelper.module_begin(), idHelper.module_end(),
                     [&stationNames, &idHelper](const Identifier& id) {
                        stationNames.insert(idHelper.stationName(id));
                    });
        for (const int st : stationNames) {
           m_histos.emplace_back(idHelper, histSvc(), m_path, st);
        }
        
        return StatusCode::SUCCESS;
    }

    StatusCode TgcRDOAnalysis::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        const TgcRdoContainer* rdos{nullptr};
        ATH_CHECK(SG::get(rdos, m_inputKey, ctx));
        for (const TgcRdo* rdo : *rdos) {
            const uint16_t tgcSubDetID = rdo->subDetectorId();
            const uint16_t tgcID = rdo->identify();
            const uint16_t tgcTrigType = rdo->triggerType();
            const uint16_t tgcRodID = rdo->rodId();
            const uint16_t tgcBcID = rdo->bcId();
            const uint16_t tgcL1ID = rdo->l1Id();

            std::vector<unsigned> fillMe{};
            /// Need to implement the other station names
            for (unsigned h = 0 ; h < m_histos.size(); ++h) {
                if (m_histos[h].stationName == -1) {
                    fillMe.push_back(h);
                }
            }

            for (int h : fillMe) {
                m_histos[h].h_tgcID->Fill(tgcID);
                m_histos[h].h_tgcSubDetID->Fill(tgcSubDetID);
                m_histos[h].h_tgcRodID->Fill(tgcRodID);
                m_histos[h].h_tgcTrigType->Fill(tgcTrigType);
                m_histos[h].h_tgcBcID->Fill(tgcBcID);
                m_histos[h].h_tgcL1ID->Fill(tgcL1ID);
            }

            for (const TgcRawData* channel: *rdo) {
                const uint16_t bctag = channel->bcTag();
                const uint16_t subDetID = channel->subDetectorId();
                const uint16_t rodID = channel->rodId();
                const uint16_t sswID = channel->sswId();
                const uint16_t slbID = channel->slbId();
                const uint16_t bcID = channel->bcId();
                const uint16_t l1ID = channel->l1Id();
                TgcRawData::DataType type_var = channel->type();
                TgcRawData::SlbType slbType_var = channel->slbType();
                const uint16_t bitPos = channel->bitpos();
                const uint16_t track = channel->tracklet();
                const bool adj = channel->isAdjacent();

                for (int h : fillMe) {
                    m_histos[h].h_bcTag->Fill(bctag);
                    m_histos[h].h_subDetID->Fill(subDetID);
                    m_histos[h].h_rodID->Fill(rodID);
                    m_histos[h].h_sswID->Fill(sswID);
                    m_histos[h].h_slbID->Fill(slbID);
                    m_histos[h].h_bcID->Fill(bcID);
                    m_histos[h].h_l1ID->Fill(l1ID);
                    m_histos[h].h_type->Fill(type_var);
                    m_histos[h].h_slbType->Fill(slbType_var);
                    if (type_var== 0) {
                        m_histos[h].h_bitPos->Fill(bitPos);
                        m_histos[h].h_track->Fill(track);
                        m_histos[h].h_adj->Fill(adj);
                    }
                }
            }
        }
        return StatusCode::SUCCESS;
    }
}