/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <src/MetadataAlg.h>

#include "HDF5Utils/Writer.h"
#include "HDF5Utils/histogram.h"
#include <boost/histogram.hpp>

#include <xAODCutFlow/CutBookkeeper.h>
#include <xAODCutFlow/CutBookkeeperContainer.h>
#include <xAODMetaData/FileMetaData.h>
#include <xAODTruth/TruthMetaData.h>
#include <xAODTruth/TruthMetaDataContainer.h>

#include <regex>
#include <fstream>
#include <boost/json.hpp>

//
// method implementations
//

namespace {

  // name for top level object in json or h5
  static const std::string top_group = "cutBookkeeper";

  void addCounts(H5::Group& grp, const OriginalAodCounts& counts){
    H5Utils::Consumers<const OriginalAodCounts&> cons;
#define ADD(NAME)                                                 \
    cons.add(#NAME,[](const OriginalAodCounts& c) {return c.NAME;})
    ADD(nEventsProcessed);
    ADD(sumOfWeights);
    ADD(sumOfWeightsSquared);
#undef ADD
    H5Utils::Writer<0, const OriginalAodCounts&> writer(grp, "counts", cons);
    writer.fill(counts);
  }
  void addCounts(boost::json::object& grp, const OriginalAodCounts& counts) {
    grp["counts"] = {
#define ADD(NAME) {#NAME, counts.NAME}
      ADD(nEventsProcessed),
      ADD(sumOfWeights),
      ADD(sumOfWeightsSquared),
#undef ADD
    };
  }

  OriginalAodCounts getCounts(const xAOD::CutBookkeeper& cbk)
  {
    OriginalAodCounts counts;
    counts.nEventsProcessed  = cbk.nAcceptedEvents();
    counts.sumOfWeights        = cbk.sumOfEventWeights();
    counts.sumOfWeightsSquared = cbk.sumOfEventWeightsSquared();
    return counts;
  }

}

namespace ftag {

  MetadataAlg::MetadataAlg(
    const std::string& name,
    ISvcLocator* pSvcLocator):
    AthAlgorithm(name, pSvcLocator),
    m_inputMetaStore("StoreGateSvc/InputMetaDataStore", name)
  {
  }

  bool MetadataAlg::isGoodBook(const xAOD::CutBookkeeper& cbk) const
  {
    return cbk.name() == "AllExecutedEvents"
      && m_allowed_streams.value().contains(cbk.inputStream());
  }

  StatusCode MetadataAlg::initialize()
  {
    // register with incident service
    ServiceHandle<IIncidentSvc> incSvc( "IncidentSvc", name() );
    CHECK( incSvc.retrieve() );
    // remove first to make sure it's not called twice
    incSvc->removeListener( this, IncidentType::BeginInputFile );
    incSvc->addListener( this, IncidentType::BeginInputFile, 0, true );

    CHECK(m_truthWeightTool.retrieve());

    if (!m_output_svc.empty()) {
      ATH_CHECK(m_output_svc.retrieve());
    }
    if (!m_hist_output_svc.empty()) {
      ATH_CHECK(m_hist_output_svc.retrieve());
    }

    return StatusCode::SUCCESS;
  }

  StatusCode MetadataAlg::execute(const EventContext& /*ctx*/)
  {
    return StatusCode::SUCCESS;
  }

  void MetadataAlg::handle(const Incident& inc)
  {

    // skip all incidents beyond the start of input files
    if (inc.type() != IncidentType::BeginInputFile) return;

    ATH_MSG_DEBUG("Updating CutBookkeeper information");

    // Retrieve complete CutBookkeeperContainer
    const xAOD::CutBookkeeperContainer *completeCBC{};
    auto rc = m_inputMetaStore->retrieve(completeCBC, "CutBookkeepers");
    if (!rc.isSuccess()) throw std::runtime_error(
      "could not retrieve CutBookkeepers");

    // Find the max cycle
    int maxCycle{-1};
    const xAOD::CutBookkeeper *allEvents{};
    for (const xAOD::CutBookkeeper *cbk : *completeCBC)
    {
      ATH_MSG_DEBUG(
        "Complete cbk name: " << cbk->name() <<
        " - stream: " << cbk->inputStream()
        );

      if (cbk->cycle() > maxCycle && isGoodBook(*cbk))
      {
        allEvents = cbk;
        maxCycle = cbk->cycle();
      }
    }

    if (allEvents == nullptr)
    {
      throw std::runtime_error(
        "Could not find AllExecutedEvents CutBookkeeper information.");
    }

    for (const xAOD::CutBookkeeper *cbk : *completeCBC)
    {
      if (cbk->cycle() == maxCycle && isGoodBook(*cbk))
      {
        static const std::regex re("AllExecutedEvents.*_([0-9]+)");
        // Get the CBK index
        size_t index{0};
        std::smatch match;
        if (std::regex_match(cbk->name(), match, re))
        {
          index = std::stoi(match[1]);
        }
        m_weights[index] += getCounts(*cbk);
      }
    }

    // now try systematics-aware containers
    for (size_t index{1}; index < m_truthWeightTool->getWeightNames().size(); ++index)
    {
      std::string cbkName = "CutBookkeepers_weight_" + std::to_string(index);
      if (!m_inputMetaStore->contains<xAOD::CutBookkeeperContainer>(cbkName))
      {
        ATH_MSG_VERBOSE("No container named " << cbkName << "available");
        continue;
      }

      if (!m_inputMetaStore->retrieve(completeCBC, cbkName).isSuccess()) {
        throw std::runtime_error("could not retrieve " + cbkName);
      }
      for (const xAOD::CutBookkeeper *cbk : *completeCBC)
      {
        if (cbk->cycle() == maxCycle && isGoodBook(*cbk))
        {
          m_weights[index] += getCounts(*cbk);
        }
      }
    }
  }


  StatusCode MetadataAlg::finalize ()
  {

    if (m_weights.empty()) {
      ATH_MSG_WARNING("No CutBookkeeper weights collected; skipping output.");
      return StatusCode::SUCCESS;
    }

    std::vector<CP::SystematicSet> systematics;
    systematics.emplace_back();               // nominal always first
    if (m_enable_systematics) {
      for (const CP::SystematicVariation& v :
           m_truthWeightTool->affectingSystematics()) {
        ATH_MSG_DEBUG(
          "using systematic " << CP::SystematicSet({v}).name());
        systematics.emplace_back(CP::SystematicSet({v}));
      }
    }

    std::optional<H5::Group> h5_cbk;
    if (!m_output_svc.empty()) {
      h5_cbk = H5::Group(m_output_svc->group()->createGroup(top_group));
    }
    std::optional<boost::json::object> json_cbk;
    if (!m_json_output.empty()) {
      json_cbk = boost::json::object{};
    }

    for (const CP::SystematicSet &sys : systematics)
    {
      const std::string sysname = sys.name().empty() ? "nominal": sys.name();
      const OriginalAodCounts &weights = m_weights.at(
        m_truthWeightTool->getSysWeightIndex(sys));

      if (h5_cbk) {
        H5::Group sysgroup(h5_cbk->createGroup(sysname));
        addCounts(sysgroup, weights);
      }
      if (json_cbk) {
        boost::json::object& cbk_root = *json_cbk;
        addCounts(cbk_root[sysname].emplace_object(), weights);
      }
    }
    if (json_cbk) {
      std::ofstream out(m_json_output.value());
      boost::json::object jroot {
        {top_group, *json_cbk}
      };
      out << jroot << std::endl;
    }

    if (!m_hist_output_svc.empty()) {
      namespace bh = boost::histogram;

      // Build index->name map and index list in one pass.
      using sys_map_t = std::map<size_t, std::string>;
      sys_map_t sys_map;
      std::vector<size_t> indices;
      for (const CP::SystematicSet& sys : systematics) {
        size_t idx = m_truthWeightTool->getSysWeightIndex(sys);
        sys_map[idx] = sys.name().empty() ? "nominal" : sys.name();
        indices.push_back(idx);
      }

      // Categorical axis: bin value = index, metadata = map for labels.
      using map_meta_t = std::pair<std::string, sys_map_t>;
      using sys_ax_t = bh::axis::category<size_t, map_meta_t>;
      const sys_ax_t ax(indices, map_meta_t{"systematic", sys_map});

      // Weighted histogram: bin=(sumOfWeights, sumOfWeightsSquared).
      auto h_w = bh::make_weighted_histogram(ax);

      // Integer histogram: nEventsProcessed.
      using int64_storage = bh::dense_storage<int64_t>;
      auto h_n = bh::make_histogram_with(int64_storage{}, ax);

      // Fill by systematic index.
      for (size_t idx : indices) {
        const auto& w = m_weights.at(idx);
        const auto bin = ax.index(idx);
        h_w.at(bin) =
          bh::accumulators::weighted_sum<double>(
            w.sumOfWeights, w.sumOfWeightsSquared);
        h_n.at(bin) =
          static_cast<int64_t>(w.nEventsProcessed);
      }

      H5::Group hist_grp(
        m_hist_output_svc->group()
          ->createGroup("cutBookkeeperHists"));
      H5Utils::hist::write_hist_to_group(
        hist_grp, h_w, "sumOfWeights");
      H5Utils::hist::write_hist_to_group(
        hist_grp, h_n, "nEventsProcessed");
    }

    return StatusCode::SUCCESS;
  }

}
