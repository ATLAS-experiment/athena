/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef Muon_SDO_ANALYSIS_H
#define Muon_SDO_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "MuonSimData/MuonSimDataCollection.h"

#include "TH1.h"


namespace MuonVal{
    /** @brief Class dumping the basic distributions of the Mdt RDOs per station index */
    class MuonSDOAnalysis : public AthHistogramAlgorithm {

        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute() override final;
        private:
            /** @brief Input read handle key */
            SG::ReadHandleKey<MuonSimDataCollection> m_inputKey{this, "InputKey", "MDT_SDO"};
            /** @brief Service handle of the IdHelperSvc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            Gaudi::Property<std::string> m_path{this, "HistPath", "MuonSDOAnalysis"};

            using StIdx_t = Muon::MuonStationIndex::StIndex;
            using TechIdx_t = Muon::MuonStationIndex::TechnologyIndex;
 
            Gaudi::Property<int> m_techIdx{this, "techIndex", -1};
            
            /** @brief Histograms per station index */
            struct HistoSet{
                /** @brief Default constructor */
                HistoSet() = default;
                /** @brief Constructor instantiating the monitoring histograms
                 *  @param histSvc: Reference to the histSvc to register the histograms with the file
                 *  @param basePath: Basic path for all histograms
                 *  @param stIdx: Station index to consider for the histogram set */
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
                /** @brief pdgId of the hit */
                TH1* h_pdgId{nullptr};
                /** @brief Station phi of the hit  */
                TH1* h_stationPhi{nullptr};
                /** @brief Station eta of the hit */
                TH1* h_stationEta{nullptr};
                /** @brief Encoded data word of the SDO */
                TH1* h_sdoWord{nullptr};
                /** @brief Position in the x-y plane */
                TH1* h_xyPos{nullptr};
                /** @brief Position in the r-z plane */
                TH1* h_rzPos{nullptr};
                /** @brief Index of the underlying event collection */
                TH1* h_eventIndex{nullptr};

                TH1* h_radius{nullptr};
                TH1* h_localZ{nullptr};
            };
 
            std::vector<HistoSet> m_histos{};
   
    };
}
#endif // MDT_RDO_ANALYSIS_H
