/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TGC_RDO_ANALYSIS_H
#define TGC_RDO_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "MuonRDO/TgcRdoContainer.h"

#include "TH1.h"


namespace MuonVal{
    class TgcRDOAnalysis : public AthHistogramAlgorithm {

        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual ~TgcRDOAnalysis()= default;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute() override final;
 
        private:
            SG::ReadHandleKey<TgcRdoContainer> m_inputKey{this, "InputKey", "TGCRDO"};

              /** @brief Service handle of the IdHelperSvc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            Gaudi::Property<std::string> m_path{this, "HistPath", "RpcRDOAnalysis"};

            struct HistoSet {

                /** @brief Constructor instantiating the monitoring histograms
                 *  @param histSvc: Reference to the histSvc to register the histograms with the file
                 *  @param basePath: Basic path for all histograms
                 *  @param stName: Station name to consider for the histogram set */
                HistoSet(const TgcIdHelper& idHelper,
                         const ServiceHandle<ITHistSvc>& histSvc,
                         const std::string& basePath,
                         const int stName);


                int stationName{-1};
                TH1* h_tgcID{nullptr};
                TH1* h_tgcSubDetID{nullptr};
                TH1* h_tgcRodID{nullptr};
                TH1* h_tgcTrigType{nullptr};
                TH1* h_tgcBcID{nullptr};
                TH1* h_tgcL1ID{nullptr};
                TH1* h_bcTag{nullptr};
                TH1* h_subDetID{nullptr};
                TH1* h_rodID{nullptr};
                TH1* h_sswID{nullptr};
                TH1* h_slbID{nullptr};
                TH1* h_bcID{nullptr};
                TH1* h_l1ID{nullptr};
                TH1* h_type{nullptr};
                TH1* h_slbType{nullptr};
                TH1* h_bitPos{nullptr};
                TH1* h_track{nullptr};
                TH1* h_adj{nullptr};
            };
            std::vector<HistoSet> m_histos{};
 

};
}
#endif // TGC_RDO_ANALYSIS_H
