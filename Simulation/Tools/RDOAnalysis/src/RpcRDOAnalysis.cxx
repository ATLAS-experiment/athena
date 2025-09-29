/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcRDOAnalysis.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"

#include <format>

namespace MuonVal{
    RpcRDOAnalysis::PadHistoSet::PadHistoSet(const ServiceHandle<ITHistSvc>& histSvc,
                                             const std::string& basePath,
                                             const ChIdx_t chIndex):
        chIdx{chIndex} {
        
        auto createHisto = [&histSvc, &basePath, chIndex](const std::string& hName,
                                                          const std::string& hTitle,
                                                          const unsigned nBins,
                                                          const float xLow,
                                                          const float xHigh) {
            TH1* h = new TH1F(hName.c_str(), hTitle.c_str(), nBins, xLow, xHigh);
            h->Sumw2();
            histSvc->regHist(std::format("/{}/PAD/Chamber_{}/{}",
                                        basePath, 
                                        chIndex != ChIdx_t::ChUnknown ? chName(chIndex) : "Inclusive", 
                                        hName), h).ignore();
            return h;
        };
        h_status = createHisto("status", "RPC Pad Status", 100, 0, 10);
        h_err = createHisto("error", "RPC Pad error code", 100, 0, 10);
        h_onlineID = createHisto("onlineID", "RPC pad online ID", 100, 0, 10);
        h_lvl1ID = createHisto("lvl1ID", "RPC Pad online ID", 100, 0, 10);
        h_bcID = createHisto("bcid", "RPC Pad BCID", 100,0, 10);
        h_sector = createHisto("sector", "RPC Pad Sector", 100, 0, 75);

        h_coinOnID = createHisto("coinOnID", "CM Online ID", 100, 0, 10);
        h_coinCrc = createHisto("coinCrc", "CM CRC", 100, 0 , 300);      
        h_coinFel1ID = createHisto("coinFel1ID", "CM FEL1ID", 100, 0, 10);
        h_coinFebcID = createHisto("coinFebcID", "CM FEBCID", 100, 0, 10);
        h_firedBcID = createHisto("h_firedBcID", "Fired Channel BCID", 100, 0, 10);
        h_firedTime = createHisto("h_firedTime", "Fired Channel time", 100, 0, 10);
        h_firedChan = createHisto("h_firedChan", "Fired Channel number", 100, 0, 35);
        h_firedIjk = createHisto("h_firedIjk", "Fired Channel ijk", 100, 0, 10);
 
        h_firedOvl = createHisto("h_firedOvl", "Fired Channel overlap", 100, 0, 10);
        h_firedThr = createHisto("h_firedThr", "Fired Channel threshold", 100, 0, 5);
    }
    RpcRDOAnalysis::RdoHistoSet::RdoHistoSet(const ServiceHandle<ITHistSvc>& histSvc,
                                             const std::string& basePath,
                                             const ChIdx_t chIndex):
        chIdx{chIndex} {
        
        auto createHisto = [&histSvc, &basePath, chIndex](const std::string& hName,
                                                          const std::string& hTitle,
                                                          const unsigned nBins,
                                                          const float xLow,
                                                          const float xHigh) {
            TH1* h = new TH1F(hName.c_str(), hTitle.c_str(), nBins, xLow, xHigh);
            h->Sumw2();
            histSvc->regHist(std::format("/{}/RDO/Chamber_{}/{}",
                                        basePath, 
                                        chIndex != ChIdx_t::ChUnknown ? chName(chIndex) : "Inclusive", 
                                        hName), h).ignore();
            return h;
        };
        h_bcid = createHisto("bcid", "Bunch Crossing ID", 5,10,100);
        h_time = createHisto("time", "Recorded time", 45, 10, 100);
        h_tOverThr = createHisto("tot", "Time over threshold", 0, 30 , 30);
        h_subDet = createHisto("subdet", "Subdetector ID", 50, 0, 100);
        h_boardSec = createHisto("boardsector", "Board sector ID", 50, 0, 100);
        h_board = createHisto("boardID", "Board identifier", 50, 0, 100);
        h_channelId = createHisto("channelID", "Channel identifier", 50,0 , 100);
 
    }

    StatusCode RpcRDOAnalysis::initialize() {
        ATH_CHECK(m_inputKeyPad.initialize(!m_inputKeyPad.empty()));
        ATH_CHECK(m_inputKeyRdo.initialize(!m_inputKeyRdo.empty()));
        ATH_CHECK(m_cablingKey.initialize(!m_inputKeyRdo.empty()));
        ATH_CHECK(m_idHelperSvc.retrieve());
        for (int8_t ch = toInt(ChIdx_t::ChUnknown); ch != toInt(ChIdx_t::BEE); ++ch) {
            using namespace Muon::MuonStationIndex;
            auto chIdx = ChIdx_t{ch};
            if (!m_inputKeyPad.empty() && toLayerIndex(chIdx) != LayerIndex::Inner) {
                m_padHistos.emplace_back(histSvc(), m_path, chIdx);   
            }
            if (m_inputKeyRdo.empty()) {
                continue;
            }
            // If RpcPads are active, only the BIS78 NRPC ROs exist
            if (!m_inputKeyPad.empty() && chIdx != ChIdx_t::BIS) {
                continue;
            }
            m_rdoHistos.emplace_back(histSvc(), m_path, chIdx);
        }

        return StatusCode::SUCCESS;
    }

    StatusCode RpcRDOAnalysis::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        const RpcPadContainer* rpcPads{nullptr};
        const xAOD::NRPCRDOContainer* rpcRdos{nullptr};
        ATH_CHECK(SG::get(rpcPads, m_inputKeyPad, ctx));
        ATH_CHECK(SG::get(rpcRdos, m_inputKeyRdo, ctx));

        if (rpcPads){
            for (const RpcPad* pad : *rpcPads){
                const ChIdx_t chIdx = m_idHelperSvc->chamberIndex(pad->identify());
                std::vector<unsigned> fillMe{};
                for (unsigned h = 0 ; h < m_padHistos.size(); ++h) {
                    if (m_padHistos[h].chIdx == ChIdx_t::ChUnknown ||
                        m_padHistos[h].chIdx == chIdx){
                        fillMe.push_back(h);
                    }
                }

                const uint16_t padStat = pad->status();
                const uint16_t padErr = pad->errorCode();
                const uint16_t padOnID = pad->onlineId();
                const uint16_t padLVL1ID = pad->lvl1Id();
                const uint16_t padBCID = pad->bcId();
                const int padSec = pad->sector();
                for (unsigned h : fillMe) {
                    m_padHistos[h].h_status->Fill(padStat);
                    m_padHistos[h].h_err->Fill(padErr);
                    m_padHistos[h].h_onlineID->Fill(padOnID);
                    m_padHistos[h].h_lvl1ID->Fill(padLVL1ID);
                    m_padHistos[h].h_bcID->Fill(padBCID);
                    m_padHistos[h].h_sector->Fill(padSec);
                }
                for (const RpcCoinMatrix* coinMat : *pad) {
                    const uint16_t coinOnID = coinMat->onlineId();
                    const uint16_t coinCrc = coinMat->crc();
                    const uint16_t coinFel1ID = coinMat->fel1Id();
                    const uint16_t coinFebcID = coinMat->febcId();
                    for (unsigned h : fillMe) {
                        m_padHistos[h].h_coinOnID->Fill(coinOnID);
                        m_padHistos[h].h_coinCrc->Fill(coinCrc);
                        m_padHistos[h].h_coinFel1ID->Fill(coinFel1ID);
                        m_padHistos[h].h_coinFebcID->Fill(coinFebcID);
                    }

                    for (const RpcFiredChannel* firedCh : *coinMat) {
                        const uint16_t firedBcID = firedCh->bcid();
                        const uint16_t firedTime = firedCh->time();
                        const uint16_t firedIjk = firedCh->ijk();
                        const uint16_t firedChan = firedCh->channel();
                        const uint16_t firedOvl = firedCh->ovl();
                        const uint16_t firedThr = firedCh->thr();
                        for (unsigned h : fillMe) {                 
                            m_padHistos[h].h_firedBcID->Fill(firedBcID);
                            m_padHistos[h].h_firedTime->Fill(firedTime);
                            m_padHistos[h].h_firedIjk->Fill(firedIjk);
                            m_padHistos[h].h_firedChan->Fill(firedChan);
                            m_padHistos[h].h_firedOvl->Fill(firedOvl);
                            m_padHistos[h].h_firedThr->Fill(firedThr);
                        }
                    }
                }
            }
            if (rpcRdos) {
                const Muon::RpcCablingMap* cabling{nullptr};
                ATH_CHECK(SG::get(cabling, m_cablingKey, ctx));
                for (const xAOD::NRPCRDO* rdo : *rpcRdos) {
                    Muon::RpcCablingData convObj{};
                    convObj.subDetector = rdo->subdetector();
                    convObj.boardSector = rdo->boardsector();
                    convObj.board = rdo->board();
                    convObj.channelId = rdo->channel();
        
                    if (!cabling->getOfflineId(convObj, msgStream())) {
                        ATH_MSG_FATAL("Failed to convert online -> offline");
                        return StatusCode::FAILURE;
                    }
                    Identifier chanId{};
                    if (!cabling->convert(convObj, chanId)) {
                        return StatusCode::FAILURE;
                    }
                    const ChIdx_t chIdx = m_idHelperSvc->chamberIndex(chanId);

                    std::vector<unsigned> fillMe{};
                    for (unsigned h = 0 ; h < m_padHistos.size(); ++h) {
                        if (m_rdoHistos[h].chIdx == ChIdx_t::ChUnknown ||
                            m_rdoHistos[h].chIdx == chIdx){
                            fillMe.push_back(h);
                        }
                    }

                    for (unsigned h : fillMe) {
                        m_rdoHistos[h].h_bcid->Fill(rdo->bcid());
                        m_rdoHistos[h].h_time->Fill(rdo->time());
                        m_rdoHistos[h].h_tOverThr->Fill(rdo->timeoverthr());
                        m_rdoHistos[h].h_subDet->Fill(rdo->subdetector());
                        m_rdoHistos[h].h_boardSec->Fill(rdo->boardsector());
                        m_rdoHistos[h].h_board->Fill(rdo->board());
                        m_rdoHistos[h].h_channelId->Fill(rdo->channel());
                    }
                }
            }
        }  
        return StatusCode::SUCCESS;
    }

}