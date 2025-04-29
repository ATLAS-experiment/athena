/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPERFORMANCEALGS_MUONTPMETADATAALG_H
#define MUONPERFORMANCEALGS_MUONTPMETADATAALG_H
#include <AthAnalysisBaseComps/AthAnalysisAlgorithm.h>
#include <AsgAnalysisInterfaces/IPileupReweightingTool.h>
#include <StoreGate/ReadHandleKey.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <xAODCutFlow/CutBookkeeper.h>
#include <xAODEventInfo/EventInfo.h>

namespace MuonVal{
    
class MetaDataAlg : public  AthAnalysisAlgorithm {
public:
    using AthAnalysisAlgorithm::AthAnalysisAlgorithm;
    virtual ~MetaDataAlg() = default;

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;
    StatusCode beginInputFile() override;
    unsigned int cardinality() const override final { return 1; }

private:
    SG::ReadHandleKey<xAOD::EventInfo> m_infoKey{this, "EvtInfoKey", "EventInfo"};
    /// Output file stream
    Gaudi::Property<std::string> m_stream{this, "OutStream", "MUONTP"};

    //
    Gaudi::Property<bool> m_isData{this, "isData", false};
    /// Is derived DAOD property
    Gaudi::Property<bool> m_isDerivedAOD{this, "isDAOD", false};
    /// On AFII simulation
    Gaudi::Property<bool> m_isAF2{this, "isAF2", false};
    /// Store LHE weights
    Gaudi::Property<bool> m_storeLHE{this, "writeLHE", false};

    ToolHandle<CP::IPileupReweightingTool> m_prwTool{this, "prwTool", ""};
    bool m_hasPrwTool{false};
    /// Meta data tree object
    std::unique_ptr<MuonTesterTree> m_MetaDataTree{nullptr};

    /// Helper structs to cache the meta-data
    /// for simulation and recorded data
    struct RunMetaData {
        ~RunMetaData() = default;
        RunMetaData(unsigned int run, const std::string& stream) : run_number{run}, trigger_stream{stream} {}
        bool operator<(const RunMetaData& other) const { return run_number < other.run_number; }
        /// run number of the current file
        unsigned int run_number{0};
        /// Total events from the upstream AOD
        Long64_t tot_events{0};
        /// Processed Events
        Long64_t proc_events{0};
        /// Name of the trigger stream
        std::string trigger_stream{};
        /// Luminosity blocks from the cache upstream
        std::set<unsigned int> total_lumi_blocks{};
        /// Luminosity blocks from the file
        std::set<unsigned int> processed_blocks{};

        bool has_book_keeper{false};
    };
    struct SimMetaData {
        ~SimMetaData() = default;
        SimMetaData(int dsid, unsigned int prw) : mc_channel{dsid}, prw_channel{prw} {}
        bool operator<(const SimMetaData& other) const {
            if (other.mc_channel != mc_channel) return mc_channel < other.mc_channel;
            if (other.prw_channel != prw_channel) return prw_channel < other.prw_channel;
            return variation_number < other.variation_number;
        }
        /// Mc channel number
        int mc_channel{0};
        /// run number used for the pile-up campaign
        unsigned int prw_channel{0};
        /// Expected luminosity
        double prw_lumi{0};
        /// Number of total events from the file upstream
        Long64_t tot_events{0};
        /// Number of processed events in the file itself
        Long64_t proc_events{0};
        /// Sum of weights
        double sum_w{0.};
        double sum_w_squared{0.};

        /// Name of the weight variation
        std::string weight_name{};
        /// Position inside the weight vector
        unsigned int variation_number{0};
        /// Print the WARNING
        bool has_book_keeper{false};
        /// Make sure that the WARNING is only printed once
        bool warned{false};
    };
    StatusCode fillDataTree();
    StatusCode fillSimulationTree();


    std::vector<SimMetaData> m_sim_meta{};
    std::vector<RunMetaData> m_run_meta{};

    const xAOD::CutBookkeeper* RetrieveCutBookKeeper(const std::string& stream, const std::string& cbk_name = "AllExecutedEvents") const;
};
}
#endif  //> !MUONPERFORMANCEALGS_MUONTPMETADATAALG_H
