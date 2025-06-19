//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloCellsCounterCPU.h"
#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/StandaloneDataIO.h"
#include "StoreGate/DataHandle.h"

#include <map>
#include <filesystem>

using namespace CaloRecGPU;

CaloCellsCounterCPU::CaloCellsCounterCPU(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent)
{
}


StatusCode CaloCellsCounterCPU::initialize()
{
  ATH_CHECK( m_noiseCDOKey.initialize() );
  ATH_CHECK( m_cellsKey.initialize() );
  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );
  return StatusCode::SUCCESS;
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
  int seed_index = -1;
  template <class Str>
  friend Str & operator << (Str & s, const cluster_info_struct & cis)
  {
    s << cis.seed_index << " "  << cis.size << " (" << cis.seed_snr << " " << cis.seed_energy << ")";
    return s;
  }
};

} // anonymous namespace

StatusCode CaloCellsCounterCPU::execute (const EventContext & ctx, xAOD::CaloClusterContainer * cluster_collection) const
{

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::FAILURE;
    }


  SG::ReadCondHandle<CaloNoise> noise_handle(m_noiseCDOKey, ctx);
  const CaloNoise * noise_tool = *noise_handle;

  unsigned int gain_counts[GainConversion::num_gain_values()] = {0};

  size_struct global_counts, global_cluster_counts;

  for (CaloCellContainer::const_iterator iCells = cell_collection->begin(); iCells != cell_collection->end(); ++iCells)
    {
      const CaloCell * cell = (*iCells);

      const float energy = cell->energy();

      float SNR = energy / noise_tool->getNoise(m_calo_id->calo_cell_hash(cell->ID()), cell->gain());

      const unsigned int gain = GainConversion::from_standard_gain(cell->gain());

      ++gain_counts[gain - GainConversion::min_gain_value()];

      SNR = std::abs(SNR);

      if (SNR > m_seedThreshold)
        {
          ++global_counts.seed;
        }
      else if (SNR > m_growThreshold)
        {
          ++global_counts.grow;
        }
      else if (SNR > m_cellThreshold)
        {
          ++global_counts.term;
        }
      else
        {
          ++global_counts.invalid;
        }
    }

  struct cluster_cell_info
  {
    const xAOD::CaloCluster * cl_1 = nullptr, * cl_2 = nullptr;
    double w_1 = -1, w_2 = -1, energy = -9e99, SNR = -9e99;
    void add_cluster(const int cell_id, const xAOD::CaloCluster * cl, const double w)
    {
      if (cl_1 == nullptr)
        {
          cl_1 = cl;
          w_1 = w;
        }
      else if (cl_2 == nullptr)
        {
          cl_2 = cl;
          w_2 = w;
        }
      else
        {
          std::cout << "WARNING! Multiple shared cell: " << cell_id << " " << cl_1 << " " << cl_2 << " " << cl << std::endl;
        }
    }

    bool is_shared() const
    {
      return cl_1 != nullptr && cl_2 != nullptr;
    }

  };

  std::map<int, cluster_cell_info> cluster_cells;


  for (const xAOD::CaloCluster * cluster : *cluster_collection)
    {
      //const xAOD::CaloCluster * cluster = (*cluster_iter);
      const CaloClusterCellLink * cell_links = cluster->getCellLinks();
      if (!cell_links)
        {
          ATH_MSG_ERROR("Can't get valid links to CaloCells (CaloClusterCellLink)!");
          return StatusCode::FAILURE;
        }

      for (auto it = cell_links->begin(); it != cell_links->end(); ++it)
        {
          const CaloCell * cell = *it;
          const float weight = it.weight();

          const float this_energy = std::abs(cell->energy());

          const float this_snr = std::abs(this_energy / noise_tool->getNoise(m_calo_id->calo_cell_hash(cell->ID()), cell->gain()));

          const IdentifierHash this_hash = m_calo_id->calo_cell_hash(cell->ID());

          auto & info = cluster_cells[this_hash];

          info.energy = this_energy;
          info.SNR = this_snr;

          info.add_cluster(this_hash, cluster, weight);

        }
    }

  std::unordered_map<const xAOD::CaloCluster *, cluster_info_struct> cluster_sizes;

  auto update_clusters = [&](const cluster_cell_info & cci, const int cell)
  {
    auto update_one = [&](const xAOD::CaloCluster * cl)
    {
      if (cl)
        {
          auto & c_info = cluster_sizes[cl];
          ++c_info.size.total;
          if (cci.SNR > m_seedThreshold)
            {
              ++c_info.size.seed;
            }
          else if (cci.SNR > m_growThreshold)
            {
              ++c_info.size.grow;
            }
          else if (cci.SNR > m_cellThreshold)
            {
              ++c_info.size.term;
            }
          else
            {
              ++c_info.size.invalid;
            }

          if (cci.is_shared())
            {
              ++c_info.size.shared;
            }
          else
            {
              if (cci.SNR > c_info.seed_snr || (cci.SNR == c_info.seed_snr && cell > c_info.seed_index))
                {
                  c_info.seed_index = cell;
                  c_info.seed_snr = cci.SNR;
                  c_info.seed_energy = cci.energy;
                }
            }
        }
    };

    if (cci.cl_1 != nullptr || cci.cl_2 != nullptr)
      {
        ++global_cluster_counts.total;
        if (cci.SNR > m_seedThreshold)
          {
            ++global_cluster_counts.seed;
          }
        else if (cci.SNR > m_growThreshold)
          {
            ++global_cluster_counts.grow;
          }
        else if (cci.SNR > m_cellThreshold)
          {
            ++global_cluster_counts.term;
          }
        else
          {
            ++global_cluster_counts.invalid;
          }
        if (cci.is_shared())
          {
            ++global_cluster_counts.shared;
          }
      }

    update_one(cci.cl_1);
    update_one(cci.cl_2);
  };

  for (auto & it : cluster_cells)
    {
      update_clusters(it.second, it.first);
    }

  std::vector<cluster_info_struct> sorted_info;

  sorted_info.reserve(cluster_sizes.size());

  for (auto & v : cluster_sizes)
    {
      sorted_info.push_back(v.second);
    }

  std::sort(sorted_info.begin(), sorted_info.end(),
            [](const auto & a, const auto & b)
  {
    return a.seed_index < b.seed_index;
  });


  const auto err1 = StandaloneDataIO::prepare_folder_for_output(std::string(m_savePath));
  if (err1 != StandaloneDataIO::ErrorState::OK)
    {
      return StatusCode::FAILURE;
    }

  const std::filesystem::path save_file = m_savePath + "/" + StandaloneDataIO::build_filename((m_filePrefix.size() > 0 ? m_filePrefix + "_counts" : "counts"),
                                                                                                ctx.evt(), m_fileSuffix, "txt", m_numWidth);

  std::ofstream out_file(save_file);

  if (!out_file.is_open())
    {
      return StatusCode::FAILURE;
    }

  out_file << "Cell counts: " << global_counts << "\n\n";

  out_file << "Cells in clusters count: " << global_cluster_counts << "\n\n";
  out_file << "Clusters:\n\n";

  for (const auto & info : sorted_info)
    {
      out_file << info << "\n";
    }

  out_file << std::endl;

  if (!out_file.good())
    {
      return StatusCode::FAILURE;
    }

  out_file.close();

  return StatusCode::SUCCESS;

}

