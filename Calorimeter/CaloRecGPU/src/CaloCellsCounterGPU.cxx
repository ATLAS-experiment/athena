//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloCellsCounterGPU.h"
#include "CaloRecGPU/StandaloneDataIO.h"

#include <map>
#include <vector>
#include <filesystem>

#include "FPHelpers.h"

using namespace CaloRecGPU;

CaloCellsCounterGPU::CaloCellsCounterGPU(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent)
{
}


namespace {


struct size_struct
{
  unsigned int total = 0, seed = 0, grow = 0, term = 0, invalid = 0, shared = 0;
  template <class Str>
  friend Str & operator << (Str & s, const size_struct & sst)
  {
    s << sst.total << " " << sst.seed << " " << sst.grow << " " << sst.term << " " << sst.invalid << " " << sst.shared;
    return s;
  }
};

struct cluster_info_struct
{
  size_struct size;
  float seed_snr = -9e99;
  float seed_energy = -9e99;
  template <class Str>
  friend Str & operator << (Str & s, const cluster_info_struct & cis)
  {
    s << cis.size << " (" << cis.seed_snr << " " << cis.seed_energy << ")";
    return s;
  }
};

} // anonymous namespace

StatusCode CaloCellsCounterGPU::execute(const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data, void * /*temporary_buffer*/) const
{
  Helpers::CPU_object<CellNoiseArr> cell_noise(constant_data.m_cell_noise_dev);
  //We could try to keep this in CPU memory by default,
  //but, since it isn't quite needed and we don't mind taking a bit longer for this
  //since we're only debugging, let this be as generic as possible.

  Helpers::CPU_object<CellInfoArr> cell_info(event_data.m_cell_info_dev);
  Helpers::CPU_object<CellStateArr> cell_state(event_data.m_cell_state_dev);
  Helpers::CPU_object<ClusterInfoArr> clusters(event_data.m_clusters_dev);

  unsigned int gain_counts[GainConversion::num_gain_values()] = {0};

  size_struct global_counts, global_cluster_counts;

  std::vector<int> cluster_max_cell(clusters->number, -1);
  std::vector<float> cluster_max_snr(clusters->number, -9e99);
  std::vector<float> cluster_max_energy(clusters->number, -9e99);

  std::vector<size_struct> cluster_counts(clusters->number);

  std::vector<int> shared_cells;

  for (int i = 0; i < NCaloCells; ++i)
    {
      if (!cell_info->is_valid(i))
        {
          continue;
        }

      const int gain = cell_info->gain[i];
      ++gain_counts[gain - GainConversion::min_gain_value()];

      const float energy = cell_info->energy[i];

      const float SNR = std::abs( energy / cell_noise->get_noise(i, gain) );

      const ClusterTag tag = cell_state->clusterTag[i];

      const bool is_cluster = tag.is_part_of_cluster();
      
      global_cluster_counts.total += is_cluster;

      if (SNR > m_seedThreshold)
        {
          ++global_counts.seed;
          global_cluster_counts.seed += is_cluster;
        }
      else if (SNR > m_growThreshold)
        {
          ++global_counts.grow;
          global_cluster_counts.grow += is_cluster;
        }
      else if (SNR > m_cellThreshold)
        {
          ++global_counts.term;
          global_cluster_counts.term += is_cluster;
        }
      else
        {
          ++global_counts.invalid;
          global_cluster_counts.invalid += is_cluster;
        }

      if (is_cluster)
        {
          if (tag.is_shared_between_clusters())
            {
              shared_cells.push_back(i);
            }
          const int cluster = tag.cluster_index();
          const int other_cluster = tag.is_shared_between_clusters() ? tag.secondary_cluster_index() : cluster;
          if (!tag.is_shared_between_clusters() && (SNR > cluster_max_snr[cluster] || (SNR == cluster_max_snr[cluster] && i > cluster_max_cell[cluster])))
            {
              cluster_max_snr[cluster] = SNR;
              cluster_max_cell[cluster] = i;
              cluster_max_energy[cluster] = std::abs(energy);
            }
          ++cluster_counts[cluster].total;
          cluster_counts[other_cluster].total += (cluster != other_cluster);

          global_cluster_counts.shared += tag.is_shared_between_clusters();
          cluster_counts[cluster].shared += tag.is_shared_between_clusters();
          cluster_counts[other_cluster].shared += tag.is_shared_between_clusters();

          if (SNR > m_seedThreshold)
            {
              ++cluster_counts[cluster].seed;
              cluster_counts[other_cluster].seed += (cluster != other_cluster);
            }
          else if (SNR > m_growThreshold)
            {
              ++cluster_counts[cluster].grow;
              cluster_counts[other_cluster].grow += (cluster != other_cluster);
            }
          else if (SNR > m_cellThreshold)
            {
              ++cluster_counts[cluster].term;
              cluster_counts[other_cluster].term += (cluster != other_cluster);
            }
          else
            {
              ++cluster_counts[cluster].invalid;
              cluster_counts[other_cluster].invalid += (cluster != other_cluster);
            }
        }
    }

  std::map<int, cluster_info_struct> cluster_sizes;

  for (int i = 0; i < clusters->number; ++i)
    {
      if (cluster_max_cell[i] >= 0)
        {
          cluster_sizes[cluster_max_cell[i]] = cluster_info_struct{cluster_counts[i], cluster_max_snr[i], cluster_max_energy[i]};
        }
    }

  std::sort(shared_cells.begin(), shared_cells.end());


  const auto err1 = StandaloneDataIO::prepare_folder_for_output(std::string(m_savePath));
  if (err1 != StandaloneDataIO::ErrorState::OK)
    {
      return StatusCode::FAILURE;
    }

  const std::filesystem::path save_file = m_savePath + "/" +  StandaloneDataIO::build_filename((m_filePrefix.size() > 0 ? m_filePrefix + "_counts" : "counts"),
                                                                                                 ctx.evt(), m_fileSuffix, "txt", m_numWidth);

  std::ofstream out_file(save_file);

  if (!out_file.is_open())
    {
      return StatusCode::FAILURE;
    }

  out_file << "Cell counts: " << global_counts << "\n\n";
  
  out_file << "Cells in clusters count: "<< global_cluster_counts << "\n\n";
  out_file << "Clusters:\n\n";

  for (const auto & it : cluster_sizes)
    {
      out_file << it.first << " " << it.second << "\n";
    }

  out_file << std::endl;

  if (!out_file.good())
    {
      return StatusCode::FAILURE;
    }

  out_file.close();

  return StatusCode::SUCCESS;

}

