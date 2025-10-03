/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef RPC_RDO_ANALYSIS_H
#define RPC_RDO_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/RpcPadContainer.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "MuonCablingData/RpcCablingMap.h"



#include "TH1.h"

namespace MuonVal{
    class RpcRDOAnalysis : public AthHistogramAlgorithm {
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            ~RpcRDOAnalysis() = default;


            virtual StatusCode initialize() override final;
            virtual StatusCode execute() override final;

        private:
            SG::ReadHandleKey<RpcPadContainer> m_inputKeyPad{this, "InputPadKey", "RPCPAD"};
            SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_inputKeyRdo{this, "InputRdoKey", "RPCPAD"};
            SG::ReadCondHandleKey<Muon::RpcCablingMap> m_cablingKey{this, "CablingKey", "MuonNRPC_CablingMap", 
                                                                    "Key of MuonNRPC_CablingMap"};



            /** @brief Service handle of the IdHelperSvc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            Gaudi::Property<std::string> m_path{this, "HistPath", "RpcRDOAnalysis"};


            using ChIdx_t = Muon::MuonStationIndex::ChIndex;
            struct PadHistoSet{
 
                /** @brief Constructor instantiating the monitoring histograms
                 *  @param histSvc: Reference to the histSvc to register the histograms with the file
                 *  @param basePath: Basic path for all histograms
                 *  @param chIndex: Station index to consider for the histogram set */
                PadHistoSet(const ServiceHandle<ITHistSvc>& histSvc,
                            const std::string& basePath,
                            const ChIdx_t chIndex);

                ChIdx_t chIdx{ChIdx_t::ChUnknown};
 
                TH1* h_status{nullptr};
                TH1* h_err{nullptr};
                TH1* h_onlineID{nullptr};
                TH1* h_lvl1ID{nullptr};
                TH1* h_bcID{nullptr};
                TH1* h_sector{nullptr};

                TH1* h_coinRpcID{nullptr};
                TH1* h_coinOnID{nullptr};
                TH1* h_coinCrc{nullptr};
                TH1* h_coinFel1ID{nullptr};
                TH1* h_coinFebcID{nullptr};

                TH1* h_firedBcID{nullptr};
                TH1* h_firedTime{nullptr};
                TH1* h_firedIjk{nullptr};
                TH1* h_firedChan{nullptr};
                TH1* h_firedOvl{nullptr};
                TH1* h_firedThr{nullptr};
            };

            struct RdoHistoSet {                
                /** @brief Constructor instantiating the monitoring histograms
                 *  @param histSvc: Reference to the histSvc to register the histograms with the file
                 *  @param basePath: Basic path for all histograms
                 *  @param chIndex: Station index to consider for the histogram set */
                RdoHistoSet(const ServiceHandle<ITHistSvc>& histSvc,
                            const std::string& basePath,
                            const ChIdx_t chIndex);

                ChIdx_t chIdx{ChIdx_t::ChUnknown};
                /** @brief Bunch crossing ID */
                TH1* h_bcid{nullptr};
                /** @brief Time of record */
                TH1* h_time{nullptr};
                /** @brief Time over threshold */
                TH1* h_tOverThr{nullptr};
                /** @brief sub detector */
                TH1* h_subDet{nullptr};
                /** @brief board sector */
                TH1* h_boardSec{nullptr};
                /** @brief board Id */
                TH1* h_board{nullptr};
                /** @brief channel Id */
                TH1* h_channelId{nullptr};
            };

            std::vector<PadHistoSet> m_padHistos{};
            std::vector<RdoHistoSet> m_rdoHistos{};
    };
}

#endif // RPC_RDO_ANALYSIS_H
