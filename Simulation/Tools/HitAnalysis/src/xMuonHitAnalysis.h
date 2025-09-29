
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HitAnalysis_xMuonSimHitAnalysis_H
#define HitAnalysis_xMuonSimHitAnalysis_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"

namespace MuonValR4{
    class xMuonHitAnalysis : public AthHistogramAlgorithm{
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute() override final;
        private:
            SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_inputKey{this, "InputKey", ""};
            // ACTS geometry context
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Service handle of the IdHelperSvc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            /** @brief MuonSimHit */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            Gaudi::Property<std::string> m_path{this, "HistPath", "MuonSDOAnalysis"};

            using StIdx_t = Muon::MuonStationIndex::StIndex;
            using TechIdx_t = Muon::MuonStationIndex::TechnologyIndex;
 
            Gaudi::Property<int> m_techIdx{this, "techIndex", -1};

            struct HistoSet{
                /** @brief Helper struct to define histograms spectrometer region 
                 *  @param idHelperSvc: Pointer to the idHelperSvc
                 *  @param histSvc: Reference to the THistSvc piping the histograms
                 *  @param basePath: Base path of the histogram
                 *  @param techIdx: Technology index of the histograms to plot
                 *  @param stIdent: Station identifier (stationName or stIndex) */
                HistoSet(const Muon::IMuonIdHelperSvc* idHelperSvc,
                         const ServiceHandle<ITHistSvc>& histSvc,
                         const std::string& basePath,
                         const TechIdx_t techIdx,
                         const int stIdent);

                /** @brief Identifier (Either the stationIndex (Mdt/Rpc) or stationName (Tgc/Mm/sTgc)) */
                int identifier{-1};
                /** @brief Local theta direction of the hit */
                TH1* h_dirTheta{nullptr};
                /** @brief Local phi direction of the hit */
                TH1* h_dirPhi{nullptr};
                /** @brief Energy of the hit */
                TH1* h_energy{nullptr};
                /** @brief Energy deposit of the hit */
                TH1* h_deposit{nullptr};
                /** @brief pdgId of the hit */
                TH1* h_pdgId{nullptr};
                /** @brief Station phi of the hit  */
                TH1* h_stationPhi{nullptr};
                /** @brief Station eta of the hit */
                TH1* h_stationEta{nullptr};
                /** @brief Local position of the hit (signed radius) / pos in x-y plane */
                TH1* h_localHitPos{nullptr};
                /** @brief Local z-hit position */
                TH1* h_localHitZ{nullptr};
                /** @brief Global hit position (x-y) plane*/
                TH1* h_globHitXY{nullptr};
                /** @brief Global hit position (r-z) plane */
                TH1* h_globHitRZ{nullptr};
            };
            std::vector<HistoSet> m_histos{};
    };
}

#endif