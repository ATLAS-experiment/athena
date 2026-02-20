//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloGPUOutput.h"

#include "CaloRecGPU/StandaloneDataIO.h"

#include <unordered_map>

using namespace CaloRecGPU;

CaloGPUOutput::CaloGPUOutput(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent),
  m_constantDataSaved(false)
{
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


  Helpers::CPU_object<ClusterInfoArr> clusters(event_data.m_clusters_dev), final_clusters(true);

  //For every new index, the corresponding old index
  std::vector<int> cluster_order(clusters->number);

  //For every old index, the corresponding new index (if any)
  std::vector<int> reverse_cluster_order(clusters->number, -1);

  std::iota(cluster_order.begin(), cluster_order.end(), 0);

  if (m_sortedAndCutClusters)
    {
      std::sort(cluster_order.begin(), cluster_order.end(), [&](const int a, const int b)
      {
        if (clusters->seedCellIndex[a] < 0)
          {
            return false;
            //This means that clusters with no cells
            //(marked as invalid) always compare lower,
            //so they appear in the end.
          }
        else if (clusters->seedCellIndex[b] < 0)
          {
            return true;
          }
        return clusters->clusterEt[a] > clusters->clusterEt[b];
      } );

      unsigned int real_cluster_number = 0;

      for (real_cluster_number = 0; real_cluster_number < cluster_order.size(); ++real_cluster_number)
        {
          if (clusters->seedCellIndex[real_cluster_number] < 0)
            {
              break;
            }
          else
            {
              reverse_cluster_order[cluster_order[real_cluster_number]] = real_cluster_number;
            }
        }

      cluster_order.resize(real_cluster_number);

      final_clusters->number = real_cluster_number;
      final_clusters->has_deleted_clusters = false;
    }

  if (m_outputTags)
    {
      if (clusters->has_cells_per_cluster())
        {
          for (int i = 0; i < cell_info->number; ++i)
            {
              final_clusters->cells.tags[i] = 0;
            }
          for (int i = 0; i < clusters->number_cells; ++i)
            {
              const int real_cluster_index = reverse_cluster_order[clusters->clusterIndices[i]];

              if (real_cluster_index < 0)
                {
                  continue;
                }

              const int this_cell_index = clusters->cells.indices[i];

              const float weight = clusters->cellWeights[i];

              uint32_t weight_as_int = 0;
              std::memcpy(&weight_as_int, &weight, sizeof(float));
              //On the platforms we expect to be running this, it should be fine.
              //Still UB.
              //With C++20, we could do that bit-cast thing.

              if (weight_as_int == 0)
                {
                  weight_as_int = 1;
                  //Subnormal,
                  //but just to distinguish from
                  //a non-shared cluster.
                }

              const ClusterTag this_tag = final_clusters->cells.tags[this_cell_index];

              const int other_index = this_tag.is_part_of_cluster() ? this_tag.cluster_index() : -1;

              if (other_index < 0)
                {
                  if (weight < 0.5f)
                    {
                      final_clusters->cells.tags[this_cell_index] = ClusterTag::make_tag(real_cluster_index, weight_as_int, 0);
                    }
                  else
                    {
                      final_clusters->cells.tags[this_cell_index] = ClusterTag::make_tag(real_cluster_index);
                    }
                }
              else if (weight > 0.5f)
                {
                  final_clusters->cells.tags[this_cell_index] = ClusterTag::make_tag(real_cluster_index, this_tag.secondary_cluster_weight(), other_index);
                }
              else if (weight == 0.5f)
                //Unlikely, but...
                {
                  const int max_cluster = real_cluster_index > other_index ? real_cluster_index : other_index;
                  const int min_cluster = real_cluster_index > other_index ? other_index : real_cluster_index;
                  final_clusters->cells.tags[this_cell_index] = ClusterTag::make_tag(max_cluster, weight_as_int, min_cluster);
                }
              else /*if (weight < 0.5f)*/
                {
                  final_clusters->cells.tags[this_cell_index] = ClusterTag::make_tag(other_index, weight_as_int, real_cluster_index);
                }

              //All of this logic assumes a cell is shared by at most two clusters
              //with weights such that w_1 + w_2 = 1.
              //This is not necessarily valid with local calibrations.

            }
        }
      else
        {
          for (int i = 0; i < clusters->number_cells; ++i)
            {
              const ClusterTag this_tag = clusters->cells.tags[i];

              const int primary_index = ( this_tag.is_part_of_cluster() ?
                                          reverse_cluster_order[this_tag.cluster_index()] :
                                          -1
                                        );
              const int secondary_index = ( this_tag.is_part_of_cluster() && this_tag.is_shared_between_clusters() ?
                                            reverse_cluster_order[this_tag.secondary_cluster_index()] :
                                            -1
                                          );

              const unsigned int weight = this_tag.secondary_cluster_weight();

              if (primary_index >= 0 && secondary_index >= 0)
                {
                  final_clusters->cells.tags[i] = ClusterTag::make_tag(primary_index, weight, secondary_index);
                }
              else if (primary_index >= 0)
                {
                  final_clusters->cells.tags[i] = ClusterTag::make_tag(primary_index);
                }
              else if (secondary_index >= 0)
                {
                  final_clusters->cells.tags[i] = ClusterTag::make_tag(secondary_index);
                }
              else
                {
                  final_clusters->cells.tags[i] = 0;
                }
            }
        }

      final_clusters->number_cells = cell_info->number;
      final_clusters->state = ClusterInformationState::TagsWithBasicInfo;
    }
  else
    {
      auto update_prefix_sum = [&]()
      {
        int running_count = 0;
        for (int i = 1; i <= final_clusters->number; ++i)
          {
            running_count += final_clusters->cellsPrefixSum[i];
            final_clusters->cellsPrefixSum[i] = running_count;
          }
        final_clusters->number_cells = running_count;
      };

      for (int i = 0; i <= final_clusters->number; ++i)
        {
          final_clusters->cellsPrefixSum[i] = 0;
          if (i < final_clusters->number)
            {
              clusters->clusterIndices[i] = 0;
              //Temporary storage for the index within the cluster.
            }
        }

      if (!clusters->has_cells_per_cluster())
        {
          for (int i = 0; i < clusters->number_cells; ++i)
            {
              const ClusterTag this_tag = clusters->cells.tags[i];

              const int primary_index = ( this_tag.is_part_of_cluster() ?
                                          reverse_cluster_order[this_tag.cluster_index()] :
                                          -1
                                        );
              const int secondary_index = ( this_tag.is_part_of_cluster() && this_tag.is_shared_between_clusters() ?
                                            reverse_cluster_order[this_tag.secondary_cluster_index()] :
                                            -1
                                          );

              if (primary_index >= 0)
                {
                  final_clusters->cellsPrefixSum[primary_index + 1] += 1;
                }

              if (secondary_index >= 0)
                {
                  final_clusters->cellsPrefixSum[secondary_index + 1] += 1;
                }
            }

          update_prefix_sum();

          for (int i = 0; i < clusters->number_cells; ++i)
            {
              const ClusterTag this_tag = clusters->cells.tags[i];

              const int primary_index = ( this_tag.is_part_of_cluster() ?
                                          reverse_cluster_order[this_tag.cluster_index()] :
                                          -1
                                        );
              const int secondary_index = ( this_tag.is_part_of_cluster() && this_tag.is_shared_between_clusters() ?
                                            reverse_cluster_order[this_tag.secondary_cluster_index()] :
                                            -1
                                          );

              const unsigned int weight_as_int = this_tag.secondary_cluster_weight();

              float weight;

              std::memcpy(&weight, &weight_as_int, sizeof(float));

              if (primary_index >= 0)
                {
                  const int this_cell_start = final_clusters->cellsPrefixSum[primary_index];

                  int & this_cell_count = clusters->clusterIndices[primary_index];

                  ++this_cell_count;

                  const bool is_seed_cell = (i == clusters->seedCellIndex[this_tag.cluster_index()]);

                  const int this_index_in_cluster = this_cell_start + this_cell_count * is_seed_cell;

                  final_clusters->cells.indices[this_index_in_cluster] = i;
                  final_clusters->cellWeights[this_index_in_cluster] = 1.0f - weight;
                  final_clusters->clusterIndices[this_index_in_cluster] = primary_index;
                }

              if (secondary_index >= 0)
                {
                  const int this_cell_start = final_clusters->cellsPrefixSum[secondary_index];

                  int & this_cell_count = clusters->clusterIndices[secondary_index];

                  ++this_cell_count;

                  const bool is_seed_cell = (i == clusters->seedCellIndex[this_tag.secondary_cluster_index()]);

                  const int this_index_in_cluster = this_cell_start + this_cell_count * is_seed_cell;

                  final_clusters->cells.indices[this_index_in_cluster] = i;
                  final_clusters->cellWeights[this_index_in_cluster] = weight;
                  final_clusters->clusterIndices[this_index_in_cluster] = primary_index;
                }
            }
        }
      else
        {
          final_clusters->cellsPrefixSum[0] = 0;

          for (unsigned int i = 0; i < cluster_order.size(); ++i)
            {
              const int original_cluster = cluster_order[i];
              final_clusters->cellsPrefixSum[i + 1] = clusters->cellsPrefixSum[original_cluster + 1] - clusters->cellsPrefixSum[original_cluster];
            }

          update_prefix_sum();

          for (unsigned int i = 0; i < cluster_order.size(); ++i)
            {
              const int original_cluster = cluster_order[i];

              int new_index = final_clusters->cellsPrefixSum[i];

              for (int j = clusters->cellsPrefixSum[original_cluster]; j < clusters->cellsPrefixSum[original_cluster + 1]; ++j, ++new_index)
                {
                  final_clusters->cells.indices[new_index] = clusters->cells.indices[j];
                  final_clusters->cellWeights[new_index] = clusters->cellWeights[j];
                  final_clusters->clusterIndices[new_index] = clusters->clusterIndices[j];
                }
            }

        }

      final_clusters->state = ClusterInformationState::WithBasicInfo;
    }

  for (unsigned int i = 0; i < cluster_order.size(); ++i)
    {
      final_clusters->clusterEnergy[i] = clusters->clusterEnergy[cluster_order[i]];
      final_clusters->clusterEt[i] = clusters->clusterEt[cluster_order[i]];
      final_clusters->clusterEta[i] = clusters->clusterEta[cluster_order[i]];
      final_clusters->clusterPhi[i] = clusters->clusterPhi[cluster_order[i]];
      final_clusters->seedCellIndex[i] = clusters->seedCellIndex[cluster_order[i]];
    }

  const auto err2 = StandaloneDataIO::save_event_to_folder(ctx.evt(), std::string(m_savePath), cell_info, final_clusters,
                                                           m_filePrefix, m_fileSuffix, m_numWidth);

  if (err2 != StandaloneDataIO::ErrorState::OK)
    {
      return StatusCode::FAILURE;
    }

  return StatusCode::SUCCESS;

}
