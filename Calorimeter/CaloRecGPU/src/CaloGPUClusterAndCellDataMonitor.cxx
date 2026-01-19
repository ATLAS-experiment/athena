//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//


#ifndef CALORECGPU_DATA_MONITOR_EXTRA_PRINTOUTS

  #define CALORECGPU_DATA_MONITOR_EXTRA_PRINTOUTS 0

#endif

#include "CaloGPUClusterAndCellDataMonitor.h"
#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "StoreGate/DataHandle.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecUtilities.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include <map>
#include <numeric>
#include <algorithm>
#include <string_view>

using namespace CaloRecGPU;

CaloGPUClusterAndCellDataMonitor::CaloGPUClusterAndCellDataMonitor(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent),
  m_plottedVariablesInitialized(false)
{
}

StatusCode CaloGPUClusterAndCellDataMonitor::initialize()
{
  ATH_CHECK( m_cellsKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  const std::string this_name = this->name();

  const std::string algorithm_name_prefix = this_name.substr(0, this_name.rfind('.'));
  //This is so we take into account the fact that tools
  //are prefixed with the parent algorithm's name.

  auto final_string = [& algorithm_name_prefix](const std::string & unpref_str) -> std::string
  {
    return algorithm_name_prefix + "." + unpref_str;
  };

  const MatchingOptions opts = m_matchingOptions;

  m_min_similarity = opts.min_similarity;
  m_seed_weight = opts.seed_w;
  m_grow_weight = opts.grow_w;
  m_terminal_weight = opts.term_w;

  for (const auto & tool : m_toolsToPlot)
    {
      const std::string tool_name = final_string(tool.tool);
      m_toolToIdMap[tool_name] = tool.plot_id;
      m_toolsToCheckFor[tool_name] = -1;
    }

  auto add_tool_from_pair = [this](const std::string & name) -> int
  {
    if (!m_toolsToCheckFor.count(name))
      {
        m_toolsToCheckFor[name] = m_numToolsToKeep;
        m_toolToIdMap[name] = "";
        return m_numToolsToKeep++;
      }
    else
      {
        const int current = m_toolsToCheckFor[name];
        if (current >= 0)
          {
            return current;
          }
        else
          {
            m_toolsToCheckFor[name] = m_numToolsToKeep;
            return m_numToolsToKeep++;
          }
      }
  };

  for (const auto & pair : m_pairsToPlot)
    {
      const int first_index = add_tool_from_pair(final_string(pair.tool_ref));
      const int second_index = add_tool_from_pair(final_string(pair.tool_test));
      m_toolCombinations.emplace_back(pair_to_plot{first_index, second_index, pair.plot_id,
                                                   pair.match_in_energy,
                                                   pair.match_without_shared,
                                                   pair.match_perfectly});
    }

  ATH_CHECK( m_moniTool.retrieve() );

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUClusterAndCellDataMonitor::finalize_plots() const
{
  //Well, not do plots, just monitor the number of events and the total number of clusters...

  auto mon_num_events = Monitored::Scalar("num_events", m_numEvents);

  for (const auto & k_v : m_toolToIdMap)
    {
      auto mon_num_clust = Monitored::Scalar(k_v.second + "_num_total_clusters", m_numClustersPerTool.at(k_v.first).load());
    }

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots_start(const EventContext & /*ctx*/,
                                                                const ConstantDataHolder & /*constant_data*/,
                                                                const xAOD::CaloClusterContainer * /*cluster_collection_ptr*/) const
{
  if (!m_plottedVariablesInitialized.load())
    {
      std::lock_guard<std::mutex> lock_guard(m_mutex);
      if (!m_plottedVariablesInitialized.load())
        {
          CaloGPUClusterAndCellDataMonitor * dhis ATLAS_THREAD_SAFE = const_cast<CaloGPUClusterAndCellDataMonitor *>(this);
          //We have the mutex.
          //It's safe.
          ATH_CHECK( dhis->initialize_plotted_variables() );
          m_plottedVariablesInitialized.store(true);
        }
    }
  if (m_numToolsToKeep > 0)
    {
      m_storageHolder.get_one().resize(m_numToolsToKeep);
    }
  //Allocate a vector of data holders for this thread and resize it to the necessary size.

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots_end(const EventContext & ctx,
                                                              const ConstantDataHolder & constant_data,
                                                              const xAOD::CaloClusterContainer * /*cluster_collection_ptr*/) const
{
  ATH_MSG_INFO("");

  for (const auto & combination : m_toolCombinations)
    {
      if (combination.index_ref < 0 || combination.index_test < 0)
        {
          ATH_MSG_WARNING("Invalid tool combination, please check your configuration! " << combination.prefix);
          continue;
        }
      ATH_CHECK( add_combination(ctx, constant_data, combination.index_ref, combination.index_test, combination.prefix,
                                 combination.match_in_energy, combination.match_without_shared, combination.match_perfectly) );
    }

  ATH_MSG_INFO("");

  if (m_numToolsToKeep > 0)
    {
      m_storageHolder.release_one();
      //Release the tool storage.
    }

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots(const EventContext & ctx,
                                                          const ConstantDataHolder & constant_data,
                                                          const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                                          const CaloClusterCollectionProcessor * tool) const
{
  if (filter_tool_by_name(tool->name()))
    {
      SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);

      EventDataHolder ed;

      ed.allocate(false);

      ed.importCells(static_cast<const CaloCellContainer *>(&(*cell_collection)), m_missingCellsToFill);

      ed.importClusters(cluster_collection_ptr, MomentsOptionsArray::all(), false, true, true, true, m_missingCellsToFill);

      std::vector<int> cells_prefix_sum;

      ATH_CHECK(update_cell_representation(ctx, constant_data, ed.m_cell_info, ed.m_clusters, cells_prefix_sum));

      return add_data(ctx, constant_data, ed.m_cell_info, ed.m_clusters, cells_prefix_sum, tool->name());
    }
  else
    {
      return StatusCode::SUCCESS;
    }
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots(const EventContext & ctx,
                                                          const ConstantDataHolder & constant_data,
                                                          const xAOD::CaloClusterContainer * /*cluster_collection_ptr*/,
                                                          const EventDataHolder & event_data,
                                                          const ICaloClusterGPUInputTransformer * tool) const
{
  if (filter_tool_by_name(tool->name()))
    {
      CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> cell_info = event_data.m_cell_info_dev;
      CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> clusters = event_data.m_clusters_dev;

      std::vector<int> cells_prefix_sum;

      ATH_CHECK(update_cell_representation(ctx, constant_data, cell_info, clusters, cells_prefix_sum));

      return add_data(ctx, constant_data, cell_info, clusters, cells_prefix_sum, tool->name());
    }
  else
    {
      return StatusCode::SUCCESS;
    }
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots(const EventContext & ctx,
                                                          const ConstantDataHolder & constant_data,
                                                          const xAOD::CaloClusterContainer * /*cluster_collection_ptr*/,
                                                          const EventDataHolder & event_data,
                                                          const CaloClusterGPUProcessor * tool) const
{
  if (filter_tool_by_name(tool->name()))
    {
      CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> cell_info = event_data.m_cell_info_dev;
      CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> clusters = event_data.m_clusters_dev;

      std::vector<int> cells_prefix_sum;

      ATH_CHECK(update_cell_representation(ctx, constant_data, cell_info, clusters, cells_prefix_sum));

      return add_data(ctx, constant_data, cell_info, clusters, cells_prefix_sum, tool->name());
    }
  else
    {
      return StatusCode::SUCCESS;
    }
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots(const EventContext & ctx,
                                                          const ConstantDataHolder & constant_data,
                                                          const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                                          const EventDataHolder & /*event_data*/,
                                                          const ICaloClusterGPUOutputTransformer * tool) const
{
  if (filter_tool_by_name(tool->name()))
    {
      SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);

      EventDataHolder ed;

      ed.allocate(false);

      ed.importCells(static_cast<const CaloCellContainer *>(&(*cell_collection)), m_missingCellsToFill);

      ed.importClusters(cluster_collection_ptr, MomentsOptionsArray::all(), false, true, true, true, m_missingCellsToFill);

      std::vector<int> cells_prefix_sum;

      ATH_CHECK(update_cell_representation(ctx, constant_data, ed.m_cell_info, ed.m_clusters, cells_prefix_sum));

      return add_data(ctx, constant_data, ed.m_cell_info, ed.m_clusters, cells_prefix_sum, tool->name());
    }
  else
    {
      return StatusCode::SUCCESS;
    }
}

bool CaloGPUClusterAndCellDataMonitor::filter_tool_by_name(const std::string & tool_name) const
{
  ATH_MSG_DEBUG("Checking : '" << tool_name << "': " << m_toolsToCheckFor.count(tool_name));
  return m_toolsToCheckFor.count(tool_name) > 0;
}




StatusCode CaloGPUClusterAndCellDataMonitor::update_cell_representation(const EventContext &,
                                                                        const CaloRecGPU::ConstantDataHolder &,
                                                                        const CaloRecGPU::CellInfoArr * cell_info,
                                                                        CaloRecGPU::ClusterInfoArr * clusters,
                                                                        std::vector<int> & cells_prefix_sum) const
{
  if (!clusters->has_cells_per_cluster())
    {
      for (int i = 0; i <= clusters->number; ++i)
        {
          clusters->cellsPrefixSum[i] = 0;
        }
      
      for (int i = 0; i < clusters->number_cells; ++i)
        {
          clusters->get_extra_cell_info(i) = clusters->cells.tags[i];
          //We overwrite some non-calculated moments that we shouldn't even have
          //at this point...
        }
      
      const int old_num_cells = clusters->number_cells;
      clusters->number_cells = 0;
      for (int i = 0; i < old_num_cells; ++i)
        {
          ClusterTag this_tag = clusters->get_extra_cell_info(i);

          const int first_cluster = this_tag.is_part_of_cluster() ? this_tag.cluster_index() : -1;

          const int second_cluster = this_tag.is_part_of_cluster() && this_tag.is_shared_between_clusters() ? this_tag.secondary_cluster_index() : -1;

          const float secondary_cluster_weight = (this_tag.is_part_of_cluster() && this_tag.is_shared_between_clusters() ?
                                                  float_unhack(this_tag.secondary_cluster_weight()) : 0.f);

          if (second_cluster >= 0)
            {
              if (second_cluster >= clusters->number)
                {
                  ATH_MSG_WARNING("Impossible cell assignment: " << i << " " << second_cluster << " (" << std::hex << this_tag << std::dec << ")");
                }
              clusters->cells.indices[clusters->number_cells] = i;
              clusters->cellWeights[clusters->number_cells] = secondary_cluster_weight;
              clusters->clusterIndices[clusters->number_cells] = second_cluster;
              clusters->number_cells += 1;
              clusters->cellsPrefixSum[second_cluster + 1] += 1;
            }

          if (first_cluster >= 0)
            {
              if (second_cluster >= clusters->number)
                {
                  ATH_MSG_WARNING("Impossible cell assignment: " << i << " " << first_cluster << " (" << std::hex << this_tag << std::dec << ")");
                }
              clusters->cells.indices[clusters->number_cells] = i;
              clusters->cellWeights[clusters->number_cells] = 1.0f - secondary_cluster_weight;
              clusters->clusterIndices[clusters->number_cells] = first_cluster;
              clusters->number_cells += 1;
              clusters->cellsPrefixSum[first_cluster + 1] += 1;
            }
        }
      int prefix = 0;
      for (int i = 0; i <= clusters->number; ++i)
        {
          prefix += clusters->cellsPrefixSum[i];
          
          clusters->cellsPrefixSum[i] = prefix;
        }
    }

  //Do note that, from here on, the assignment between cells and clusters is broken:
  //we simply have a list of cells in clusters...

  std::vector<int> cell_orderer(clusters->number_cells);

  std::iota(cell_orderer.begin(), cell_orderer.end(), 0);

  std::sort(cell_orderer.begin(), cell_orderer.end(), [&](const int a, const int b)
  {
    if (clusters->cells.indices[a] == clusters->cells.indices[b])
      {
        return clusters->cellWeights[a] < clusters->cellWeights[b];
      }
    else
      {
        const int hash_ID_a = cell_info->get_hash_ID(clusters->cells.indices[a]);
        const int hash_ID_b = cell_info->get_hash_ID(clusters->cells.indices[b]);
        if (hash_ID_a < 0)
          {
            ATH_MSG_WARNING("Attempting to sort impossible cell: " << a << " " << hash_ID_a);
          }
        if (hash_ID_b < 0)
          {
            ATH_MSG_WARNING("Attempting to sort impossible cell: " << b << " " << hash_ID_b);
          }
        return hash_ID_a < hash_ID_b;
      }
  } );
  
  auto order_array = [&](auto * ptr)
  {
    std::vector<std::decay_t<decltype(ptr[0])>> prev(clusters->number_cells);
    
    for (int i = 0; i < clusters->number_cells; ++i)
      {
        prev[i] = ptr[i];
      }
    
    for (int i = 0; i < clusters->number_cells; ++i)
      {
        ptr[i] = prev[cell_orderer[i]];
      }
  };
  
  order_array(clusters->cells.indices);
  order_array(clusters->cellWeights);
  order_array(clusters->clusterIndices);
  
  cells_prefix_sum.clear();
  cells_prefix_sum.resize(NCaloCells + 1, 0);

  int prev_cell_index = -1;
  int cell_count = 0;

  for (int i = 0; i < clusters->number_cells; ++i)
    {
      const int this_cell_index = clusters->cells.indices[i];

      if (this_cell_index != prev_cell_index)
        {
          const int prev_cell_ID = cell_info->get_hash_ID(prev_cell_index);
          const int this_cell_ID = cell_info->get_hash_ID(this_cell_index);

          for (int j = prev_cell_ID + 1; j <= this_cell_ID; ++j)
            {
              cells_prefix_sum[j] = cell_count;
            }
        }

      ++cell_count;

      prev_cell_index = this_cell_index;
    }

  if (clusters->number_cells > 0)
    {
      for (int i = cell_info->get_hash_ID(clusters->cells.indices[clusters->number_cells - 1]); i <= NCaloCells; ++i)
        {
          cells_prefix_sum[i + 1] = cell_count;
        }
    }

  return StatusCode::SUCCESS;
}


//WeighMatch takes as arguments the cell index
//and two vectors of pairs of cluster index and cell weight,
//for the reference and test assignments.
//WeighMatch takes as arguments a bool that indicates
//whether this assignment is for test (false for reference),
//and a vector of pairs of cluster index and cell weight.
template <class WeighMatch, class WeighNotMatch>
static void build_similarity_map_helper(const CaloRecGPU::CellInfoArr & /*cell_info_1*/,
                                        const CaloRecGPU::CellInfoArr & /*cell_info_2*/,
                                        const std::vector<int> & cells_prefix_sum_1,
                                        const std::vector<int> & cells_prefix_sum_2,
                                        const CaloRecGPU::ClusterInfoArr & cluster_info_1,
                                        const CaloRecGPU::ClusterInfoArr & cluster_info_2,
                                        WeighMatch match,
                                        WeighNotMatch not_match)
{
  std::vector<std::pair<int, float>> cluster_weights_1, cluster_weights_2;
  
  for (unsigned int this_hash_ID = 0; this_hash_ID < NCaloCells; ++this_hash_ID)
    {
      cluster_weights_1.clear();
      cluster_weights_2.clear();
      
      for (int i = cells_prefix_sum_1[this_hash_ID]; i < cells_prefix_sum_1[this_hash_ID + 1]; ++i)
        {
          cluster_weights_1.push_back({cluster_info_1.clusterIndices[i], cluster_info_1.cellWeights[i] + 1e-8});
        }
      for (int i = cells_prefix_sum_2[this_hash_ID]; i < cells_prefix_sum_2[this_hash_ID + 1]; ++i)
        {
          cluster_weights_2.push_back({cluster_info_2.clusterIndices[i], cluster_info_2.cellWeights[i] + 1e-8});
        }
      
      if (cluster_weights_1.size() != 0 && cluster_weights_2.size() != 0)
        {
          match(this_hash_ID, cluster_weights_1, cluster_weights_2);
        }
      else if (cluster_weights_1.size() != 0)
        {
          not_match(false, this_hash_ID, cluster_weights_1);
        }
      else if (cluster_weights_2.size() != 0)
        {
          not_match(true, this_hash_ID, cluster_weights_2);
        }
    }
}

StatusCode CaloGPUClusterAndCellDataMonitor::match_clusters(sample_comparisons_holder & sch,
                                                            const CaloRecGPU::ConstantDataHolder & constant_data,
                                                            const CaloRecGPU::CellInfoArr & cell_info_1,
                                                            const CaloRecGPU::CellInfoArr & cell_info_2,
                                                            const std::vector<int> & cells_prefix_sum_1,
                                                            const std::vector<int> & cells_prefix_sum_2,
                                                            const CaloRecGPU::ClusterInfoArr & cluster_info_1,
                                                            const CaloRecGPU::ClusterInfoArr & cluster_info_2,
                                                            const bool match_in_energy,
                                                            const bool match_without_shared) const
{
  sch.r2t_table.clear();
  sch.r2t_table.resize(cluster_info_1.number, -1);

  sch.t2r_table.clear();
  sch.t2r_table.resize(cluster_info_2.number, -1);

  std::vector<double> similarity_map(cluster_info_1.number * cluster_info_2.number, 0.);

  std::vector<double> ref_normalization(cluster_info_1.number, 0.);
  std::vector<double> test_normalization(cluster_info_2.number, 0.);

  auto calculate_weight = [&](const int cell)
  {
    double SNR = 0.00001;

    if (!cell_info_1.is_bad(cell))
      {
        const int gain = cell_info_1.gain[cell];

        const double cellNoise = constant_data.m_cell_noise->get_noise(cell_info_1.get_hash_ID(cell), gain);
        if (std::isfinite(cellNoise) && cellNoise > 0.0f)
          {
            SNR = std::abs(cell_info_1.energy[cell] / cellNoise);
          }
      }

    const double quantity = ( match_in_energy ? std::abs(cell_info_1.energy[cell]) : SNR );
    const double weight = (quantity + 1e-7) *
                          ( SNR > m_seedThreshold ? (match_in_energy ? 1000 : m_seed_weight) :
                            (
                                    SNR > m_growThreshold ? (match_in_energy ? 950 : m_grow_weight) :
                                    (
                                            SNR > m_termThreshold ? (match_in_energy ? 900 : m_terminal_weight) : (match_in_energy ? 100 : 1e-8)
                                    )
                            )
                          );

    return weight + 1e-8;
  };

  auto matched_clusters = [&](const int hash_ID, const std::vector<std::pair<int, float>> & v1, const std::vector<std::pair<int, float>> & v2)
  {
#if CALORECGPU_DATA_MONITOR_EXTRA_PRINTOUTS && CALORECGPU_DATA_MONITOR_EXTRA_PRINTOUTS > 1
    msg(MSG::INFO) <<  "MATCH: " << hash_ID << " " << calculate_weight(cell_info_1.get_cell_with_hash_ID(hash_ID)) << " |";
    for (const auto & p : v1)
      {
        msg() << " (" << p.first << ", " << p.second << ")";
      }
    msg() << " |";
    for (const auto & p : v2)
      {
        msg() << " (" << p.first << ", " << p.second << ")";
      }
    msg() << endmsg;
#endif
    
    if (match_without_shared && (v1.size() > 1 || v2.size() > 1))
      {
        return;
      }

    const float weight = calculate_weight(cell_info_1.get_cell_with_hash_ID(hash_ID));

    for (const auto & p1 : v1)
      {
        for (const auto & p2 : v2)
          {
            similarity_map[p2.first * cluster_info_1.number + p1.first] += weight * p1.second * p2.second;
          }
      }

    for (const auto & p1 : v1)
      {
        ref_normalization[p1.first] += weight * p1.second * p1.second;
      }

    for (const auto & p2 : v2)
      {
        test_normalization[p2.first] += weight * p2.second * p2.second;
      }
  };

  auto unmatched_clusters = [&](const bool is_test, const int hash_ID, const std::vector<std::pair<int, float>> & v)
  {
#if CALORECGPU_DATA_MONITOR_EXTRA_PRINTOUTS
    msg(MSG::INFO) << "UNMATCH: " << hash_ID << " " << calculate_weight(cell_info_1.get_cell_with_hash_ID(hash_ID)) << " | " << is_test << " |";
    for (const auto & p : v)
      {
        msg() << " (" << p.first << ", " << p.second << ")";
      }
    msg() << endmsg;
#endif
    
    if (match_without_shared && v.size() > 1)
      {
        return;
      }

    const float weight = calculate_weight(cell_info_1.get_cell_with_hash_ID(hash_ID));

    std::vector<double> & normalization = (is_test ? test_normalization : ref_normalization);

    for (const auto & p : v)
      {
        normalization[p.first] += weight * p.second * p.second;
      }
  };

  build_similarity_map_helper(cell_info_1,        cell_info_2,
                              cells_prefix_sum_1, cells_prefix_sum_2,
                              cluster_info_1,     cluster_info_2,
                              matched_clusters,   unmatched_clusters);

  for (int testc = 0; testc < cluster_info_2.number; ++testc)
    {
      const double test_norm = test_normalization[testc] + double(test_normalization[testc] == 0.);
      for (int refc = 0; refc < cluster_info_1.number; ++refc)
        {
          const double ref_norm = ref_normalization[refc] + double(ref_normalization[refc] == 0.);
          similarity_map[testc * cluster_info_1.number + refc] /= std::sqrt(ref_norm * test_norm);
        }
    }

  //In essence, the Gale-Shapley Algorithm

  std::vector<std::vector<int>> sorted_GPU_matches;

  sorted_GPU_matches.reserve(cluster_info_2.number);

  for (int testc = 0; testc < cluster_info_2.number; ++testc)
    {
      std::vector<int> sorter(cluster_info_1.number);
      std::iota(sorter.begin(), sorter.end(), 0);

      std::sort(sorter.begin(), sorter.end(),
                [&](const int a, const int b)
      {
        const double a_weight = similarity_map[testc * cluster_info_1.number + a];
        const double b_weight = similarity_map[testc * cluster_info_1.number + b];
        return a_weight > b_weight;
      }
               );

      size_t wanted_size = 0;

      for (; wanted_size < sorter.size(); ++wanted_size)
        {
          const double match_weight = similarity_map[testc * cluster_info_1.number + sorter[wanted_size]];
          if (match_weight < m_min_similarity)
            {
              break;
            }
        }

      //Yeah, we could do a binary search for best worst-case complexity,
      //but we are expecting 1~2 similar clusters and the rest garbage,
      //so we're expecting only 1~2 iterations.
      //This actually means all that sorting is way way overkill,
      //but we must make sure in the most general case that this works...

      sorter.resize(wanted_size);

      sorted_GPU_matches.push_back(std::move(sorter));
    }

  int num_iter = 0;

  constexpr int max_iter = 32;

  std::vector<double> matched_weights(cluster_info_1.number, -1.);

  std::vector<size_t> skipped_matching(cluster_info_2.number, 0);

  for (int stop_counter = 0; stop_counter < cluster_info_2.number && num_iter < max_iter; ++num_iter)
    {
      stop_counter = 0;
      for (int testc = 0; testc < int(sorted_GPU_matches.size()); ++testc)
        {
          if (skipped_matching[testc] < sorted_GPU_matches[testc].size())
            {
              const int match_c = sorted_GPU_matches[testc][skipped_matching[testc]];
              const double match_weight = similarity_map[testc * cluster_info_1.number + match_c];
              if (match_weight >= m_min_similarity && match_weight > matched_weights[match_c])
                {
                  const int prev_match = sch.r2t_table[match_c];
                  if (prev_match >= 0)
                    {
                      ++skipped_matching[prev_match];
                      --stop_counter;
                    }
                  sch.r2t_table[match_c] = testc;
                  matched_weights[match_c] = match_weight;
                  ++stop_counter;
                }
              else
                {
                  ++skipped_matching[testc];
                }
            }
          else
            {
              ++stop_counter;
            }
        }
    }

  sch.unmatched_ref_list.clear();
  sch.unmatched_test_list.clear();

  for (size_t i = 0; i < sch.r2t_table.size(); ++i)
    {
      const int match = sch.r2t_table[i];
      if (match < 0)
        {
          sch.unmatched_ref_list.push_back(i);
        }
      else
        {
          sch.t2r_table[match] = i;
        }
    }

  for (size_t i = 0; i < sch.t2r_table.size(); ++i)
    {
      if (sch.t2r_table[i] < 0)
        {
          sch.unmatched_test_list.push_back(i);
        }
    }

  {
    char message_buffer[256];
    snprintf(message_buffer, 256,
             "%2d: %5d / %5d || %5d / %5d || %3d || %5d | %5d || %5d",
             num_iter,
             int(sch.r2t_table.size()) - int(sch.unmatched_ref_list.size()), int(sch.r2t_table.size()),
             int(sch.t2r_table.size()) - int(sch.unmatched_test_list.size()), int(sch.t2r_table.size()),
             int(sch.r2t_table.size()) - int(sch.t2r_table.size()),
             int(sch.unmatched_ref_list.size()),
             int(sch.unmatched_test_list.size()),
             int(sch.unmatched_ref_list.size()) - int(sch.unmatched_test_list.size())
            );
    ATH_MSG_INFO(message_buffer);
  }

  return StatusCode::SUCCESS;

}

StatusCode CaloGPUClusterAndCellDataMonitor::match_clusters_perfectly(sample_comparisons_holder & sch,
                                                                      const CaloRecGPU::ConstantDataHolder & /*constant_data*/,
                                                                      const CaloRecGPU::CellInfoArr & cell_info_1,
                                                                      const CaloRecGPU::CellInfoArr & cell_info_2,
                                                                      const std::vector<int> & cells_prefix_sum_1,
                                                                      const std::vector<int> & cells_prefix_sum_2,
                                                                      const CaloRecGPU::ClusterInfoArr & cluster_info_1,
                                                                      const CaloRecGPU::ClusterInfoArr & cluster_info_2,
                                                                      const bool match_without_shared) const
{
  sch.r2t_table.clear();
  sch.r2t_table.resize(cluster_info_1.number, -1);

  sch.t2r_table.clear();
  sch.t2r_table.resize(cluster_info_2.number, -1);

  std::vector<char> match_possibilities(cluster_info_1.number * cluster_info_2.number, 1);

  auto matched_clusters = [&]([[maybe_unused]] const int hash_ID, const std::vector<std::pair<int, float>> & v1, const std::vector<std::pair<int, float>> & v2)
  {
#if CALORECGPU_DATA_MONITOR_EXTRA_PRINTOUTS && CALORECGPU_DATA_MONITOR_EXTRA_PRINTOUTS > 1
    msg(MSG::INFO) << "MATCH: " << hash_ID << " <> |";
    for (const auto & p : v1)
      {
        msg() << " (" << p.first << ", " << p.second << ")";
      }
    msg() << " |";
    for (const auto & p : v2)
      {
        msg() << " (" << p.first << ", " << p.second << ")";
      }
    msg() << endmsg;
#endif
    
    if (match_without_shared && (v1.size() > 1 || v2.size() > 1))
      {
        return;
      }

    for (const auto & t_p : v2)
      {   
        for (int rc = 0; rc < cluster_info_1.number; ++rc)
          {
            bool is_possibility = false;

            for (const auto & p: v1)
              {
                if (p.first == rc)
                  {
                    is_possibility = true;
                    break;
                  }
              }

            if (is_possibility)
              {
                continue;
              }
            
            match_possibilities[t_p.first * cluster_info_1.number + rc] = 0;
          }
      }

    for (const auto & r_p : v1)
      {   
        for (int tc = 0; tc < cluster_info_2.number; ++tc)
          {
            bool is_possibility = false;

            for (const auto & p: v2)
              {
                if (p.first == tc)
                  {
                    is_possibility = true;
                    break;
                  }
              }

            if (is_possibility)
              {
                continue;
              }
            
            match_possibilities[tc * cluster_info_1.number + r_p.first] = 0;
          }
      }
  };

  auto unmatched_clusters = [&](const bool is_test, [[maybe_unused]] const int hash_ID, const std::vector<std::pair<int, float>> & v)
  {
#if CALORECGPU_DATA_MONITOR_EXTRA_PRINTOUTS
    msg(MSG::INFO) << "UNMATCH: " << hash_ID << " <> | " << is_test << " |";
    for (const auto & p : v)
      {
        msg() << " (" << p.first << ", " << p.second << ")";
      }
    msg() << endmsg;
#endif
    
    if (match_without_shared && v.size() > 1)
      {
        return;
      }

    const int cluster_number = is_test ? cluster_info_2.number : cluster_info_1.number;

    for (const auto & p : v)
      {
        for (int i = 0; i < cluster_number; ++i)
          {
            const int this_index = (is_test ? p.first * cluster_info_1.number + i : i * cluster_info_1.number + p.first);
            match_possibilities[this_index] = 0;
          }
      }
  };

  build_similarity_map_helper(cell_info_1,        cell_info_2,
                              cells_prefix_sum_1, cells_prefix_sum_2,
                              cluster_info_1,     cluster_info_2,
                              matched_clusters,   unmatched_clusters);
  
  for (int testc = 0; testc < cluster_info_2.number; ++testc)
    {
      for (int refc = 0; refc < cluster_info_1.number; ++refc)
        {
          if (match_possibilities[testc * cluster_info_1.number + refc] > 0)
            {
              sch.r2t_table[refc] = testc;
              sch.t2r_table[testc] = refc;
            }
        }
    }

  for (int refc = 0; refc < cluster_info_1.number; ++refc)
    {
      if (sch.r2t_table[refc] < 0)
        {
          sch.unmatched_ref_list.push_back(refc);
        }
    }

  for (int testc = 0; testc < cluster_info_2.number; ++testc)
    {
      if (sch.t2r_table[testc] < 0)
        {
          sch.unmatched_test_list.push_back(testc);
        }
    }

  {
    char message_buffer[256];
    snprintf(message_buffer, 256,
             "%2d: %5d / %5d || %5d / %5d || %3d || %5d | %5d || %5d",
             0,
             int(sch.r2t_table.size()) - int(sch.unmatched_ref_list.size()), int(sch.r2t_table.size()),
             int(sch.t2r_table.size()) - int(sch.unmatched_test_list.size()), int(sch.t2r_table.size()),
             int(sch.r2t_table.size()) - int(sch.t2r_table.size()),
             int(sch.unmatched_ref_list.size()),
             int(sch.unmatched_test_list.size()),
             int(sch.unmatched_ref_list.size()) - int(sch.unmatched_test_list.size())
            );
    ATH_MSG_INFO(message_buffer);
  }

  return StatusCode::SUCCESS;

}

namespace
{

  namespace ClusterProperties
  {
#define CALORECGPU_BASIC_CLUSTER_PROPERTY(NAME, ...)                                                                              \
  struct clusters_ ## NAME                                                                                                        \
  {                                                                                                                               \
    static std::string name()                                                                                                     \
    {                                                                                                                             \
      return # NAME;                                                                                                              \
    }                                                                                                                             \
    static double get_property([[maybe_unused]] const ConstantDataHolder & constant_data,                                         \
                               [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info,                                        \
                               [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info,                                  \
                               [[maybe_unused]] const std::vector<int> & cells_prefix_sum,                                        \
                               [[maybe_unused]] const int cluster_index                                   )                       \
    {                                                                                                                             \
      __VA_ARGS__                                                                                                                 \
    }                                                                                                                             \
  };

    CALORECGPU_BASIC_CLUSTER_PROPERTY(index, return cluster_index;)

    CALORECGPU_BASIC_CLUSTER_PROPERTY(E, return cluster_info.clusterEnergy[cluster_index] / CLHEP::MeV;)

    CALORECGPU_BASIC_CLUSTER_PROPERTY(abs_E, return std::abs(cluster_info.clusterEnergy[cluster_index]) / CLHEP::MeV;)

    CALORECGPU_BASIC_CLUSTER_PROPERTY(Et, return cluster_info.clusterEt[cluster_index] / CLHEP::MeV;)

    CALORECGPU_BASIC_CLUSTER_PROPERTY(eta, return cluster_info.clusterEta[cluster_index];)

    CALORECGPU_BASIC_CLUSTER_PROPERTY(phi, return cluster_info.clusterPhi[cluster_index];)
    
    CALORECGPU_BASIC_CLUSTER_PROPERTY(number_cells, return cluster_info.cellsPrefixSum[cluster_index + 1] - cluster_info.cellsPrefixSum[cluster_index];)

    //CALORECGPU_BASIC_CLUSTER_PROPERTY(time, return cluster_moments.time[cluster_index] / CLHEP::us;)

#define CALORECGPU_CLUSTER_MOMENT(...) CALORECGPU_CLUSTER_MOMENT_INNER(__VA_ARGS__, 1, 1)
#define CALORECGPU_CLUSTER_MOMENT_INNER(NAME, PROPERTY, UNIT, ...) CALORECGPU_BASIC_CLUSTER_PROPERTY(moments_ ## NAME, return cluster_info.moments. PROPERTY [cluster_index] / UNIT;)

    CALORECGPU_CLUSTER_MOMENT(time, time, CLHEP::us)
    CALORECGPU_CLUSTER_MOMENT(FIRST_PHI, firstPhi)
    CALORECGPU_CLUSTER_MOMENT(FIRST_ETA, firstEta)
    CALORECGPU_CLUSTER_MOMENT(SECOND_R, secondR)
    CALORECGPU_CLUSTER_MOMENT(SECOND_LAMBDA, secondLambda)
    CALORECGPU_CLUSTER_MOMENT(DELTA_PHI, deltaPhi)
    CALORECGPU_CLUSTER_MOMENT(DELTA_THETA, deltaTheta)
    CALORECGPU_CLUSTER_MOMENT(DELTA_ALPHA, deltaAlpha)
    CALORECGPU_CLUSTER_MOMENT(CENTER_X, centerX)
    CALORECGPU_CLUSTER_MOMENT(CENTER_Y, centerY)
    CALORECGPU_CLUSTER_MOMENT(CENTER_Z, centerZ)
    CALORECGPU_CLUSTER_MOMENT(CENTER_MAG, centerMag)
    CALORECGPU_CLUSTER_MOMENT(CENTER_LAMBDA, centerLambda)
    CALORECGPU_CLUSTER_MOMENT(LATERAL, lateral)
    CALORECGPU_CLUSTER_MOMENT(LONGITUDINAL, longitudinal)
    CALORECGPU_CLUSTER_MOMENT(ENG_FRAC_EM, engFracEM)
    CALORECGPU_CLUSTER_MOMENT(ENG_FRAC_MAX, engFracMax)
    CALORECGPU_CLUSTER_MOMENT(ENG_FRAC_CORE, engFracCore)
    CALORECGPU_CLUSTER_MOMENT(FIRST_ENG_DENS, firstEngDens)
    CALORECGPU_CLUSTER_MOMENT(SECOND_ENG_DENS, secondEngDens)
    CALORECGPU_CLUSTER_MOMENT(ISOLATION, isolation)
    CALORECGPU_CLUSTER_MOMENT(ENG_BAD_CELLS, engBadCells)
    CALORECGPU_CLUSTER_MOMENT(N_BAD_CELLS, nBadCells)
    CALORECGPU_CLUSTER_MOMENT(N_BAD_CELLS_CORR, nBadCellsCorr)
    CALORECGPU_CLUSTER_MOMENT(BAD_CELLS_CORR_E, badCellsCorrE)
    CALORECGPU_CLUSTER_MOMENT(BADLARQ_FRAC, badLArQFrac)
    CALORECGPU_CLUSTER_MOMENT(ENG_POS, engPos)
    CALORECGPU_CLUSTER_MOMENT(SIGNIFICANCE, significance)
    CALORECGPU_CLUSTER_MOMENT(CELL_SIGNIFICANCE, cellSignificance)
    CALORECGPU_CLUSTER_MOMENT(CELL_SIG_SAMPLING, cellSigSampling)
    CALORECGPU_CLUSTER_MOMENT(AVG_LAR_Q, avgLArQ)
    CALORECGPU_CLUSTER_MOMENT(AVG_TILE_Q, avgTileQ)
    CALORECGPU_CLUSTER_MOMENT(ENG_BAD_HV_CELLS, engBadHVCells)
    CALORECGPU_CLUSTER_MOMENT(N_BAD_HV_CELLS, nBadHVCells)
    CALORECGPU_CLUSTER_MOMENT(PTD, PTD)
    CALORECGPU_CLUSTER_MOMENT(MASS, mass)
    CALORECGPU_CLUSTER_MOMENT(EM_PROBABILITY, EMProbability)
    CALORECGPU_CLUSTER_MOMENT(HAD_WEIGHT, hadWeight)
    CALORECGPU_CLUSTER_MOMENT(OOC_WEIGHT, OOCweight)
    CALORECGPU_CLUSTER_MOMENT(DM_WEIGHT, DMweight)
    CALORECGPU_CLUSTER_MOMENT(TILE_CONFIDENCE_LEVEL, tileConfidenceLevel)
    CALORECGPU_CLUSTER_MOMENT(SECOND_TIME, secondTime)
    CALORECGPU_CLUSTER_MOMENT(VERTEX_FRACTION, vertexFraction)
    CALORECGPU_CLUSTER_MOMENT(NVERTEX_FRACTION, nVertexFraction)
    CALORECGPU_CLUSTER_MOMENT(ETACALOFRAME, etaCaloFrame)
    CALORECGPU_CLUSTER_MOMENT(PHICALOFRAME, phiCaloFrame)
    CALORECGPU_CLUSTER_MOMENT(ETA1CALOFRAME, eta1CaloFrame)
    CALORECGPU_CLUSTER_MOMENT(PHI1CALOFRAME, phi1CaloFrame)
    CALORECGPU_CLUSTER_MOMENT(ETA2CALOFRAME, eta2CaloFrame)
    CALORECGPU_CLUSTER_MOMENT(PHI2CALOFRAME, phi2CaloFrame)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_TOT, engCalibTot)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_OUT_L, engCalibOutL)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_OUT_M, engCalibOutM)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_OUT_T, engCalibOutT)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_L, engCalibDeadL)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_M, engCalibDeadM)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_T, engCalibDeadT)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_EMB0, engCalibEMB0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_EME0, engCalibEME0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_TILEG3, engCalibTileG3)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_TOT, engCalibDeadTot)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_EMB0, engCalibDeadEMB0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_TILE0, engCalibDeadTile0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_TILEG3, engCalibDeadTileG3)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_EME0, engCalibDeadEME0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_HEC0, engCalibDeadHEC0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_FCAL, engCalibDeadFCAL)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_LEAKAGE, engCalibDeadLeakage)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_UNCLASS, engCalibDeadUnclass)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_FRAC_EM, engCalibFracEM)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_FRAC_HAD, engCalibFracHad)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_FRAC_REST, engCalibFracRest)

    using BasicClusterProperties = multi_class_holder <

                                   clusters_index,
                                   clusters_E,
                                   clusters_abs_E,
                                   clusters_Et,
                                   clusters_eta,
                                   clusters_phi,
                                   clusters_number_cells,
                                   clusters_moments_time,
                                   clusters_moments_FIRST_PHI,
                                   clusters_moments_FIRST_ETA,
                                   clusters_moments_SECOND_R,
                                   clusters_moments_SECOND_LAMBDA,
                                   clusters_moments_DELTA_PHI,
                                   clusters_moments_DELTA_THETA,
                                   clusters_moments_DELTA_ALPHA,
                                   clusters_moments_CENTER_X,
                                   clusters_moments_CENTER_Y,
                                   clusters_moments_CENTER_Z,
                                   clusters_moments_CENTER_MAG,
                                   clusters_moments_CENTER_LAMBDA,
                                   clusters_moments_LATERAL,
                                   clusters_moments_LONGITUDINAL,
                                   clusters_moments_ENG_FRAC_EM,
                                   clusters_moments_ENG_FRAC_MAX,
                                   clusters_moments_ENG_FRAC_CORE,
                                   clusters_moments_FIRST_ENG_DENS,
                                   clusters_moments_SECOND_ENG_DENS,
                                   clusters_moments_ISOLATION,
                                   clusters_moments_ENG_BAD_CELLS,
                                   clusters_moments_N_BAD_CELLS,
                                   clusters_moments_N_BAD_CELLS_CORR,
                                   clusters_moments_BAD_CELLS_CORR_E,
                                   clusters_moments_BADLARQ_FRAC,
                                   clusters_moments_ENG_POS,
                                   clusters_moments_SIGNIFICANCE,
                                   clusters_moments_CELL_SIGNIFICANCE,
                                   clusters_moments_CELL_SIG_SAMPLING,
                                   clusters_moments_AVG_LAR_Q,
                                   clusters_moments_AVG_TILE_Q,
                                   clusters_moments_ENG_BAD_HV_CELLS,
                                   clusters_moments_N_BAD_HV_CELLS,
                                   clusters_moments_PTD,
                                   clusters_moments_MASS,
                                   clusters_moments_EM_PROBABILITY,
                                   clusters_moments_HAD_WEIGHT,
                                   clusters_moments_OOC_WEIGHT,
                                   clusters_moments_DM_WEIGHT,
                                   clusters_moments_TILE_CONFIDENCE_LEVEL,
                                   clusters_moments_SECOND_TIME,
                                   clusters_moments_VERTEX_FRACTION,
                                   clusters_moments_NVERTEX_FRACTION,
                                   clusters_moments_ETACALOFRAME,
                                   clusters_moments_PHICALOFRAME,
                                   clusters_moments_ETA1CALOFRAME,
                                   clusters_moments_PHI1CALOFRAME,
                                   clusters_moments_ETA2CALOFRAME,
                                   clusters_moments_PHI2CALOFRAME,
                                   clusters_moments_ENG_CALIB_TOT,
                                   clusters_moments_ENG_CALIB_OUT_L,
                                   clusters_moments_ENG_CALIB_OUT_M,
                                   clusters_moments_ENG_CALIB_OUT_T,
                                   clusters_moments_ENG_CALIB_DEAD_L,
                                   clusters_moments_ENG_CALIB_DEAD_M,
                                   clusters_moments_ENG_CALIB_DEAD_T,
                                   clusters_moments_ENG_CALIB_EMB0,
                                   clusters_moments_ENG_CALIB_EME0,
                                   clusters_moments_ENG_CALIB_TILEG3,
                                   clusters_moments_ENG_CALIB_DEAD_TOT,
                                   clusters_moments_ENG_CALIB_DEAD_EMB0,
                                   clusters_moments_ENG_CALIB_DEAD_TILE0,
                                   clusters_moments_ENG_CALIB_DEAD_TILEG3,
                                   clusters_moments_ENG_CALIB_DEAD_EME0,
                                   clusters_moments_ENG_CALIB_DEAD_HEC0,
                                   clusters_moments_ENG_CALIB_DEAD_FCAL,
                                   clusters_moments_ENG_CALIB_DEAD_LEAKAGE,
                                   clusters_moments_ENG_CALIB_DEAD_UNCLASS,
                                   clusters_moments_ENG_CALIB_FRAC_EM,
                                   clusters_moments_ENG_CALIB_FRAC_HAD,
                                   clusters_moments_ENG_CALIB_FRAC_REST

                                   >;

  }

  using ClusterProperties::BasicClusterProperties;

  namespace ExtraClusterComparisons
  {
#define CALORECGPU_COMPARED_CLUSTER_PROPERTY(NAME, ...)                                                                           \
  struct clusters_ ## NAME                                                                                                        \
  {                                                                                                                               \
    static std::string name()                                                                                                     \
    {                                                                                                                             \
      return # NAME;                                                                                                              \
    }                                                                                                                             \
    static double get_property([[maybe_unused]] const ConstantDataHolder & constant_data,                                         \
                               [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info_1,                                      \
                               [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info_1,                                \
                               [[maybe_unused]] const std::vector<int> & cells_prefix_sum_1,                                      \
                               [[maybe_unused]] const int cluster_index_1,                                                        \
                               [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info_2,                                      \
                               [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info_2,                                \
                               [[maybe_unused]] const std::vector<int> & cells_prefix_sum_2,                                      \
                               [[maybe_unused]] const int cluster_index_2)                                                        \
    {                                                                                                                             \
      __VA_ARGS__                                                                                                                 \
    }                                                                                                                             \
  };

    CALORECGPU_COMPARED_CLUSTER_PROPERTY(delta_phi_in_range,
                                         return Helpers::regularize_angle( Helpers::regularize_angle(cluster_info_2.clusterPhi[cluster_index_2]) -
                                                                           Helpers::regularize_angle(cluster_info_1.clusterPhi[cluster_index_1])    );
                                        )

    CALORECGPU_COMPARED_CLUSTER_PROPERTY(delta_R,
                                         const double delta_eta = cluster_info_2.clusterEta[cluster_index_2] - cluster_info_1.clusterEta[cluster_index_1];
                                         const double delta_phi = Helpers::regularize_angle( Helpers::regularize_angle(cluster_info_2.clusterPhi[cluster_index_2]) -
                                                                                             Helpers::regularize_angle(cluster_info_1.clusterPhi[cluster_index_1])    );
                                         return std::sqrt(delta_eta * delta_eta + delta_phi * delta_phi);

                                        )


    using ComparedClusterProperties = multi_class_holder <

                                      clusters_delta_phi_in_range,
                                      clusters_delta_R

                                      >;

  }

  using ExtraClusterComparisons::ComparedClusterProperties;

  namespace CellProperties
  {
#define CALORECGPU_BASIC_CELL_PROPERTY(NAME, ...)                                                                                 \
  struct cells_ ## NAME                                                                                                           \
  {                                                                                                                               \
    static std::string name()                                                                                                     \
    {                                                                                                                             \
      return # NAME;                                                                                                              \
    }                                                                                                                             \
    static double get_property([[maybe_unused]] const ConstantDataHolder & constant_data,                                         \
                               [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info,                                        \
                               [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info,                                  \
                               [[maybe_unused]] const std::vector<int> & cells_prefix_sum,                                        \
                               [[maybe_unused]] const int cell                                          )                         \
    {                                                                                                                             \
      __VA_ARGS__                                                                                                                 \
    }                                                                                                                             \
  };

    CALORECGPU_BASIC_CELL_PROPERTY(E, return cell_info.energy[cell] / CLHEP::MeV;)
    CALORECGPU_BASIC_CELL_PROPERTY(abs_E, return std::abs(cell_info.energy[cell]) / CLHEP::MeV; )

    CALORECGPU_BASIC_CELL_PROPERTY(gain, return cell_info.gain[cell];)

    CALORECGPU_BASIC_CELL_PROPERTY(noise, return constant_data.m_cell_noise->get_noise(cell_info.get_hash_ID(cell), cell_info.gain[cell]);)

    CALORECGPU_BASIC_CELL_PROPERTY(SNR, return cell_info.energy[cell] /
                                               protect_from_zero(constant_data.m_cell_noise->get_noise(cell_info.get_hash_ID(cell), cell_info.gain[cell]));
                                  )

    CALORECGPU_BASIC_CELL_PROPERTY(abs_SNR,  return std::abs(cell_info.energy[cell] /
                                                             protect_from_zero(constant_data.m_cell_noise->get_noise(cell_info.get_hash_ID(cell), cell_info.gain[cell])));)

    CALORECGPU_BASIC_CELL_PROPERTY(time, return cell_info.time[cell] / CLHEP::us;)

    CALORECGPU_BASIC_CELL_PROPERTY(index, return cell;)

    CALORECGPU_BASIC_CELL_PROPERTY(hash_ID, return cell_info.get_hash_ID(cell);)

    CALORECGPU_BASIC_CELL_PROPERTY(sampling, return constant_data.m_geometry->sampling(cell_info.get_hash_ID(cell));)


    CALORECGPU_BASIC_CELL_PROPERTY(x, return constant_data.m_geometry->x[cell_info.get_hash_ID(cell)] / CLHEP::cm; )
    CALORECGPU_BASIC_CELL_PROPERTY(y, return constant_data.m_geometry->y[cell_info.get_hash_ID(cell)] / CLHEP::cm; )
    CALORECGPU_BASIC_CELL_PROPERTY(z, return constant_data.m_geometry->z[cell_info.get_hash_ID(cell)] / CLHEP::cm; )

    CALORECGPU_BASIC_CELL_PROPERTY(phi, return constant_data.m_geometry->phi[cell_info.get_hash_ID(cell)];)

    CALORECGPU_BASIC_CELL_PROPERTY(eta, return constant_data.m_geometry->eta[cell_info.get_hash_ID(cell)];)

    CALORECGPU_BASIC_CELL_PROPERTY(number_of_clusters, return (cells_prefix_sum[cell + 1] - cells_prefix_sum[cell]);)

    CALORECGPU_BASIC_CELL_PROPERTY(primary_cluster_index,
    {
      float max_weight = 0.f;
      int max_index = 0;
      for (int i = cells_prefix_sum[cell]; i < cells_prefix_sum[cell + 1]; ++i)
        {
          const float this_weight = cluster_info.cellWeights[i];
          const int this_index = cluster_info.clusterIndices[i];
          if (this_weight > max_weight || (this_weight == max_weight && this_index > max_index))
            {
              max_weight = this_weight;
              max_index = this_index;
            }
        }
      return max_index;
    }
                                  )

    CALORECGPU_BASIC_CELL_PROPERTY(secondary_cluster_index,
    {
      float max_weight = 0.f, second_max_weight = 0.f;
      int max_index = 0, second_max_index = 0;
      for (int i = cells_prefix_sum[cell]; i < cells_prefix_sum[cell + 1]; ++i)
        {
          const float this_weight = cluster_info.cellWeights[i];
          const int this_index = cluster_info.clusterIndices[i];
          if (this_weight > max_weight || (this_weight == max_weight && this_index > max_index))
            {
              second_max_weight = max_weight;
              second_max_index = max_index;
              max_weight = this_weight;
              max_index = this_index;
            }
          else if (this_weight > second_max_weight || (this_weight == second_max_weight && this_index > second_max_index))
            {
              second_max_weight = this_weight;
              second_max_index = this_index;
            }
        }
      return second_max_index;
    }
                                  )

    CALORECGPU_BASIC_CELL_PROPERTY(primary_weight,
    {
      float max_weight = 0.f;
      int max_index = 0;
      for (int i = cells_prefix_sum[cell]; i < cells_prefix_sum[cell + 1]; ++i)
        {
          const float this_weight = cluster_info.cellWeights[i];
          const int this_index = cluster_info.clusterIndices[i];
          if (this_weight > max_weight || (this_weight == max_weight && this_index > max_index))
            {
              max_weight = this_weight;
              max_index = this_index;
            }
        }
      return max_weight;
    }
                                  )


    CALORECGPU_BASIC_CELL_PROPERTY(secondary_weight,
    {
      float max_weight = 0.f, second_max_weight = 0.f;
      int max_index = 0, second_max_index = 0;
      for (int i = cells_prefix_sum[cell]; i < cells_prefix_sum[cell + 1]; ++i)
        {
          const float this_weight = cluster_info.cellWeights[i];
          const int this_index = cluster_info.clusterIndices[i];
          if (this_weight > max_weight || (this_weight == max_weight && this_index > max_index))
            {
              second_max_weight = max_weight;
              second_max_index = max_index;
              max_weight = this_weight;
              max_index = this_index;
            }
          else if (this_weight > second_max_weight || (this_weight == second_max_weight && this_index > second_max_index))
            {
              second_max_weight = this_weight;
              second_max_index = this_index;
            }
        }
      return second_max_weight;
    }
                                  )

    using BasicCellProperties = multi_class_holder <

                                cells_E,
                                cells_abs_E,
                                cells_gain,
                                cells_noise,
                                cells_SNR,
                                cells_abs_SNR,
                                cells_time,
                                cells_index,
                                cells_hash_ID,
                                cells_sampling,
                                cells_x,
                                cells_y,
                                cells_z,
                                cells_phi,
                                cells_eta,
                                cells_number_of_clusters,
                                cells_primary_cluster_index,
                                cells_secondary_cluster_index,
                                cells_primary_weight,
                                cells_secondary_weight

                                >;

  }

  using CellProperties::BasicCellProperties;

  namespace CellTypes
  {
#define CALORECGPU_BASIC_CELL_TYPE(NAME, ...)                                                                                     \
  struct cell_type_ ## NAME                                                                                                       \
  {                                                                                                                               \
    static std::string name()                                                                                                     \
    {                                                                                                                             \
      return # NAME;                                                                                                              \
    }                                                                                                                             \
    static bool is_type([[maybe_unused]] const ConstantDataHolder & constant_data,                                                \
                        [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info,                                               \
                        [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info,                                         \
                        [[maybe_unused]] const std::vector<int> & cells_prefix_sum,                                               \
                        [[maybe_unused]] const int cell                                          )                                \
    {                                                                                                                             \
      __VA_ARGS__                                                                                                                 \
    }                                                                                                                             \
  };

    CALORECGPU_BASIC_CELL_TYPE(intracluster, return (cells_prefix_sum[cell + 1] > cells_prefix_sum[cell]);)

    CALORECGPU_BASIC_CELL_TYPE(extracluster, return (cells_prefix_sum[cell + 1] == cells_prefix_sum[cell]);)

    CALORECGPU_BASIC_CELL_TYPE(shared, return (cells_prefix_sum[cell + 1] > cells_prefix_sum[cell] + 1);)

    using BasicCellTypes = multi_class_holder <

                           cell_type_intracluster,
                           cell_type_extracluster,
                           cell_type_shared

                           >;

  }

  using CellTypes::BasicCellTypes;


  enum ExtraThingsToCalculate
  {
    ClusterSize = 0,
    ClusterComparedSize,
    DiffCells,
    SameECells,
    SameECellsCombined,
    SameSNRCells,
    SameSNRCellsCombined,
    ExtraThingsSize
  };
}

StatusCode CaloGPUClusterAndCellDataMonitor::initialize_plotted_variables()
{
  const std::vector<std::string> histo_strings = m_moniTool->histogramService()->getHists();
  //Small problem: other histograms with matching names.
  //Mitigated by the fact that we use cell_<property> and cluster_<property>...

  m_clusterPropertiesToDo.resize(BasicClusterProperties::size(), false);
  m_comparedClusterPropertiesToDo.resize(BasicClusterProperties::size(), false);
  m_extraComparedClusterPropertiesToDo.resize(ComparedClusterProperties::size(), false);
  m_cellPropertiesToDo.resize(BasicCellProperties::size(), false);
  m_comparedCellPropertiesToDo.resize(BasicCellProperties::size(), false);
  m_cellTypesToDo.resize(BasicCellTypes::size(), false);
  m_comparedCellTypesToDo.resize(BasicCellTypes::size(), false);
  m_extraThingsToDo.resize(ExtraThingsSize, false);

  auto string_contains = [](std::string_view container, std::string_view contained) -> bool
  {
    return container.find(contained) != std::string::npos;
  };

  auto search_lambda = [&](const auto & prop, const size_t count, bool & check,
                           const std::string & str, std::vector<bool> & to_do,
                           std::string_view prefix = "", std::string_view  suffix = "")
  {
    if (string_contains(str, std::string(prefix) + prop.name() + std::string(suffix)))
      {
        to_do[count] = true;
        check = true;
      }
  };

  for (const auto & str : histo_strings)
    {
      bool found = false;

      apply_to_multi_class(search_lambda, BasicCellProperties{}, found, str, m_cellPropertiesToDo, "_cell_");
      apply_to_multi_class(search_lambda, BasicCellTypes{}, found, str, m_cellTypesToDo, "_", "_cells");

      if (found)
        {
          m_doCells = true;
        }

      found = false;

      apply_to_multi_class(search_lambda, BasicClusterProperties{}, found, str, m_clusterPropertiesToDo, "_cluster_");

      if (found)
        {
          m_doClusters = true;
        }

      found = false;

      apply_to_multi_class(search_lambda, BasicCellProperties{}, found, str, m_comparedCellPropertiesToDo, "_cell_delta_");
      apply_to_multi_class(search_lambda, BasicCellProperties{}, found, str, m_comparedCellPropertiesToDo, "_cell_", "_ref");
      apply_to_multi_class(search_lambda, BasicCellProperties{}, found, str, m_comparedCellPropertiesToDo, "_cell_", "_test");
      apply_to_multi_class(search_lambda, BasicCellTypes{}, found, str, m_comparedCellTypesToDo, "num_ref_", "_cells");
      apply_to_multi_class(search_lambda, BasicCellTypes{}, found, str, m_comparedCellTypesToDo, "num_test_", "_cells");
      apply_to_multi_class(search_lambda, BasicCellTypes{}, found, str, m_comparedCellTypesToDo, "delta_", "_cells");

      if (found)
        {
          m_doCombinedCells = true;
        }

      found = false;

      apply_to_multi_class(search_lambda, BasicClusterProperties{}, found, str, m_comparedClusterPropertiesToDo, "_cluster_delta_", "");
      apply_to_multi_class(search_lambda, BasicClusterProperties{}, found, str, m_comparedClusterPropertiesToDo, "_cluster_", "_ref");
      apply_to_multi_class(search_lambda, BasicClusterProperties{}, found, str, m_comparedClusterPropertiesToDo, "_cluster_", "_test");
      apply_to_multi_class(search_lambda, ComparedClusterProperties{}, found, str, m_extraComparedClusterPropertiesToDo);

      if (found)
        {
          m_doCombinedClusters = true;
        }

      if ( string_contains(str, "cluster_size_ref")            ||
           string_contains(str, "cluster_size_test")           ||
           string_contains(str, "cluster_delta_size")          ||
           string_contains(str, "cluster_weighted_size_ref")   ||
           string_contains(str, "cluster_weighted_size_test")  ||
           string_contains(str, "cluster_delta_weighted_size")    )
        {
          m_extraThingsToDo[ClusterComparedSize] = true;
          m_doCombinedCells = true;
          m_doCombinedClusters = true;
        }
      else if ( string_contains(str, "cluster_size")          ||
                string_contains(str, "cluster_weighted_size")    )
        {
          m_extraThingsToDo[ClusterSize] = true;
          m_doCells = true;
          m_doClusters = true;
        }

      if (string_contains(str, "cluster_diff_cells"))
        {
          m_extraThingsToDo[DiffCells] = true;
          m_doCombinedCells = true;
          m_doCombinedClusters = true;
        }

      if ( string_contains(str, "_num_same_E_cells_ref")      ||
           string_contains(str, "_num_same_E_cells_test")     ||
           string_contains(str, "delta_num_same_E_cells")     ||
           string_contains(str, "_num_same_abs_E_cells_ref")  ||
           string_contains(str, "_num_same_abs_E_cells_test") ||
           string_contains(str, "delta_num_same_abs_E_cells")    )
        {
          m_extraThingsToDo[SameECellsCombined] = true;
          m_doCombinedCells = true;
        }
      else if ( string_contains(str, "_num_same_E_cells")     ||
                string_contains(str, "_num_same_abs_E_cells")    )
        {
          m_extraThingsToDo[SameECells] = true;
          m_doCells = true;
        }

      if ( string_contains(str, "_num_same_SNR_cells_ref")      ||
           string_contains(str, "_num_same_SNR_cells_test")     ||
           string_contains(str, "delta_num_same_SNR_cells")     ||
           string_contains(str, "_num_same_abs_SNR_cells_ref")  ||
           string_contains(str, "_num_same_abs_SNR_cells_test") ||
           string_contains(str, "delta_num_same_abs_SNR_cells")    )
        {
          m_extraThingsToDo[SameSNRCellsCombined] = true;
          m_doCombinedCells = true;
        }
      else if ( string_contains(str, "_num_same_SNR_cells")     ||
                string_contains(str, "_num_same_abs_SNR_cells")    )
        {
          m_extraThingsToDo[SameSNRCells] = true;
          m_doCells = true;
        }
    }

  return StatusCode::SUCCESS;

}


StatusCode CaloGPUClusterAndCellDataMonitor::add_data(const EventContext & /*ctx*/,
                                                      const ConstantDataHolder & constant_data,
                                                      const CaloRecGPU::CellInfoArr * cell_info,
                                                      const CaloRecGPU::ClusterInfoArr * clusters,
                                                      const std::vector<int> & cells_prefix_sum,
                                                      const std::string & tool_name) const
{
  m_numClustersPerTool[tool_name].fetch_add(clusters->number);

  const std::string prefix = m_toolToIdMap.at(tool_name);

  const int index = m_toolsToCheckFor.at(tool_name);

  if (index >= 0 && m_numToolsToKeep > 0)
    {
      std::vector<per_tool_storage> & store_vec = m_storageHolder.get_for_thread();
      store_vec[index].cell_info = *cell_info;
      store_vec[index].clusters = *clusters;
      store_vec[index].cells_prefix_sum = cells_prefix_sum;
    }

  if (prefix != "")
    //Tools that are not meant to be plotted individually
    //have the empty string as a prefix.
    {
      std::unordered_map<std::string, std::vector<double>> cluster_properties, cell_properties;

      std::unordered_map<std::string, long long int> cell_counts;

      cluster_properties["size"].resize(clusters->number, 0.);
      cluster_properties["weighted_size"].resize(clusters->number, 0.);

      long long int same_energy = 0, same_abs_energy = 0, same_snr = 0, same_abs_snr = 0;

      std::set<double> energies, snrs;

      if (m_doCells)
        {

          for (int cell = 0; cell < cell_info->number; ++cell)
            {
              if (!cell_info->is_valid(cell_info->hashID[cell]))
                {
                  continue;
                }

              apply_to_multi_class([&](const auto & prop, const size_t i)
              {
                if (m_cellPropertiesToDo[i])
                  {
                    cell_properties[prop.name()].push_back(prop.get_property(constant_data, *cell_info, *clusters, cells_prefix_sum, cell));
                  }
              }, BasicCellProperties{});

              apply_to_multi_class([&](const auto & prop, const size_t i)
              {
                if (m_cellTypesToDo[i])
                  {
                    cell_counts[prop.name()] += prop.is_type(constant_data, *cell_info, *clusters, cells_prefix_sum, cell);
                  }
              }, BasicCellTypes{});

              const float this_energy = cell_info->energy[cell];

              if (m_extraThingsToDo[SameECells])
                {

                  if (energies.count(this_energy))
                    {
                      ++same_energy;
                      ++same_abs_energy;
                    }
                  else if (energies.count(-this_energy))
                    {
                      ++same_abs_energy;
                    }
                  energies.insert(this_energy);
                }

              if (m_extraThingsToDo[SameSNRCells])
                {

                  const float this_snr = this_energy / protect_from_zero(constant_data.m_cell_noise->get_noise(cell_info->get_hash_ID(cell), cell_info->gain[cell]));

                  if (snrs.count(this_snr))
                    {
                      ++same_snr;
                      ++same_abs_snr;
                    }
                  else if (snrs.count(-this_snr))
                    {
                      ++same_abs_snr;
                    }
                  snrs.insert(this_snr);
                }
            }

          if (m_extraThingsToDo[ClusterSize])
            {
              for (int i = 0; i < clusters->number_cells; ++i)
                {
                  const int this_cluster = clusters->clusterIndices[i];
                  const float this_weight = clusters->cellWeights[i];

                  cluster_properties["size"][this_cluster] += 1;
                  cluster_properties["weighted_size"][this_cluster] += this_weight;
                }
            }
        }

      if (m_doClusters)
        {
          for (int cluster = 0; cluster < clusters->number; ++cluster)
            {
              apply_to_multi_class([&](const auto & prop, const size_t i)
              {
                if (m_clusterPropertiesToDo[i])
                  {
                    cluster_properties[prop.name()].push_back(prop.get_property(constant_data, *cell_info, *clusters, cells_prefix_sum, cluster));
                  }
              }, BasicClusterProperties{});
            }
        }

      using coll_type = decltype(Monitored::Collection("", std::declval<std::vector<double> &>()));
      using scalar_type = decltype(Monitored::Scalar("", std::declval<long long int>()));

      std::vector<coll_type> collections;
      std::vector<scalar_type> count_scalars;
      std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> cluster_group, cell_group, counts_group;

      collections.reserve(cluster_properties.size() + cell_properties.size());
      count_scalars.reserve(cell_counts.size());
      cluster_group.reserve(cluster_properties.size());
      cell_group.reserve(cell_properties.size());
      counts_group.reserve(cell_counts.size() + 5);

      auto mon_clus_num = Monitored::Scalar(prefix + "_num_clusters", clusters->number);
      auto mon_same_energy = Monitored::Scalar(prefix + "_num_same_E_cells", same_energy);
      auto mon_same_abs_energy = Monitored::Scalar(prefix + "_num_same_abs_E_cells", same_abs_energy);
      auto mon_same_snr = Monitored::Scalar(prefix + "_num_same_SNR_cells", same_snr);
      auto mon_same_abs_snr = Monitored::Scalar(prefix + "_num_same_abs_SNR_cells", same_abs_snr);

      counts_group.push_back(std::ref(mon_clus_num));
      counts_group.push_back(std::ref(mon_same_energy));
      counts_group.push_back(std::ref(mon_same_abs_energy));
      counts_group.push_back(std::ref(mon_same_snr));
      counts_group.push_back(std::ref(mon_same_abs_snr));

      //If we're not doing these plots,
      //we're still saving,
      //which is slightly inefficient, but.. let's not complicate.

      for (const auto & k_v : cluster_properties)
        {
          collections.emplace_back(Monitored::Collection(prefix + "_cluster_" + k_v.first, k_v.second));
          cluster_group.push_back(std::ref(collections.back()));
        }

      for (const auto & k_v : cell_properties)
        {
          collections.emplace_back(Monitored::Collection(prefix + "_cell_" + k_v.first, k_v.second));
          cell_group.push_back(std::ref(collections.back()));
        }

      for (const auto & k_v : cell_counts)
        {
          count_scalars.emplace_back(Monitored::Scalar(prefix + "_num_" + k_v.first + "_cells", k_v.second));
          counts_group.push_back(std::ref(count_scalars.back()));
        }

      // Taking a std::ref of the back() iterator above is safe because the vector
      // has been reserved with the correct number of elements.
      // cppcheck-suppress invalidContainer
      auto monitor_clusters = Monitored::Group(m_moniTool, cluster_group);
      auto monitor_cells = Monitored::Group(m_moniTool, cell_group);
      auto monitor_counts = Monitored::Group(m_moniTool, counts_group);

    }
  return StatusCode::SUCCESS;
}


StatusCode CaloGPUClusterAndCellDataMonitor::add_combination(const EventContext & /*ctx*/,
                                                             const CaloRecGPU::ConstantDataHolder & constant_data,
                                                             const int index_1,
                                                             const int index_2,
                                                             const std::string & prefix,
                                                             const bool match_in_energy,
                                                             const bool match_without_shared,
                                                             const bool match_perfectly) const
{

  //Note: Part of the work here is superfluous in the case
  //      where we are monitoring the tools individually too,
  //      but in the most generic case that is not guaranteed.
  //      Partially wasted work, but it's cleaner than the alternative...

  std::vector<per_tool_storage> & store_vec = m_storageHolder.get_for_thread();

  const CaloRecGPU::CellInfoArr & cell_info_1 = store_vec[index_1].cell_info;
  const CaloRecGPU::ClusterInfoArr & clusters_1 = store_vec[index_1].clusters;
  const std::vector<int> & cells_prefix_sum_1 = store_vec[index_1].cells_prefix_sum;

  const CaloRecGPU::CellInfoArr & cell_info_2 = store_vec[index_2].cell_info;
  const CaloRecGPU::ClusterInfoArr & clusters_2 = store_vec[index_2].clusters;
  const std::vector<int> & cells_prefix_sum_2 = store_vec[index_2].cells_prefix_sum;

  sample_comparisons_holder sch;

  if (match_perfectly)
    {
      ATH_CHECK( match_clusters_perfectly(sch, constant_data,
                                          cell_info_1,        cell_info_2,
                                          cells_prefix_sum_1, cells_prefix_sum_2,
                                          clusters_1,         clusters_2,
                                          match_without_shared) );
    }
  else
    {
      ATH_CHECK( match_clusters(sch, constant_data,
                                cell_info_1,        cell_info_2,
                                cells_prefix_sum_1, cells_prefix_sum_2,
                                clusters_1,         clusters_2,
                                match_in_energy, match_without_shared) );
    }

  std::unordered_map<std::string, std::vector<double>> cluster_properties, cell_properties;

  std::unordered_map<std::string, long long int> cell_counts;

  std::vector<double> ref_size_vec(clusters_1.number, 0.), test_size_vec(clusters_2.number, 0.),
      ref_weighted_size_vec(clusters_1.number, 0.), test_weighted_size_vec(clusters_2.number, 0.),
      ref_diff_cells(clusters_1.number, 0.), test_diff_cells(clusters_2.number, 0.),
      ref_diff_cells_weight(clusters_1.number, 0.), test_diff_cells_weight(clusters_2.number, 0.);
  //We can store integers up to 2^53 on a double...

  long long int same_energy_1 = 0, same_energy_2 = 0,
                same_abs_energy_1 = 0, same_abs_energy_2 = 0,
                same_snr_1 = 0, same_snr_2 = 0,
                same_abs_snr_1 = 0, same_abs_snr_2 = 0,
                same_cluster_cells_count = 0, diff_cluster_cells_count = 0;

  std::set<double> energies_1, energies_2, snrs_1, snrs_2;

  if (m_doCombinedCells)
    {
      for (int cell = 0; cell < NCaloCells; ++cell)
        //The way the validity is checked means we will
        //simply continue for all cells beyond cell_info_N->number.
        {
          if ( cell >= cell_info_1.number                       ||
               cell >= cell_info_2.number                       ||
               !cell_info_1.is_valid(cell_info_1.hashID[cell])  ||
               !cell_info_2.is_valid(cell_info_2.hashID[cell])      )
            {
              continue;
            }

          apply_to_multi_class([&](const auto & prop, const size_t i)
          {
            if (m_comparedCellPropertiesToDo[i])
              {
                const auto prop_1 = prop.get_property(constant_data, cell_info_1, clusters_1, cells_prefix_sum_1, cell);
                const auto prop_2 = prop.get_property(constant_data, cell_info_2, clusters_2, cells_prefix_sum_2, cell);

                cell_properties[prop.name() + "_ref"].push_back(prop_1);
                cell_properties[prop.name() + "_test"].push_back(prop_2);

                cell_properties["delta_" + prop.name()].push_back(prop_2 - prop_1);
                cell_properties["delta_" + prop.name() + "_rel_ref"].push_back((prop_2 - prop_1) / protect_from_zero(std::abs(prop_1)));
                cell_properties["delta_" + prop.name() + "_rel_test"].push_back((prop_2 - prop_1) / protect_from_zero(std::abs(prop_2)));
              }
          }, BasicCellProperties{});

          apply_to_multi_class([&](const auto & prop, const size_t i)
          {
            if (m_comparedCellTypesToDo[i])
              {
                const auto is_1 = prop.is_type(constant_data, cell_info_1, clusters_1, cells_prefix_sum_1, cell);
                const auto is_2 = prop.is_type(constant_data, cell_info_2, clusters_2, cells_prefix_sum_2, cell);

                cell_counts["num_" + prop.name() + "_cells_ref"] += is_1;
                cell_counts["num_" + prop.name() + "_cells_test"] += is_2;
                cell_counts["delta_num_" + prop.name() + "_cells"] += is_2 - is_1;
              }
          }, BasicCellTypes{});

          const float this_energy_1 = cell_info_1.energy[cell];
          const float this_energy_2 = cell_info_2.energy[cell];

          if (m_extraThingsToDo[SameECellsCombined])
            {
              if (energies_1.count(this_energy_1))
                {
                  ++same_energy_1;
                  ++same_abs_energy_1;
                }
              else if (energies_1.count(-this_energy_1))
                {
                  ++same_abs_energy_1;
                }
              energies_1.insert(this_energy_1);

              if (energies_2.count(this_energy_2))
                {
                  ++same_energy_2;
                  ++same_abs_energy_2;
                }
              else if (energies_2.count(-this_energy_2))
                {
                  ++same_abs_energy_2;
                }
              energies_2.insert(this_energy_2);
            }

          if (m_extraThingsToDo[SameSNRCellsCombined])
            {
              const float this_snr_1 = this_energy_1 / protect_from_zero(constant_data.m_cell_noise->get_noise(cell_info_1.get_hash_ID(cell), cell_info_1.gain[cell]));

              if (snrs_1.count(this_snr_1))
                {
                  ++same_snr_1;
                  ++same_abs_snr_1;
                }
              else if (snrs_1.count(-this_snr_1))
                {
                  ++same_abs_snr_1;
                }
              snrs_1.insert(this_snr_1);


              const float this_snr_2 = this_energy_2 / protect_from_zero(constant_data.m_cell_noise->get_noise(cell_info_2.get_hash_ID(cell), cell_info_2.gain[cell]));

              if (snrs_2.count(this_snr_2))
                {
                  ++same_snr_2;
                  ++same_abs_snr_2;
                }
              else if (snrs_2.count(-this_snr_2))
                {
                  ++same_abs_snr_2;
                }
              snrs_2.insert(this_snr_2);
            }

          if (m_extraThingsToDo[DiffCells] || m_extraThingsToDo[ClusterComparedSize])
            {
              bool cell_is_diff = false;

              for (int i = cells_prefix_sum_1[cell]; i < cells_prefix_sum_1[cell + 1] && i < cell_info_1.number; ++i)
                {
                  const int this_index = clusters_1.clusterIndices[i];
                  const float this_weight = clusters_1.cellWeights[i];
                  bool found_match = false;
                  for (int j = cells_prefix_sum_2[cell]; j < cells_prefix_sum_2[cell + 1] && j < cell_info_2.number; ++j)
                    {
                      if (this_index == sch.t2r(clusters_2.clusterIndices[j]))
                        {
                          found_match = true;
                          break;
                        }
                    }
                  ref_size_vec[this_index] += 1;
                  ref_weighted_size_vec[this_index] += this_weight;
                  if (!found_match)
                    {
                      ref_diff_cells[this_index] += 1;
                      ref_diff_cells_weight[this_index] += this_weight;
                      cell_is_diff = true;
                    }
                }

              for (int i = cells_prefix_sum_2[cell]; i < cells_prefix_sum_2[cell + 1] && i < cell_info_2.number; ++i)
                {
                  const int this_index = clusters_2.clusterIndices[i];
                  const float this_weight = clusters_2.cellWeights[i];
                  bool found_match = false;
                  for (int j = cells_prefix_sum_1[cell]; j < cells_prefix_sum_1[cell + 1] && j < cell_info_1.number; ++j)
                    {
                      if (this_index == sch.r2t(clusters_1.clusterIndices[j]))
                        {
                          found_match = true;
                          break;
                        }
                    }
                  test_size_vec[this_index] += 1;
                  test_weighted_size_vec[this_index] += this_weight;
                  if (!found_match)
                    {
                      test_diff_cells[this_index] += 1;
                      test_diff_cells_weight[this_index] += this_weight;
                      cell_is_diff = true;
                    }
                }

              if (cell_is_diff)
                {
#if CALORECGPU_DATA_MONITOR_EXTRA_PRINTOUTS
                  msg(MSG::INFO) << "Diff: " << cell << " |";
                  for (int i = cells_prefix_sum_1[cell]; i < cells_prefix_sum_1[cell + 1] && i < cell_info_1.number; ++i)
                    {
                      msg() << " {" << clusters_1.cellWeights[i] << ", " << clusters_1.clusterIndices[i] << " (" << sch.r2t(clusters_1.clusterIndices[i]) << ")}";
                    }
                  msg() << " |";
                  for (int i = cells_prefix_sum_2[cell]; i < cells_prefix_sum_2[cell + 1] && i < cell_info_2.number; ++i)
                    {
                      msg() << " {" << clusters_2.cellWeights[i] << ", " << clusters_2.clusterIndices[i] << " (" << sch.t2r(clusters_2.clusterIndices[i]) << ")}";
                    }
                  msg() << endmsg;
#endif
                  
                  ++diff_cluster_cells_count;
                }
              else if ((cells_prefix_sum_1[cell + 1] > cells_prefix_sum_1[cell]) || (cells_prefix_sum_2[cell + 1] > cells_prefix_sum_2[cell]))
                {
                  ++same_cluster_cells_count;
                }
            }
        }
    }

  if (m_doCombinedClusters)
    {

      for (int cluster = 0; cluster < clusters_1.number; ++cluster)
        {
          const int match = sch.r2t(cluster);
          if (match < 0)
            //The cluster isn't matched.
            {
              continue;
            }

          apply_to_multi_class([&](const auto & prop, const size_t i)
          {
            if (m_comparedClusterPropertiesToDo[i])
              {
                const auto prop_1 = prop.get_property(constant_data, cell_info_1, clusters_1, cells_prefix_sum_1, cluster);
                const auto prop_2 = prop.get_property(constant_data, cell_info_2, clusters_2, cells_prefix_sum_2, match);

                cluster_properties[prop.name() + "_ref"].push_back(prop_1);
                cluster_properties[prop.name() + "_test"].push_back(prop_2);

                cluster_properties["delta_" + prop.name()].push_back(prop_2 - prop_1);
                cluster_properties["delta_" + prop.name() + "_rel_ref"].push_back((prop_2 - prop_1) / protect_from_zero(std::abs(prop_1)));
                cluster_properties["delta_" + prop.name() + "_rel_test"].push_back((prop_2 - prop_1) / protect_from_zero(std::abs(prop_2)));
              }
          }, BasicClusterProperties{});

          apply_to_multi_class([&](const auto & prop, const size_t i)
          {
            if (m_extraComparedClusterPropertiesToDo[i])
              {
                cluster_properties[prop.name()].push_back(prop.get_property(constant_data, cell_info_1, clusters_1, cells_prefix_sum_1, cluster,
                                                                            cell_info_2, clusters_2, cells_prefix_sum_2, match));
              }
          }, ComparedClusterProperties{});

          if (m_extraThingsToDo[ClusterComparedSize])
            {
              cluster_properties["size_ref"].push_back(ref_size_vec[cluster]);
              cluster_properties["size_test"].push_back(test_size_vec[match]);
              cluster_properties["delta_size"].push_back(ref_size_vec[cluster] - test_size_vec[match]);
              cluster_properties["delta_size_rel_ref"].push_back((ref_size_vec[cluster] - test_size_vec[match]) / protect_from_zero(ref_size_vec[cluster]));
              cluster_properties["delta_size_rel_test"].push_back((ref_size_vec[cluster] - test_size_vec[match]) / protect_from_zero(test_size_vec[match]));

              cluster_properties["weighted_size_ref"].push_back(ref_weighted_size_vec[cluster]);
              cluster_properties["weighted_size_test"].push_back(test_weighted_size_vec[match]);
              cluster_properties["delta_weighted_size"].push_back(ref_weighted_size_vec[cluster] - test_weighted_size_vec[match]);
              cluster_properties["delta_weighted_size_rel_ref"].push_back((ref_weighted_size_vec[cluster] - test_weighted_size_vec[match]) / protect_from_zero(ref_weighted_size_vec[cluster]));
              cluster_properties["delta_weighted_size_rel_test"].push_back((ref_weighted_size_vec[cluster] - test_weighted_size_vec[match]) / protect_from_zero(test_weighted_size_vec[match]));
            }

          if (m_extraThingsToDo[DiffCells])
            {
              cluster_properties["diff_cells_ref"].push_back(ref_diff_cells[cluster]);
              cluster_properties["diff_cells_ref_rel_size"].push_back(ref_diff_cells[cluster] / protect_from_zero(ref_size_vec[cluster]));
              cluster_properties["diff_cells_test"].push_back(test_diff_cells[match]);
              cluster_properties["diff_cells_test_rel_size"].push_back(test_diff_cells[match] / protect_from_zero(test_size_vec[match]));
              cluster_properties["diff_cells"].push_back(ref_diff_cells[cluster] + test_diff_cells[match]);

              cluster_properties["weighted_diff_cells_ref"].push_back(ref_diff_cells_weight[cluster]);
              cluster_properties["weighted_diff_cells_ref_rel_size"].push_back(ref_diff_cells_weight[cluster] / protect_from_zero(ref_weighted_size_vec[cluster]));
              cluster_properties["weighted_diff_cells_test"].push_back(test_diff_cells_weight[match]);
              cluster_properties["weighted_diff_cells_test_rel_size"].push_back(test_diff_cells_weight[match] / protect_from_zero(test_weighted_size_vec[match]));
              cluster_properties["weighted_diff_cells"].push_back(ref_diff_cells_weight[cluster] + test_diff_cells_weight[match]);
            }
        }
    }

  using coll_type = decltype(Monitored::Collection("", std::declval<std::vector<double> &>()));
  using scalar_type = decltype(Monitored::Scalar("", std::declval<long long int>()));

  std::vector<coll_type> collections;
  std::vector<scalar_type> count_scalars;
  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> cluster_group, cell_group, counts_group;

  collections.reserve(cluster_properties.size() + cell_properties.size());
  count_scalars.reserve(cell_counts.size() + 6 * 3);
  cluster_group.reserve(cluster_properties.size());
  cell_group.reserve(cell_properties.size());
  counts_group.reserve(cell_counts.size() + 3 + 6 * 3);

  auto add_count_vars = [&](const std::string & name, const long long int ref_num, const long long int test_num)
  {
    count_scalars.emplace_back(Monitored::Scalar(prefix + "_" + name + "_ref", ref_num));
    counts_group.push_back(std::ref(count_scalars.back()));

    count_scalars.emplace_back(Monitored::Scalar(prefix + "_" + name + "_test", test_num));
    counts_group.push_back(std::ref(count_scalars.back()));  // cppcheck-suppress invalidContainer; reserve used

    count_scalars.emplace_back(Monitored::Scalar(prefix + "_delta_" + name, test_num - ref_num));
    counts_group.push_back(std::ref(count_scalars.back()));  // cppcheck-suppress invalidContainer; reserve used
  };

  add_count_vars("num_clusters", clusters_1.number, clusters_2.number);
  add_count_vars("num_unmatched_clusters", sch.ref_unmatched(), sch.test_unmatched());

  add_count_vars("num_same_E_cells", same_energy_1, same_energy_2);
  add_count_vars("num_same_abs_E_cells", same_abs_energy_1, same_abs_energy_2);
  add_count_vars("num_same_SNR_cells", same_snr_1, same_snr_2);
  add_count_vars("num_same_abs_SNR_cells", same_abs_snr_1, same_abs_snr_2);

  auto mon_total_unmatched = Monitored::Scalar(prefix + "_num_unmatched_clusters", sch.ref_unmatched() + sch.test_unmatched());
  auto mon_same_cluster_cell = Monitored::Scalar(prefix + "_same_cluster_cells", same_cluster_cells_count);
  auto mon_diff_cluster_cell = Monitored::Scalar(prefix + "_diff_cluster_cells", diff_cluster_cells_count);

  if(m_extraThingsToDo[DiffCells])
    {
      ATH_MSG_INFO("Different cells:                                                      " << diff_cluster_cells_count);
    }
  
  counts_group.push_back(std::ref(mon_total_unmatched));
  counts_group.push_back(std::ref(mon_same_cluster_cell));
  counts_group.push_back(std::ref(mon_diff_cluster_cell));

  for (const auto & k_v : cluster_properties)
    {
      collections.emplace_back(Monitored::Collection(prefix + "_cluster_" + k_v.first, k_v.second));
      cluster_group.push_back(std::ref(collections.back()));
    }

  for (const auto & k_v : cell_properties)
    {
      collections.emplace_back(Monitored::Collection(prefix + "_cell_" + k_v.first, k_v.second));
      cell_group.push_back(std::ref(collections.back()));
    }

  for (const auto & k_v : cell_counts)
    {
      count_scalars.emplace_back(Monitored::Scalar(prefix + "_" + k_v.first, k_v.second));
      counts_group.push_back(std::ref(count_scalars.back()));
    }

  // Taking a std::ref of the back() iterator above is safe because the vector
  // has been reserved with the correct number of elements.
  // cppcheck-suppress invalidContainer
  auto monitor_clusters = Monitored::Group(m_moniTool, cluster_group);
  auto monitor_cells = Monitored::Group(m_moniTool, cell_group);
  auto monitor_counts = Monitored::Group(m_moniTool, counts_group);

  return StatusCode::SUCCESS;
}
