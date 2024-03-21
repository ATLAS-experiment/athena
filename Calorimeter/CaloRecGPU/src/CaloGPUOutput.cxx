//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloGPUOutput.h"

#include "CaloRecGPU/StandaloneDataIO.h"

#include <unordered_map>

using namespace CaloRecGPU;

CaloGPUOutput::CaloGPUOutput(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  m_constantDataSaved(false)
{
  declareInterface<CaloClusterGPUProcessor> (this);
}

StatusCode CaloGPUOutput::execute(const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data, void * /*temporary_buffer*/) const
{
  if (!m_constantDataSaved.load())
    {
      std::lock_guard<std::mutex> lock_guard(m_mutex);
      if (!m_constantDataSaved.load())
        {
          const auto err1 = StandaloneDataIO::save_constants_to_folder(std::string(m_savePath), constant_data.m_geometry_dev,
                                                                       constant_data.m_cell_noise_dev, m_filePrefix, m_fileSuffix);
          if (err1 != StandaloneDataIO::ErrorState::OK)
            {
              return StatusCode::FAILURE;
            }
          m_constantDataSaved.store(true);
        }
    }

  Helpers::CPU_object<CellInfoArr> cell_info(event_data.m_cell_info_dev);

  if (m_onlyCellInfo)
    {
      const auto err2 = StandaloneDataIO::save_cell_info_to_folder(ctx.evt(), std::string(m_savePath), cell_info, m_filePrefix, m_fileSuffix, m_numWidth);

      if (err2 != StandaloneDataIO::ErrorState::OK)
        {
          return StatusCode::FAILURE;
        }

      return StatusCode::SUCCESS;
    }


  Helpers::CPU_object<CellStateArr> cell_state(event_data.m_cell_state_dev);
  Helpers::CPU_object<ClusterInfoArr> clusters(event_data.m_clusters_dev);

  std::unordered_map<int, int> tag_map;

  if (m_sortedAndCutClusters)
    {
      std::vector<int> cluster_order(clusters->number);

      std::iota(cluster_order.begin(), cluster_order.end(), 0);

      std::sort(cluster_order.begin(), cluster_order.end(), [&](const int a, const int b)
      {
        if (clusters->seedCellID[a] < 0)
          {
            return false;
            //This means that clusters with no cells
            //(marked as invalid) always compare lower,
            //so they appear in the end.
          }
        else if (clusters->seedCellID[b] < 0)
          {
            return true;
          }
        return clusters->clusterEt[a] > clusters->clusterEt[b];
      } );

      int real_cluster_numbers = clusters->number;

      for (size_t i = 0; i < cluster_order.size(); ++i)
        {
          const int this_id = cluster_order[i];
          if (clusters->seedCellID[this_id] < 0)
            {
              tag_map[this_id] = -1;
              --real_cluster_numbers;
            }
          else
            {
              tag_map[this_id] = i;
            }
        }

      const Helpers::CPU_object<ClusterInfoArr> temp_clusters(clusters);

      clusters->number = real_cluster_numbers;

      for (int i = 0; i < temp_clusters->number; ++i)
        {
          clusters->clusterEnergy[i] = temp_clusters->clusterEnergy[cluster_order[i]];
          clusters->clusterEt[i] = temp_clusters->clusterEt[cluster_order[i]];
          clusters->clusterEta[i] = temp_clusters->clusterEta[cluster_order[i]];
          clusters->clusterPhi[i] = temp_clusters->clusterPhi[cluster_order[i]];
          clusters->seedCellID[i] = temp_clusters->seedCellID[cluster_order[i]];
        }

    }
  size_t shared_count = 0;
  for (int i = 0; i < NCaloCells; ++i)
    {
      if (!cell_info->is_valid(i))
        {
          continue;
        }
      const ClusterTag this_tag = cell_state->clusterTag[i];
      if (!this_tag.is_part_of_cluster())
        {
          cell_state->clusterTag[i] = ClusterTag::make_invalid_tag();
        }
      else if (this_tag.is_part_of_cluster() && m_sortedAndCutClusters)
        {
          const int old_idx = this_tag.cluster_index();
          const int new_idx = tag_map[old_idx];
          const int old_idx2 = this_tag.is_shared_between_clusters() ? this_tag.secondary_cluster_index() : -1;
          const int new_idx2 = old_idx2 >= 0 ? tag_map[old_idx2] : -1;
          if (new_idx < 0 && new_idx2 < 0)
            {
              cell_state->clusterTag[i] = ClusterTag::make_invalid_tag();
            }
          else if (new_idx < 0)
            {
              cell_state->clusterTag[i] = ClusterTag::make_tag(new_idx2);
            }
          else if (new_idx2 < 0)
            {
              cell_state->clusterTag[i] = ClusterTag::make_tag(new_idx);
            }
          else
            {
              ++shared_count;
              cell_state->clusterTag[i] = ClusterTag::make_tag(new_idx, this_tag.secondary_cluster_weight(), new_idx2);
            }
        }
    }


  ATH_MSG_INFO("Clusters:     " << clusters->number << " (" << tag_map.size() << " total)");
  ATH_MSG_INFO("Shared Cells: " << shared_count);


  const auto err2 = StandaloneDataIO::save_event_to_folder(ctx.evt(), std::string(m_savePath), cell_info, cell_state, clusters,
                                                           m_filePrefix, m_fileSuffix, m_numWidth);

  if (err2 != StandaloneDataIO::ErrorState::OK)
    {
      return StatusCode::FAILURE;
    }

  return StatusCode::SUCCESS;


}
