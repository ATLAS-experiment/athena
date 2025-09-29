/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MDT_RDO_ANALYSIS_H
#define MDT_RDO_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "MuonRDO/MdtCsmContainer.h"

#include "TH1.h"


namespace MuonVal{
    /** @brief Class dumping the basic distributions of the Mdt RDOs per station index */
    class MdtRDOAnalysis : public AthHistogramAlgorithm {

        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute() override final;
        private:
            /** @brief Input read handle key */
            SG::ReadHandleKey<MdtCsmContainer> m_inputKey{this, "InputKey", "MDTCSM"};
            /** @brief Service handle of the IdHelperSvc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            Gaudi::Property<std::string> m_path{this, "HistPath", "MdtRDOAnalysis"};


            using StIdx_t = Muon::MuonStationIndex::StIndex;
            /** @brief Histograms per station index */
            struct HistoSet{
                /** @brief Default constructor */
                HistoSet() = default;
                /** @brief Constructor instantiating the monitoring histograms
                 *  @param histSvc: Reference to the histSvc to register the histograms with the file
                 *  @param basePath: Basic path for all histograms
                 *  @param stIdx: Station index to consider for the histogram set */
                HistoSet(const ServiceHandle<ITHistSvc>& histSvc,
                         const std::string& basePath,
                         const StIdx_t stIdx);

                StIdx_t stIdx{StIdx_t::StUnknown};
                TH1* h_subID{nullptr};
                TH1* h_mrodID{nullptr};
                TH1* h_csmID{nullptr};
                TH1* h_tdcID{nullptr};
                TH1* h_chanID{nullptr};
                TH1* h_coarse{nullptr};
                TH1* h_fine{nullptr};
                TH1* h_width{nullptr};
            }; 
            std::vector<HistoSet> m_histos{};
    };
}
#endif // MDT_RDO_ANALYSIS_H
