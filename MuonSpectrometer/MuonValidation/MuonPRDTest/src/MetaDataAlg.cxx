/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MetaDataAlg.h"

#include <AthAnalysisBaseComps/AthAnalysisHelper.h>
#include <EventInfo/EventStreamInfo.h>
#include <MuonTesterTree/EventInfoBranch.h>
#include <PileupReweighting/TPileupReweighting.h>
#include <StoreGate/ReadHandle.h>
#include <xAODCutFlow/CutBookkeeperContainer.h>
#include <xAODMetaData/FileMetaData.h>


namespace {
   std::string ReplaceExpInString(std::string str, const std::string &exp, const std::string &rep) {
        size_t ExpPos = str.find(exp);
        if (ExpPos == std::string::npos) return str;
        str.replace(ExpPos,exp.size(),rep);
        if (str.find(exp) != std::string::npos) return ReplaceExpInString(str, exp, rep);
        return str;
    }
    
}
namespace MuonVal{

StatusCode MetaDataAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << "...");
    ATH_CHECK(m_infoKey.initialize());
    m_MetaDataTree = std::make_unique<MuonTesterTree>("MetaDataTree", m_stream);
    ATH_CHECK(m_MetaDataTree->init(this));
    
    if (!m_prwTool.empty()) {
        ATH_CHECK(m_prwTool.retrieve());
        m_hasPrwTool = true;
    }
    if (m_storeLHE && !m_isData) {
        std::map<std::string, int> weight_names{};
        AthAnalysisHelper::retrieveMetadata("/Generation/Parameters", "HepMCWeightNames", weight_names).isSuccess();
        EventInfoBranch::setNumLHE(weight_names.size());
    }
    return StatusCode::SUCCESS;
}

StatusCode MetaDataAlg::finalize() {
    if (m_sim_meta.empty() == m_run_meta.empty()) {
        ATH_MSG_FATAL("Found run and simulation meta data");
        return StatusCode::FAILURE;
    }
    /// Define common branches
    m_MetaDataTree->newScalar<bool>("isData", m_isData);
    m_MetaDataTree->newScalar<bool>("isDerivedAOD", m_isDerivedAOD);    
    if (m_isData) ATH_CHECK(fillDataTree());
    else ATH_CHECK(fillSimulationTree());
    ATH_CHECK(m_MetaDataTree->write());
    m_MetaDataTree.reset();
    
    return StatusCode::SUCCESS;
}

StatusCode MetaDataAlg::execute() {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    ATH_MSG_DEBUG("Executing " << name() << "...");
    setFilterPassed(true);

    SG::ReadHandle<xAOD::EventInfo> evtInfo{m_infoKey, ctx};
    if (!evtInfo.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve " << m_infoKey.fullKey());
        return StatusCode::FAILURE;
    }

    bool isData = !evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION);

    int mc_channel = !isData ? evtInfo->mcChannelNumber() : 0.;
    unsigned int run_number = evtInfo->runNumber();
    if (isData) {
        std::string trigger_stream{};
        ATH_CHECK(AthAnalysisHelper::retrieveMetadata("/TagInfo", "triggerStreamOfFile", trigger_stream));

        std::vector<RunMetaData>::iterator itr =
            std::find_if(m_run_meta.begin(), m_run_meta.end(), [&run_number, &trigger_stream](const RunMetaData& meta) {
                return meta.run_number == run_number && meta.trigger_stream == trigger_stream;
            });
        if (itr == m_run_meta.end()) {
            ATH_MSG_WARNING("No meta data information is available for " << run_number << ". Please check on which stream you're running");
            m_run_meta.emplace_back(run_number, trigger_stream);
            itr = m_run_meta.end() - 1;
        }
        itr->proc_events += 1;
        itr->processed_blocks.insert(evtInfo->lumiBlock());
        if (!itr->has_book_keeper) {
            itr->total_lumi_blocks.insert(evtInfo->lumiBlock());
            itr->tot_events += 1;
        }
    } else {
        const unsigned int num_lhe = !m_storeLHE ? 1 : evtInfo->mcEventWeights().size();
        for (unsigned int lhe_var = 0; lhe_var < num_lhe; ++lhe_var) {
            std::vector<SimMetaData>::iterator itr =
                std::find_if(m_sim_meta.begin(), m_sim_meta.end(), [&mc_channel, &run_number, &lhe_var](const SimMetaData& meta) {
                    return meta.mc_channel == mc_channel && meta.prw_channel == run_number && meta.variation_number == lhe_var;
                });
            if (itr == m_sim_meta.end()) {
                ATH_MSG_WARNING("Failed to retrieve a proper meta data for dsid: " << mc_channel << " period: " << run_number
                                                                                   << " lhe weight: " << lhe_var);
                m_sim_meta.emplace_back(mc_channel, run_number);
                itr = m_sim_meta.end() - 1;
                itr->weight_name = "IncompleteMetaData";
                itr->variation_number = lhe_var;
            }
            itr->proc_events += 1;
            if (itr->has_book_keeper) continue;
            if (!itr->warned) {
                ATH_MSG_WARNING("Cut book keeper has not been loaded for sample DSID: "
                                << itr->mc_channel << " period: " << itr->prw_channel << " lhe variation: " << lhe_var);
                itr->warned = true;
            }
            itr->tot_events += 1;
            const double weight = evtInfo->mcEventWeight(lhe_var);
            itr->sum_w += weight;
            itr->sum_w_squared += weight * weight;
        }
    }

    return StatusCode::SUCCESS;
}
const xAOD::CutBookkeeper* MetaDataAlg::RetrieveCutBookKeeper(const std::string& Stream, const std::string& cbk_name) const {
    const xAOD::CutBookkeeper* all = nullptr;
    if (inputMetaStore()->contains<xAOD::CutBookkeeperContainer>("CutBookkeepers")) {
        const xAOD::CutBookkeeperContainer* bks = nullptr;
        if (!inputMetaStore()->retrieve(bks, "CutBookkeepers").isSuccess()) {
            ATH_MSG_WARNING("Could not retrieve the CutBookKeeperContainer. Although it should be there");
            return all;
        }
        int maxCycle = -1;  // need to find the max cycle where input stream is StreamAOD and the name is AllExecutedEvents
        for (const xAOD::CutBookkeeper* cbk : *bks) {
            ATH_MSG_INFO("Check cutbook keeper "<<cbk->inputStream()<<" name: "<<cbk->name()<<" cycle: "<<cbk->cycle());
            if (cbk->inputStream() == Stream && cbk->name() == cbk_name && cbk->cycle() > maxCycle) {
                maxCycle = cbk->cycle();
                all = cbk;
            }
        }
    } else ATH_MSG_WARNING("The CutBookkeepers are not present in the file ");
    if (!all) ATH_MSG_DEBUG("Failed to retrieve cut book keeper for Stream: "<<Stream<<" cbk_name: "<<cbk_name);
    return all;
}
StatusCode MetaDataAlg::beginInputFile() {
    //
    // This method is called at the start of each input file, even if
    // the input file contains no events. Accumulate metadata information here
    //
    const EventStreamInfo* esi = nullptr;
    ATH_CHECK(inputMetaStore()->retrieve(esi));
    if (esi->getEventTypes().size() > 1) { ATH_MSG_WARNING("There seem to be more event types than one"); }
    if (esi->getEventTypes().size() == 0) {
        ATH_MSG_FATAL("The EventTypes() container of the EventStreamInfo is empty! Something wrong with the input file?");
        return StatusCode::FAILURE;
    }

    bool isData = !esi->getEventTypes().begin()->test(EventType::IS_SIMULATION);

    const xAOD::FileMetaData* fmd = nullptr;
    if (inputMetaStore()->contains<xAOD::FileMetaData>("FileMetaData")) ATH_CHECK(inputMetaStore()->retrieve(fmd, "FileMetaData"));
    if (!fmd) {
        ATH_MSG_WARNING("FileMetaData not found in input file, setting m_isDerivedAOD=false.");
        // m_isDerivedAOD = false;
    } else {
        std::string dataType{};
        if (!(fmd->value(xAOD::FileMetaData::MetaDataType::dataType, dataType))) {
            ATH_MSG_WARNING("MetaDataType::dataType not found in xAOD::FileMetaData, setting m_isDerivedAOD=false.");
            // m_isDerivedAOD = false;
        }
        ATH_MSG_DEBUG("Data type is " << dataType);
    }
    const xAOD::CutBookkeeper* all{nullptr};
    std::string cbk_stream{};
    for (std::string trial: {"StreamAOD", "StreamESD", "unknownStream" }) {
         all = RetrieveCutBookKeeper(trial);
         cbk_stream = trial;
        if (all) break;
    }
    const bool contains_keeper = all != nullptr;
    if (!contains_keeper) {
        ATH_MSG_WARNING("No Common CutBook keeper has been found");
    }
    /// We run on data
    if (isData) {
        std::string trigger_stream{};
        ATH_CHECK(AthAnalysisHelper::retrieveMetadata("/TagInfo", "triggerStreamOfFile", trigger_stream));
        unsigned int run_number = (*esi->getRunNumbers().begin());
        std::vector<RunMetaData>::iterator itr =
            std::find_if(m_run_meta.begin(), m_run_meta.end(), [&run_number, &trigger_stream](const RunMetaData& meta) {
                return meta.run_number == run_number && trigger_stream == meta.trigger_stream;
            });
        /// If no run meta data is found then just create a new one
        if (itr == m_run_meta.end()) {
            m_run_meta.emplace_back(run_number, trigger_stream);
            itr = m_run_meta.end() - 1;
        }

        /// Copy the lumi block information
        for (const auto& Lumi : esi->getLumiBlockNumbers()) { itr->total_lumi_blocks.insert(Lumi); }
        itr->has_book_keeper = contains_keeper;
        if (contains_keeper) itr->tot_events += all->nAcceptedEvents();
    } else {
        int dsid = esi->getEventTypes().begin()->mc_channel_number();
        unsigned int run = (*esi->getRunNumbers().begin());
        ATH_MSG_DEBUG("DSID: "<<dsid<<" prw number: "<<run);
        /// Save the extra LHE weight variations
        std::vector<std::string> weight_names{};
        if (m_storeLHE) {
            std::map<std::string, int> weight_map{};
            AthAnalysisHelper::retrieveMetadata("/Generation/Parameters", "HepMCWeightNames", weight_map).isSuccess();
            weight_names.resize(weight_map.size());
            for (const auto& w_pair : weight_map) { weight_names[w_pair.second] = w_pair.first; }
            if (weight_names.empty()) {
                ATH_MSG_WARNING("No weight names extracted from the meta data ");
                weight_names.push_back("");
            }
        } else
            weight_names.push_back("");

        for (unsigned int lhe_var = 0; lhe_var < weight_names.size(); ++lhe_var) {
            const std::string& lhe_weight = weight_names[lhe_var];
            std::vector<SimMetaData>::iterator itr =
                std::find_if(m_sim_meta.begin(), m_sim_meta.end(), [&dsid, &run, &lhe_var](const SimMetaData& meta) {
                    return meta.mc_channel == dsid && meta.prw_channel == run && meta.variation_number == lhe_var;
                });
            if (itr == m_sim_meta.end()) {
                m_sim_meta.emplace_back(dsid, run);
                itr = m_sim_meta.end() - 1;
                itr->weight_name = weight_names[lhe_var];
                itr->variation_number = lhe_var;
                if (m_hasPrwTool) itr->prw_lumi = m_prwTool->expert()->GetIntegratedLumi(run, 0, -1);
            }

            const xAOD::CutBookkeeper* mc_keeper = lhe_var == 0 ? all : nullptr;
            if (lhe_var) {
                std::vector<std::string> cb_names;
                cb_names.emplace_back(lhe_weight);
                cb_names.emplace_back("LHE3Weight_" + ReplaceExpInString(lhe_weight, ".", ""));
                cb_names.push_back(ReplaceExpInString(cb_names.back(), " ", ""));
                cb_names.emplace_back("AllExecutedEvents_NonNominalMCWeight_" + std::to_string(lhe_var));
                for (const std::string& cb_name : cb_names) {
                    mc_keeper = RetrieveCutBookKeeper(cbk_stream, cb_name);
                    if (mc_keeper) {
                        ATH_MSG_DEBUG("Found LHE cut book keeper " << cb_name);
                        break;
                    }
                }
            }

            itr->has_book_keeper = mc_keeper != nullptr;

            if (itr->has_book_keeper) {
                itr->tot_events += mc_keeper->nAcceptedEvents();
                itr->sum_w += mc_keeper->sumOfEventWeights();
                itr->sum_w_squared += mc_keeper->sumOfEventWeightsSquared();
            }
        }
    }
    std::sort(m_sim_meta.begin(), m_sim_meta.end());
    std::sort(m_run_meta.begin(), m_run_meta.end());
    if (m_sim_meta.empty() == m_run_meta.empty()) {
        ATH_MSG_FATAL("Found run and simulation meta data");
        return StatusCode::FAILURE;
    }
    if (m_run_meta.empty() == m_isData) {
        ATH_MSG_FATAL("Metadata is inconsistent with the configuration");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}
StatusCode MetaDataAlg::fillDataTree(){

    ScalarBranch<Long64_t>& tot_ev {m_MetaDataTree->newScalar<Long64_t>("TotalEvents")};
    ScalarBranch<Long64_t>& proc_ev {m_MetaDataTree->newScalar<Long64_t>("ProcessedEvents")};
    ScalarBranch<unsigned int>& run_number{m_MetaDataTree->newScalar<unsigned int>("runNumber")};
    
    SetBranch<unsigned int>& tot_lumi{m_MetaDataTree->newSet<unsigned int>("TotalLumiBlocks")};
    SetBranch<unsigned int>& proc_lumi{m_MetaDataTree->newSet<unsigned int>("ProcessedLumiBlocks")};
    ScalarBranch<std::string>& stream_name{m_MetaDataTree->newScalar<std::string>("triggerStream")};
    ATH_CHECK(m_MetaDataTree->init(this));
    for (const RunMetaData& meta : m_run_meta) {
        run_number = meta.run_number;
        tot_ev = meta.tot_events;
        proc_ev = meta.proc_events;
        stream_name = meta.trigger_stream;
        tot_lumi = meta.total_lumi_blocks;
        proc_lumi = meta.processed_blocks;
        m_MetaDataTree->fill(Gaudi::Hive::currentContext());
    }    
    return StatusCode::SUCCESS;
}
StatusCode MetaDataAlg::fillSimulationTree() {
    m_MetaDataTree->newScalar<bool>("isAF2", m_isAF2);
    ScalarBranch<Long64_t>& tot_ev {m_MetaDataTree->newScalar<Long64_t>("TotalEvents")};
    ScalarBranch<Long64_t>& proc_ev {m_MetaDataTree->newScalar<Long64_t>("ProcessedEvents")};
    ScalarBranch<unsigned int>& run_number{m_MetaDataTree->newScalar<unsigned int>("runNumber")};
    
    ScalarBranch<double>& sum_w{m_MetaDataTree->newScalar<double>("TotalSumW")};
    ScalarBranch<double>& sum_w2{m_MetaDataTree->newScalar<double>("TotalSumW2")};
    ScalarBranch<double>& prw_lumi{m_MetaDataTree->newScalar<double>("prwLuminosity")};
    
    ScalarBranch<int>& mc_dsid{m_MetaDataTree->newScalar<int>("mcChannelNumber")};
    ScalarBranch<unsigned int>& lhe_var{m_MetaDataTree->newScalar<unsigned int>("LheId")};
    ScalarBranch<std::string>& stream_name{m_MetaDataTree->newScalar<std::string>("LheWeightName")};

    if (!m_hasPrwTool) m_MetaDataTree->disableBranch(prw_lumi.name());
    ATH_CHECK(m_MetaDataTree->init(this));
    
    for (const SimMetaData& meta : m_sim_meta) {
        run_number = meta.prw_channel;
        mc_dsid = meta.mc_channel;
        lhe_var = meta.variation_number;

        prw_lumi = meta.prw_lumi;
        sum_w = meta.sum_w;
        sum_w2 = meta.sum_w_squared;
        stream_name = meta.weight_name;

        tot_ev = meta.tot_events;
        proc_ev = meta.proc_events;
        m_MetaDataTree->fill(Gaudi::Hive::currentContext());
    }
    return StatusCode::SUCCESS;
}
}    
