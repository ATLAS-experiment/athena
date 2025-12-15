//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include <vector>
#include <memory>

#include "CaloRecGPU/DataHolders.h"
#include "MacroHelpers.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "TileEvent/TileCell.h"

#include "CaloEvent/CaloPrefetch.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

void CaloRecGPU::ConstantDataHolder::sendToGPU(const bool clear_CPU)
{
  m_cell_noise_dev = m_cell_noise;
  m_geometry_dev = m_geometry;
  if (clear_CPU)
    {
      m_cell_noise.clear();
      m_geometry.clear();
    }
}

int CaloRecGPU::MomentsOptionsArray::moment_to_linear(const int moment)
{
  switch (moment)
    {
      default:
        return num_moments;
      case xAOD::CaloCluster::FIRST_PHI:
        return 0;
      case xAOD::CaloCluster::FIRST_ETA:
        return 1;
      case xAOD::CaloCluster::SECOND_R:
        return 2;
      case xAOD::CaloCluster::SECOND_LAMBDA:
        return 3;
      case xAOD::CaloCluster::DELTA_PHI:
        return 4;
      case xAOD::CaloCluster::DELTA_THETA:
        return 5;
      case xAOD::CaloCluster::DELTA_ALPHA:
        return 6;
      case xAOD::CaloCluster::CENTER_X:
        return 7;
      case xAOD::CaloCluster::CENTER_Y:
        return 8;
      case xAOD::CaloCluster::CENTER_Z:
        return 9;
      case xAOD::CaloCluster::CENTER_MAG:
        return 10;
      case xAOD::CaloCluster::CENTER_LAMBDA:
        return 11;
      case xAOD::CaloCluster::LATERAL:
        return 12;
      case xAOD::CaloCluster::LONGITUDINAL:
        return 13;
      case xAOD::CaloCluster::ENG_FRAC_EM:
        return 14;
      case xAOD::CaloCluster::ENG_FRAC_MAX:
        return 15;
      case xAOD::CaloCluster::ENG_FRAC_CORE:
        return 16;
      case xAOD::CaloCluster::FIRST_ENG_DENS:
        return 17;
      case xAOD::CaloCluster::SECOND_ENG_DENS:
        return 18;
      case xAOD::CaloCluster::ISOLATION:
        return 19;
      case xAOD::CaloCluster::ENG_BAD_CELLS:
        return 20;
      case xAOD::CaloCluster::N_BAD_CELLS:
        return 21;
      case xAOD::CaloCluster::N_BAD_CELLS_CORR:
        return 22;
      case xAOD::CaloCluster::BAD_CELLS_CORR_E:
        return 23;
      case xAOD::CaloCluster::BADLARQ_FRAC:
        return 24;
      case xAOD::CaloCluster::ENG_POS:
        return 25;
      case xAOD::CaloCluster::SIGNIFICANCE:
        return 26;
      case xAOD::CaloCluster::CELL_SIGNIFICANCE:
        return 27;
      case xAOD::CaloCluster::CELL_SIG_SAMPLING:
        return 28;
      case xAOD::CaloCluster::AVG_LAR_Q:
        return 29;
      case xAOD::CaloCluster::AVG_TILE_Q:
        return 30;
      case xAOD::CaloCluster::ENG_BAD_HV_CELLS:
        return 31;
      case xAOD::CaloCluster::N_BAD_HV_CELLS:
        return 32;
      case xAOD::CaloCluster::PTD:
        return 33;
      case xAOD::CaloCluster::MASS:
        return 34;
      case xAOD::CaloCluster::EM_PROBABILITY:
        return 35;
      case xAOD::CaloCluster::HAD_WEIGHT:
        return 36;
      case xAOD::CaloCluster::OOC_WEIGHT:
        return 37;
      case xAOD::CaloCluster::DM_WEIGHT:
        return 38;
      case xAOD::CaloCluster::TILE_CONFIDENCE_LEVEL:
        return 39;
      case xAOD::CaloCluster::SECOND_TIME:
        return 40;
      case xAOD::CaloCluster::NCELL_SAMPLING:
        return 41;
      case xAOD::CaloCluster::VERTEX_FRACTION:
        return 42;
      case xAOD::CaloCluster::NVERTEX_FRACTION:
        return 43;
      case xAOD::CaloCluster::ETACALOFRAME:
        return 44;
      case xAOD::CaloCluster::PHICALOFRAME:
        return 45;
      case xAOD::CaloCluster::ETA1CALOFRAME:
        return 46;
      case xAOD::CaloCluster::PHI1CALOFRAME:
        return 47;
      case xAOD::CaloCluster::ETA2CALOFRAME:
        return 48;
      case xAOD::CaloCluster::PHI2CALOFRAME:
        return 49;
      case xAOD::CaloCluster::ENG_CALIB_TOT:
        return 50;
      case xAOD::CaloCluster::ENG_CALIB_OUT_L:
        return 51;
      case xAOD::CaloCluster::ENG_CALIB_OUT_M:
        return 52;
      case xAOD::CaloCluster::ENG_CALIB_OUT_T:
        return 53;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_L:
        return 54;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_M:
        return 55;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_T:
        return 56;
      case xAOD::CaloCluster::ENG_CALIB_EMB0:
        return 57;
      case xAOD::CaloCluster::ENG_CALIB_EME0:
        return 58;
      case xAOD::CaloCluster::ENG_CALIB_TILEG3:
        return 59;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_TOT:
        return 60;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_EMB0:
        return 61;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_TILE0:
        return 62;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_TILEG3:
        return 63;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_EME0:
        return 64;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_HEC0:
        return 65;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_FCAL:
        return 66;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_LEAKAGE:
        return 67;
      case xAOD::CaloCluster::ENG_CALIB_DEAD_UNCLASS:
        return 68;
      case xAOD::CaloCluster::ENG_CALIB_FRAC_EM:
        return 69;
      case xAOD::CaloCluster::ENG_CALIB_FRAC_HAD:
        return 70;
      case xAOD::CaloCluster::ENG_CALIB_FRAC_REST:
        return 71;
    }
}

bool & CaloRecGPU::MomentsOptionsArray::operator[] (const int moment)
{
  return array[moment_to_linear(moment)];
}

bool CaloRecGPU::MomentsOptionsArray::operator[] (const int moment) const
{
  return array[moment_to_linear(moment)];
}

CaloRecGPU::MomentsOptionsArray CaloRecGPU::MomentsOptionsArray::all()
{
  CaloRecGPU::MomentsOptionsArray ret;

  for (int i = 0; i < num_moments; ++i)
    {
      ret.array[i] = true;
    }

  return ret;
}

void CaloRecGPU::EventDataHolder::importCells(const void * p_cell_collection, const std::vector<int> & extra_cells_to_fill)
{
  const CaloCellContainer * cell_collection = static_cast<const CaloCellContainer *>(p_cell_collection);

  m_cell_info.allocate();

  auto export_cell = [&](const CaloCell * cell, const int hash_ID, const int index_inside)
  {
    const float energy = cell->energy();
    const unsigned int gain = GainConversion::from_standard_gain(cell->gain());
    m_cell_info->energy[index_inside] = energy;
    m_cell_info->gain[index_inside] = gain;
    m_cell_info->time[index_inside] = cell->time();

    if (CaloRecGPU::GeometryArr::is_tile(hash_ID))
      {
        const TileCell * tile_cell = static_cast<const TileCell *>(cell);

        m_cell_info->qualityProvenance[index_inside] = QualityProvenance{tile_cell->qual1(),
                                                                         tile_cell->qual2(),
                                                                         tile_cell->qbit1(),
                                                                         tile_cell->qbit2()};
      }
    else
      {
        m_cell_info->qualityProvenance[index_inside] = QualityProvenance{cell->quality(), cell->provenance()};
      }

    m_cell_info->hashID[index_inside] = hash_ID;
    m_cell_info->hashIDToCollection[hash_ID] = index_inside;
  };

  if (cell_collection->isOrderedAndComplete())
    //Fast path: cell indices within the collection and identifierHashes match!
    {
      int cell_index = 0;
      for (auto cell_it = cell_collection->begin(); cell_it != cell_collection->end(); ++cell_it, ++cell_index)
        {
          const CaloCell * cell = (*cell_it);
          export_cell(cell, cell_index, cell_index);
        }
      
      m_cell_info->number = CaloRecGPU::NCaloCells;
      m_cell_info->complete = true;
      m_cell_info->all_cells_valid = true;
    }
  else if (cell_collection->isOrdered() && extra_cells_to_fill.size() > 0)
    //Remediated: we know the missing cells, force them to be invalid.
    //(Tests so far, on samples both oldish and newish, had 186986 and 187352 missing...)
    {
      int cell_index = 0;
      size_t missing_cell_count = 0;

      for (CaloCellContainer::const_iterator cell_it = cell_collection->begin(); cell_it != cell_collection->end(); ++cell_it, ++cell_index)
        {
          const CaloCell * cell = (*cell_it);

          if (missing_cell_count < extra_cells_to_fill.size() && cell_index == extra_cells_to_fill[missing_cell_count])
            {
              --cell_it;
              m_cell_info->gain[cell_index] = GainConversion::invalid_gain();
              ++missing_cell_count;
              m_cell_info->hashID[cell_index] = cell_index;
              m_cell_info->hashIDToCollection[cell_index] = -1;
              continue;
            }
          else
            {
              export_cell(cell, cell_index, cell_index);
            }
        }

      m_cell_info->number = CaloRecGPU::NCaloCells;
      m_cell_info->complete = true;
      m_cell_info->all_cells_valid = false;
    }
  else
    {
      for (unsigned int cell_index = 0; cell_index < CaloRecGPU::NCaloCells; ++cell_index)
        {
          m_cell_info->hashIDToCollection[cell_index] = -1;
        }

      int index_inside = 0;

      const auto cells_end = cell_collection->end();
      
      for (CaloCellContainer::const_iterator cell_it = cell_collection->begin(); cell_it != cells_end; ++cell_it, ++index_inside)
        {
          CaloPrefetch::nextDDE(cell_it, cells_end, 2);
          //May be adjusted later...
          
          const CaloCell * cell = (*cell_it);
          
          const int cell_index = cell->caloDDE()->calo_hash();
          export_cell(cell, cell_index, index_inside);
        }

      m_cell_info->number = cell_collection->size();
      m_cell_info->complete = false;
      m_cell_info->all_cells_valid = true;
    }
}


void CaloRecGPU::EventDataHolder::importClusters(const void * p_cluster_collection,
                                                 const MomentsOptionsArray & moments_to_add,
                                                 const bool output_tags,
                                                 const bool consider_shared_cells,
                                                 const bool output_moments,
                                                 const bool output_extra_moments,
                                                 const std::vector<int> & extra_cells_to_fill)
{
  const xAOD::CaloClusterContainer * cluster_collection = static_cast<const xAOD::CaloClusterContainer *>(p_cluster_collection);

  m_clusters.allocate();

  if (cluster_collection->size() > 0)
    {
      std::vector<int> real_cells_to_fill;

      if (m_cell_info->number == CaloRecGPU::NCaloCells && !m_cell_info->all_cells_valid)
        {
          real_cells_to_fill.reserve(extra_cells_to_fill.size());

          for (const int cell : extra_cells_to_fill)
            {
              if (m_cell_info->hashIDToCollection[cell] <= 0)
                {
                  real_cells_to_fill.push_back(cell);
                }
            }
        }

      auto index_to_corrected_index = [&](const int index)
      {
        if (m_cell_info->all_cells_valid)
          {
            return index;
          }

        int ret = index;

        for (const int cell : real_cells_to_fill)
          {
            ret += (index > cell);
          }

        return ret;
      };

      auto get_cell_tag = [&](const int index) -> tag_type &
      {
        if (output_tags)
          {
            return m_clusters->cells.tags[index];
          }
        else
          {
            return m_clusters->get_extra_cell_info(index);
          }
          
      };
      
      for (int i = 0; i < CaloRecGPU::NCaloCells; ++i)
        {
          get_cell_tag(i) = ClusterTag::make_invalid_tag();
        }
      
      if (output_tags)
        {
          m_clusters->state = ClusterInformationState::TagsWithBasicInfo;
        }
      else
        {
          m_clusters->state = (output_moments ?
                               (output_extra_moments ? ClusterInformationState::WithExtraMoments : ClusterInformationState::WithMoments) :
                               ClusterInformationState::WithBasicInfo);
          m_clusters->cellsPrefixSum[0] = 0;
        }

      m_clusters->has_deleted_clusters = false;

      const auto cluster_end = cluster_collection->end();
      auto cluster_iter = cluster_collection->begin();

      int overall_cell_index = 0;

      for (int cluster_number = 0; cluster_iter != cluster_end; ++cluster_iter, ++cluster_number)
        {
          const xAOD::CaloCluster * cluster = (*cluster_iter);
          const CaloClusterCellLink * cell_links = cluster->getCellLinks();

          m_clusters->clusterEnergy[cluster_number] = cluster->e();
          m_clusters->clusterEt[cluster_number] = cluster->et();
          m_clusters->clusterEta[cluster_number] = cluster->eta();
          m_clusters->clusterPhi[cluster_number] = cluster->phi();

          const int seed_cell_index = (cluster->cell_begin() != cluster->cell_end() ? cluster->cell_begin().index() : -1);

          if (cluster->cell_begin() == cluster->cell_end())
            {
              m_clusters->has_deleted_clusters = true;
              m_clusters->seedCellIndex[cluster_number] = -1;
            }
          else
            {
              m_clusters->seedCellIndex[cluster_number] = index_to_corrected_index(seed_cell_index);
            }

          for (auto it = cell_links->begin(); it != cell_links->end(); ++it)
            {
              const int cell_index_in_collection = index_to_corrected_index(it.index());
              if (consider_shared_cells)
                {
                  const float weight = it.weight();
                  
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
                  
                  const ClusterTag other_tag = get_cell_tag(cell_index_in_collection);

                  const int other_index = other_tag.is_part_of_cluster() ? other_tag.cluster_index() : -1;

                  if (other_index < 0)
                    {
                      if (weight < 0.5f)
                        {
                          get_cell_tag(cell_index_in_collection) = ClusterTag::make_tag(cluster_number, weight_as_int, 0);
                        }
                      else
                        {
                          get_cell_tag(cell_index_in_collection) = ClusterTag::make_tag(cluster_number);
                        }
                    }
                  else if (weight > 0.5f)
                    {
                      get_cell_tag(cell_index_in_collection) = ClusterTag::make_tag(cluster_number, other_tag.secondary_cluster_weight(), other_index);
                    }
                  else if (weight == 0.5f)
                    //Unlikely, but...
                    {
                      const int max_cluster = cluster_number > other_index ? cluster_number : other_index;
                      const int min_cluster = cluster_number > other_index ? other_index : cluster_number;
                      get_cell_tag(cell_index_in_collection) = ClusterTag::make_tag(max_cluster, weight_as_int, min_cluster);
                    }
                  else /*if (weight < 0.5f)*/
                    {
                      get_cell_tag(cell_index_in_collection) = ClusterTag::make_tag(other_index, weight_as_int, cluster_number);
                    }
                        
                  //All of this logic assumes a cell is shared by at most two clusters
                  //with weights such that w_1 + w_2 = 1.
                  //This is not necessarily valid with local calibrations.
                      
                }
              else
                {
                  get_cell_tag(cell_index_in_collection) = ClusterTag::make_tag(cluster_number);
                }
            }
          
          if (!output_tags)
            {
              for (auto it = cell_links->begin(); it != cell_links->end(); ++it, ++overall_cell_index)
                {
                  m_clusters->cells.indices[overall_cell_index] = index_to_corrected_index(it.index());
                  m_clusters->clusterIndices[overall_cell_index] = cluster_number;
                  m_clusters->cellWeights[overall_cell_index] = it.weight();
                }

              m_clusters->cellsPrefixSum[cluster_number + 1] = overall_cell_index;
            }

          if (output_moments)
            {
              for (int s = 0; s < NumSamplings; ++s)
                {
                  m_clusters->moments.nCellSampling[s][cluster_number] = cluster->numberCellsInSampling(static_cast<CaloSampling::CaloSample>(s), false);
                }

              m_clusters->moments.nExtraCellSampling[cluster_number] = cluster->numberCellsInSampling(CaloSampling::EME2, true);

              for (int s = 0; s < NumSamplings; ++s)
                {
                  m_clusters->moments.energyPerSample[s][cluster_number] = cluster->eSample(static_cast<CaloSampling::CaloSample>(s));
                }
              for (int s = 0; s < NumSamplings; ++s)
                {
                  m_clusters->moments.maxEPerSample[s][cluster_number] = cluster->energy_max(static_cast<CaloSampling::CaloSample>(s));
                }
              for (int s = 0; s < NumSamplings; ++s)
                {
                  m_clusters->moments.maxEtaPerSample[s][cluster_number] = cluster->etamax(static_cast<CaloSampling::CaloSample>(s));
                }
              for (int s = 0; s < NumSamplings; ++s)
                {
                  m_clusters->moments.maxPhiPerSample[s][cluster_number] = cluster->phimax(static_cast<CaloSampling::CaloSample>(s));
                }
              for (int s = 0; s < NumSamplings; ++s)
                {
                  m_clusters->moments.etaPerSample[s][cluster_number] = cluster->etaSample(static_cast<CaloSampling::CaloSample>(s));
                }
              for (int s = 0; s < NumSamplings; ++s)
                {
                  m_clusters->moments.phiPerSample[s][cluster_number] = cluster->phiSample(static_cast<CaloSampling::CaloSample>(s));
                }

              m_clusters->moments.time[cluster_number] = cluster->time();

#define CALORECGPU_MOMENTS_INPUT_HELPER(VAR_NAME, PROPER_MOMENT, NORMAL_ASSIGN, IS_CALCULATED, MOMENT_NAME, ...) \
  CRGPU_CONCAT(CRGPU_CONCAT(CALORECGPU_MOMENTS_INPUT_HELPER_, NORMAL_ASSIGN), IS_CALCULATED) (VAR_NAME, MOMENT_NAME)

#define CALORECGPU_MOMENTS_INPUT_HELPER_11(VAR_NAME, MOMENT_NAME)                                                 \
  if (moments_to_add[xAOD::CaloCluster:: MOMENT_NAME])                                                            \
    {                                                                                                             \
      m_clusters->moments. VAR_NAME [cluster_number] = cluster->getMomentValue(xAOD::CaloCluster:: MOMENT_NAME ); \
    }

#define CALORECGPU_MOMENTS_INPUT_HELPER_10(VAR_NAME, MOMENT_NAME)                                                 \
  if (output_extra_moments && moments_to_add[xAOD::CaloCluster:: MOMENT_NAME])                                    \
    {                                                                                                             \
      m_clusters->moments. VAR_NAME [cluster_number] = cluster->getMomentValue(xAOD::CaloCluster:: MOMENT_NAME ); \
    }

#define CALORECGPU_MOMENTS_INPUT_HELPER_00(...)
#define CALORECGPU_MOMENTS_INPUT_HELPER_01(...)

              CALORECGPU_FORALLMOMENTS_INSTANTIATE(CALORECGPU_MOMENTS_INPUT_HELPER)

              double second_time_retriever = 0;

              if (!cluster->retrieveMoment(xAOD::CaloCluster::SECOND_TIME, second_time_retriever))
                {
                  m_clusters->moments.secondTime[cluster_number] = cluster->secondTime();
                  //Special casing for SECOND_TIME as it comes from CalculateKine instead,
                  //thus stored in a member variable instead of the moment.
                }
              else
                {
                  m_clusters->moments.secondTime[cluster_number] = second_time_retriever;
                }
            }
        }

      m_clusters->number = cluster_collection->size();
      m_clusters->number_cells = (output_tags ? m_cell_info->number : overall_cell_index);
    }
  else
    {
      m_clusters->state = CaloRecGPU::ClusterInformationState::None;
      m_clusters->has_deleted_clusters = false;
      m_clusters->number = 0;
      m_clusters->number_cells = 0;
    }
}

template <class ReadStateFrom, class OutputCells, class InputCells, class CopyFunc>
static void cell_transfer_helper(ReadStateFrom & state_holder,
                                 OutputCells & output_cells,
                                 InputCells & input_cells,
                                 CopyFunc copy_func,
                                 const bool full_copy,
                                 CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  if (full_copy)
    {
      copy_func(output_cells, input_cells, sizeof(CaloRecGPU::CellInfoArr), stream);
    }
  else
    {
      copy_func(output_cells->gain,
                input_cells->gain,
                sizeof(unsigned char) * state_holder->number, stream);

      copy_func(output_cells->energy,
                input_cells->energy,
                sizeof(float) * state_holder->number, stream);

      copy_func(output_cells->time,
                input_cells->time,
                sizeof(float) * state_holder->number, stream);

      copy_func(output_cells->qualityProvenance,
                input_cells->qualityProvenance,
                sizeof(QualityProvenance::carrier) * state_holder->number, stream);

      copy_func(output_cells->hashID,
                input_cells->hashID,
                sizeof(int) * state_holder->number, stream);

      copy_func(output_cells->hashIDToCollection,
                input_cells->hashIDToCollection,
                sizeof(int) * CaloRecGPU::NCaloCells, stream);

    }
}

template <class ReadStateFrom, class OutputClusters, class InputClusters, class CopyFunc>
static void cluster_transfer_helper_basic_info(ReadStateFrom & state_holder,
                                               OutputClusters & output_clusters,
                                               InputClusters & input_clusters,
                                               CopyFunc copy_func,
                                               CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{

  if (state_holder->has_basic_info())
    {
      copy_func(output_clusters->clusterEnergy,
                input_clusters->clusterEnergy,
                sizeof(float) * state_holder->number, stream);

      copy_func(output_clusters->clusterEt,
                input_clusters->clusterEt,
                sizeof(float) * state_holder->number, stream);

      copy_func(output_clusters->clusterEta,
                input_clusters->clusterEta,
                sizeof(float) * state_holder->number, stream);

      copy_func(output_clusters->clusterPhi,
                input_clusters->clusterPhi,
                sizeof(float) * state_holder->number, stream);

      copy_func(output_clusters->seedCellIndex,
                input_clusters->seedCellIndex,
                sizeof(int) * state_holder->number, stream);
    }

}

template <class ReadStateFrom, class OutputClusters, class InputClusters, class CopyFunc>
static void cluster_transfer_helper_cell_assignment(ReadStateFrom & state_holder,
                                                    OutputClusters & output_clusters,
                                                    InputClusters & input_clusters,
                                                    CopyFunc copy_func,
                                                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream,
                                                    const int num_total_cells = NCaloCells)
{

  if (state_holder->has_cells_per_cluster())
    {
      copy_func(output_clusters->cellsPrefixSum,
                input_clusters->cellsPrefixSum,
                sizeof(int) * (state_holder->number + 1), stream);

      copy_func(output_clusters->cells.indices,
                input_clusters->cells.indices,
                sizeof(int) * state_holder->number_cells, stream);

      copy_func(output_clusters->cellWeights,
                input_clusters->cellWeights,
                sizeof(float) * state_holder->number_cells, stream);

      copy_func(output_clusters->clusterIndices,
                input_clusters->clusterIndices,
                sizeof(int) * state_holder->number_cells, stream);

      int remaining_cells = num_total_cells;

#define CALORECGPU_MOMENTS_EXTRA_TAGS_HELPER(VARNAME)                                                             \
  if (remaining_cells > 0)                                                                                        \
    {                                                                                                             \
      constexpr int max_size_per_array = (sizeof(float) * NMaxClusters)/sizeof(tag_type);                         \
      copy_func(output_clusters->moments. VARNAME,                                                                \
                 input_clusters->moments. VARNAME,                                                                \
                sizeof(tag_type) * std::min(remaining_cells, max_size_per_array), stream);                        \
      remaining_cells -= max_size_per_array;                                                                      \
    } (void) 0                                                                                                    \

      CALORECGPU_MOMENTS_EXTRA_TAGS_HELPER(engCalibTot);
      CALORECGPU_MOMENTS_EXTRA_TAGS_HELPER(engCalibOutL);
      CALORECGPU_MOMENTS_EXTRA_TAGS_HELPER(engCalibOutM);
      CALORECGPU_MOMENTS_EXTRA_TAGS_HELPER(engCalibOutT);
      CALORECGPU_MOMENTS_EXTRA_TAGS_HELPER(engCalibDeadL);
      CALORECGPU_MOMENTS_EXTRA_TAGS_HELPER(engCalibDeadM);
      
    }
  else if (state_holder->state != CaloRecGPU::ClusterInformationState::None)
    {
      copy_func(output_clusters->cells.tags,
                input_clusters->cells.tags,
                sizeof(tag_type) * state_holder->number_cells, stream);

    }
}

template <class ReadStateFrom, class OutputClusters, class InputClusters, class CopyFunc>
static void cluster_transfer_helper_moments(ReadStateFrom & state_holder,
                                            OutputClusters & output_clusters,
                                            InputClusters & input_clusters,
                                            const MomentsOptionsArray & moments_to_add,
                                            CopyFunc copy_func,
                                            CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  if (state_holder->has_moments())
    {
      auto moments_copy_helper = [&](auto & output_arr, const auto & input_arr)
      {
        if constexpr(std::is_pointer_v<std::decay_t<decltype(*input_arr)>>)
          //This is a per-sampling array.
          {
            for (int i = 0; i < CaloRecGPU::NumSamplings; ++i)
              {
                copy_func(output_arr[i],
                          input_arr[i],
                          sizeof(decltype(input_arr[0][0])) * state_holder->number,
                          stream);
              }
          }
        else
          {
            copy_func(output_arr,
                      input_arr,
                      sizeof(decltype(input_arr[0])) * state_holder->number,
                      stream);
          }
      };

#define CALORECGPU_MOMENTS_TO_GPU_HELPER(VAR_NAME, PROPER_MOMENT, NORMAL_ASSIGN, IS_CALCULATED, MOMENT_NAME, ...) \
  CRGPU_CONCAT(CRGPU_CONCAT(CALORECGPU_MOMENTS_TO_GPU_HELPER_, PROPER_MOMENT), IS_CALCULATED)  (VAR_NAME, MOMENT_NAME)

#define CALORECGPU_MOMENTS_TO_GPU_HELPER_11(VAR_NAME, MOMENT_NAME)                                                \
  if (moments_to_add[xAOD::CaloCluster:: MOMENT_NAME ])                                                           \
    {                                                                                                             \
      moments_copy_helper(output_clusters->moments. VAR_NAME, input_clusters->moments. VAR_NAME);                 \
    }

#define CALORECGPU_MOMENTS_TO_GPU_HELPER_10(VAR_NAME, MOMENT_NAME)                                                \
  if ( state_holder->state == CaloRecGPU::ClusterInformationState::WithExtraMoments &&                            \
       moments_to_add[xAOD::CaloCluster:: MOMENT_NAME ]                                  )                        \
    {                                                                                                             \
      moments_copy_helper(output_clusters->moments. VAR_NAME, input_clusters->moments. VAR_NAME);                 \
    }

#define CALORECGPU_MOMENTS_TO_GPU_HELPER_01( VAR_NAME, MOMENT_NAME)                                               \
  moments_copy_helper(output_clusters->moments. VAR_NAME, input_clusters->moments. VAR_NAME);

#define CALORECGPU_MOMENTS_TO_GPU_HELPER_00(PROPER_MOMENT, VAR_NAME, MOMENT_NAME)                                 \
  if (state_holder->state == CaloRecGPU::ClusterInformationState::WithExtraMoments)                               \
    {                                                                                                             \
      moments_copy_helper(output_clusters->moments. VAR_NAME, input_clusters->moments. VAR_NAME);                 \
    }

      CALORECGPU_FORALLMOMENTS_INSTANTIATE(CALORECGPU_MOMENTS_TO_GPU_HELPER)
    }
}

template <class ReadStateFrom, class OutputClusters, class InputClusters, class CopyFunc>
static void cluster_transfer_helper(ReadStateFrom & state_holder,
                                    OutputClusters & output_clusters,
                                    InputClusters & input_clusters,
                                    const MomentsOptionsArray & moments_to_add,
                                    CopyFunc copy_func,
                                    const bool full_copy,
                                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream,
                                    const int num_total_cells = CaloRecGPU::NCaloCells)
{
  if (full_copy)
    {
      copy_func(output_clusters, input_clusters, sizeof(CaloRecGPU::ClusterInfoArr), stream);
    }
  else
    {
      cluster_transfer_helper_basic_info(state_holder, output_clusters, input_clusters, copy_func, stream);

      cluster_transfer_helper_cell_assignment(state_holder, output_clusters, input_clusters, copy_func, stream, num_total_cells);

      cluster_transfer_helper_moments(state_holder, output_clusters, input_clusters, moments_to_add, copy_func, stream);
    }
}


void CaloRecGPU::EventDataHolder::sendToGPU(const MomentsOptionsArray & moments_to_add,
                                            const bool full_copy,
                                            const bool clear_CPU,
                                            const bool synchronize,
                                            CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{  
  m_cell_info_dev.allocate();
  m_clusters_dev.allocate();

  if (!full_copy)
    {
      CaloRecGPU::CUDA_Helpers::CPU_to_GPU_async(static_cast<CellBaseInfo *>(m_cell_info_dev),
                                                 static_cast<const CellBaseInfo *>(m_cell_info),
                                                 sizeof(CellBaseInfo), stream);

      CaloRecGPU::CUDA_Helpers::CPU_to_GPU_async(static_cast<ClusterBaseInfo *>(m_clusters_dev),
                                                 static_cast<const ClusterBaseInfo *>(m_clusters),
                                                 sizeof(ClusterBaseInfo), stream);

    }

  cell_transfer_helper(m_cell_info, m_cell_info_dev, m_cell_info,
                       CaloRecGPU::CUDA_Helpers::CPU_to_GPU_async, full_copy, stream);

  cluster_transfer_helper(m_clusters, m_clusters_dev, m_clusters, moments_to_add,
                          CaloRecGPU::CUDA_Helpers::CPU_to_GPU_async, full_copy, stream,
                          m_cell_info->number);
  
  if (clear_CPU)
    {
      CaloRecGPU::CUDA_Helpers::GPU_synchronize(stream);

      m_clusters.clear();
      m_cell_info.clear();
    }
  else if (synchronize)
    {
      CaloRecGPU::CUDA_Helpers::GPU_synchronize(stream);
    }
}

void CaloRecGPU::EventDataHolder::returnToCPU(const MomentsOptionsArray & moments_to_add,
                                              const bool full_copy,
                                              const bool clear_GPU,
                                              const bool synchronize,
                                              CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream,
                                              const bool also_return_cells)
{
  if (also_return_cells)
    {
      m_cell_info.allocate();
    }
  m_clusters.allocate();

  if (!full_copy)
    {
      if (also_return_cells)
        {
          CaloRecGPU::CUDA_Helpers::GPU_to_CPU_async(static_cast<CellBaseInfo *>(m_cell_info),
                                                     static_cast<CellBaseInfo *>(m_cell_info_dev),
                                                     sizeof(CellBaseInfo), stream);
        }

      CaloRecGPU::CUDA_Helpers::GPU_to_CPU_async(static_cast<ClusterBaseInfo *>(m_clusters),
                                                 static_cast<ClusterBaseInfo *>(m_clusters_dev),
                                                 sizeof(ClusterBaseInfo), stream);

      CaloRecGPU::CUDA_Helpers::GPU_synchronize(stream);

    }

  if (also_return_cells)
    {
      cell_transfer_helper(m_cell_info, m_cell_info, m_cell_info_dev,
                           CaloRecGPU::CUDA_Helpers::GPU_to_CPU_async, full_copy, stream);
    }

  cluster_transfer_helper(m_clusters, m_clusters, m_clusters_dev, moments_to_add,
                          CaloRecGPU::CUDA_Helpers::GPU_to_CPU_async, full_copy, stream);

  if (clear_GPU)
    {
      CaloRecGPU::CUDA_Helpers::GPU_synchronize(stream);

      m_clusters_dev.clear();
      m_cell_info_dev.clear();
    }
  else if (synchronize)
    {
      CaloRecGPU::CUDA_Helpers::GPU_synchronize(stream);
    }
}

//clusters->number must be available!
//if skip_validation is true, clusters->seedCellIndex must also be available.
static void export_cluster_initialize_links(const ClusterInfoArr * clusters,
                                            std::vector<std::unique_ptr<CaloClusterCellLink>> & cell_links,
                                            const DataLink<CaloCellContainer> & cell_collection_link,
                                            const bool skip_validation)
{
  cell_links.reserve(clusters->number);

  for (int i = 0; i < clusters->number; ++i)
    {
      if (skip_validation || clusters->seedCellIndex[i] >= 0)
        {
          cell_links.emplace_back(std::make_unique<CaloClusterCellLink>(cell_collection_link));
          cell_links.back()->reserve(256);
          //To be adjusted.
        }
      else
        {
          cell_links.emplace_back(nullptr);
          //The excluded clusters don't have any cells.
        }
    }
}

//The CPU cells must be available!
//The cluster status and number of cells must be available!
//
static void export_cluster_process_cells(const ClusterInfoArr * clusters,
                                         const CellInfoArr * cells,
                                         std::vector<std::unique_ptr<CaloClusterCellLink>> & cell_links,
                                         const std::vector<int> & extra_cells_to_fill)
{

  std::vector<int> real_cells_to_fill;

  if (cells->number == CaloRecGPU::NCaloCells && !cells->all_cells_valid)
    {
      real_cells_to_fill.reserve(extra_cells_to_fill.size());

      for (const int cell : extra_cells_to_fill)
        {
          if (cells->hashIDToCollection[cell] <= 0)
            {
              real_cells_to_fill.push_back(cell);
            }
        }
    }

  auto corrected_index_to_real_index = [&](const int index)
  {
    if (cells->all_cells_valid)
      {
        return index;
      }

    int ret = index;

    for (const int cell : real_cells_to_fill)
      {
        ret -= (index > cell);
      }

    return ret;
  };

  if (clusters->has_cells_per_cluster())
    {
      for (int i = 0; i < clusters->number_cells; ++i)
        {
          std::unique_ptr<CaloClusterCellLink> & this_link_ptr = cell_links[clusters->clusterIndices[i]];
          if (this_link_ptr)
            //This should always be true at this stage,
            //but guarding against any potentially invalidated clusters.
            {
              this_link_ptr->addCell(corrected_index_to_real_index(clusters->cells.indices[i]), clusters->cellWeights[i]);
            }
        }
    }
  else
    {
      for (int i = 0; i < clusters->number; ++i)
        {
          if (cell_links[i])
            {
              cell_links[i]->addCell(corrected_index_to_real_index(clusters->seedCellIndex[i]), 1);
              //Seed cells aren't shared, by construction.
            }
        }

      for (int i = 0; i < clusters->number_cells; ++i)
        {
          const ClusterTag this_tag = clusters->cells.tags[i];

          if (this_tag.is_part_of_cluster())
            {
              const int this_cell_index = corrected_index_to_real_index(i);

              const int this_index = this_tag.cluster_index();
              const int32_t weight_pattern = this_tag.secondary_cluster_weight();

              float tempf = 1.0f;

              std::memcpy(&tempf, &weight_pattern, sizeof(float));

              const float reverse_weight = tempf;

              const float this_weight = 1.0f - reverse_weight;

              if (cell_links[this_index] && clusters->seedCellIndex[this_index] != i)
                {
                  cell_links[this_index]->addCell(this_cell_index, this_weight);
                }

              if (this_tag.is_shared_between_clusters())
                {
                  const int other_index = this_tag.secondary_cluster_index();
                  if (cell_links[other_index] && clusters->seedCellIndex[other_index] != i)
                    {
                      cell_links[other_index]->addCell(this_cell_index, reverse_weight);
                    }
                }
            }
        }
    }
}

static void export_cluster_sort(const ClusterInfoArr * clusters,
                                std::vector<int> & cluster_order,
                                const bool really_sort)
{
  if (really_sort)
    {
      cluster_order.resize(clusters->number);

      std::iota(cluster_order.begin(), cluster_order.end(), 0);

      std::sort(cluster_order.begin(), cluster_order.end(), [&](const int a, const int b) -> bool
      {
        const bool a_valid = clusters->seedCellIndex[a] >= 0;
        const bool b_valid = clusters->seedCellIndex[b] >= 0;
        if (a_valid && b_valid)
          {
            return (clusters->clusterEt[a] > clusters->clusterEt[b]);
          }
        else if (a_valid)
          {
            return true;
          }
        else if (b_valid)
          {
            return false;
          }
        else
          {
            return b > a;
          }
      } );
    }
  else
    {
      cluster_order.clear();
    }
}

static void export_cluster_fill_cells_and_basic_info(const ClusterInfoArr * clusters,
                                                     xAOD::CaloClusterContainer * cluster_collection,
                                                     std::vector<std::unique_ptr<CaloClusterCellLink>> & cell_links,
                                                     std::vector<int> & cluster_order,
                                                     const bool save_uncalibrated)
{  
  const int total_size = (cluster_order.size() ? cluster_order.size() : clusters->number);
  for (int i = 0; i < total_size; ++i)
    {
      const int cluster_index = (cluster_order.size() ? cluster_order[i] : i);

      if (cell_links[cluster_index] != nullptr && cell_links[cluster_index]->size() > 0)
        {
          xAOD::CaloCluster * cluster = new xAOD::CaloCluster();
          cluster_collection->push_back(cluster);

          cluster->addCellLink(std::move(cell_links[cluster_index]));

          if (clusters->has_basic_info())
            {

              cluster->setE  (clusters->clusterEnergy[cluster_index]);
              cluster->setEta(clusters->clusterEta   [cluster_index]);
              cluster->setPhi(clusters->clusterPhi   [cluster_index]);

              if (save_uncalibrated)
                {
                  cluster->setRawE(cluster->calE());
                  cluster->setRawEta(cluster->calEta());
                  cluster->setRawPhi(cluster->calPhi());
                  cluster->setRawM(cluster->calM());
                }
            }
        }
    }
}

static void export_cluster_fill_moments(const ClusterInfoArr * clusters,
                                        xAOD::CaloClusterContainer * cluster_collection,
                                        std::vector<int> & cluster_order,
                                        const MomentsOptionsArray & moments_to_add,
                                        const bool output_extra_moments)
{
  if (clusters->has_moments())
    {
      const int total_size = (cluster_order.size() ? cluster_order.size() : clusters->number);
      for (int i = 0; i < total_size; ++i)
        {
          const int cluster_index = (cluster_order.size() ? cluster_order[i] : i);

          xAOD::CaloCluster * cluster = (*cluster_collection)[cluster_index];

          cluster->clearSamplingData();

          uint32_t sampling_pattern = 0;
          for (int sampl = 0; sampl < NumSamplings; ++sampl)
            {
              const int cells_per_sampling = clusters->moments.nCellSampling[sampl][cluster_index];

              if (cells_per_sampling > 0)
                {
                  sampling_pattern |= (0x1U << sampl);
                }
            }

          if (clusters->moments.nExtraCellSampling[cluster_index] > 0)
            {
              sampling_pattern |= (1U << static_cast<unsigned int>(CaloSampling::EME2));
            }
          
          cluster->setSamplingPattern(sampling_pattern);

          for (int s = 0; s < NumSamplings; ++s)
            {
              if (sampling_pattern & (1U << s))
                {
                  cluster->setNumberCellsInSampling(static_cast<CaloSampling::CaloSample>(s), clusters->moments.nCellSampling[s][cluster_index], false);
                }
            }

          if (clusters->moments.nExtraCellSampling[cluster_index] > 0)
            {
              cluster->setNumberCellsInSampling(CaloSampling::EME2, clusters->moments.nExtraCellSampling[cluster_index], true);              
            }

          for (int s = 0; s < NumSamplings; ++s)
            {
              if (sampling_pattern & (1U << s))
                {
                  cluster->setEnergy(static_cast<CaloSampling::CaloSample>(s), clusters->moments.energyPerSample[s][cluster_index]);
                }
            }
          for (int s = 0; s < NumSamplings; ++s)
            {
              if (sampling_pattern & (1U << s))
                {
                  cluster->setEmax(static_cast<CaloSampling::CaloSample>(s), clusters->moments.maxEPerSample[s][cluster_index]);
                }
            }
          for (int s = 0; s < NumSamplings; ++s)
            {
              if (sampling_pattern & (1U << s))
                {
                  cluster->setEtamax(static_cast<CaloSampling::CaloSample>(s), clusters->moments.maxEtaPerSample[s][cluster_index] );
                }
            }
          for (int s = 0; s < NumSamplings; ++s)
            {
              if (sampling_pattern & (1U << s))
                {
                  cluster->setPhimax(static_cast<CaloSampling::CaloSample>(s), clusters->moments.maxPhiPerSample[s][cluster_index]);
                }
            }
          for (int s = 0; s < NumSamplings; ++s)
            {
              if (sampling_pattern & (1U << s))
                {
                  cluster->setEta(static_cast<CaloSampling::CaloSample>(s), clusters->moments.etaPerSample[s][cluster_index]);
                }
            }
          for (int s = 0; s < NumSamplings; ++s)
            {
              if (sampling_pattern & (1U << s))
                {
                  cluster->setPhi(static_cast<CaloSampling::CaloSample>(s), clusters->moments.phiPerSample[s][cluster_index]);
                }
            }

          cluster->setTime(clusters->moments.time[cluster_index]);
          cluster->setSecondTime(clusters->moments.secondTime[cluster_index]);

#define CALORECGPU_MOMENTS_OUTPUT_HELPER(VAR_NAME, PROPER_MOMENT, NORMAL_ASSIGN, IS_CALCULATED, MOMENT_NAME, ...) \
  CRGPU_CONCAT(CRGPU_CONCAT(CALORECGPU_MOMENTS_OUTPUT_HELPER_, NORMAL_ASSIGN), IS_CALCULATED) (VAR_NAME, MOMENT_NAME)

#define CALORECGPU_MOMENTS_OUTPUT_HELPER_11(VAR_NAME, MOMENT_NAME)                                                \
  if (moments_to_add[xAOD::CaloCluster:: MOMENT_NAME])                                                            \
    {                                                                                                             \
      cluster->insertMoment(xAOD::CaloCluster:: MOMENT_NAME , clusters->moments. VAR_NAME [cluster_index]);       \
    }

#define CALORECGPU_MOMENTS_OUTPUT_HELPER_10(VAR_NAME, MOMENT_NAME)                                                \
  if (output_extra_moments && moments_to_add[xAOD::CaloCluster:: MOMENT_NAME])                                    \
    {                                                                                                             \
      cluster->insertMoment(xAOD::CaloCluster:: MOMENT_NAME , clusters->moments. VAR_NAME [cluster_index]);       \
    }

#define CALORECGPU_MOMENTS_OUTPUT_HELPER_00(...)
#define CALORECGPU_MOMENTS_OUTPUT_HELPER_01(...)

          CALORECGPU_FORALLMOMENTS_INSTANTIATE(CALORECGPU_MOMENTS_OUTPUT_HELPER)
        }
    }
}


void CaloRecGPU::EventDataHolder::exportClusters(void * p_cluster_collection,
                                                 const void * p_cell_collection_link,
                                                 const MomentsOptionsArray & moments_to_add,
                                                 bool sort_clusters,
                                                 const bool save_uncalibrated,
                                                 const bool output_extra_moments,
                                                 const std::vector<int> & extra_cells_to_fill,
                                                 size_t * time_measurements)
{
  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  xAOD::CaloClusterContainer * cluster_collection = static_cast<xAOD::CaloClusterContainer *>(p_cluster_collection);
  const DataLink<CaloCellContainer> & cell_collection_link = *(static_cast<const DataLink<CaloCellContainer> *>(p_cell_collection_link));

  const auto start = clock_type::now();

  std::vector<std::unique_ptr<CaloClusterCellLink>> cell_links;

  export_cluster_initialize_links(m_clusters, cell_links, cell_collection_link, false);

  const auto after_link_creation = clock_type::now();

  export_cluster_process_cells(m_clusters, m_cell_info, cell_links, extra_cells_to_fill);

  const auto after_cell_processing = clock_type::now();

  std::vector<int> cluster_order;

  export_cluster_sort(m_clusters, cluster_order, sort_clusters);

  const auto after_sorting = clock_type::now();

  export_cluster_fill_cells_and_basic_info(m_clusters, cluster_collection, cell_links, cluster_order, save_uncalibrated);

  const auto after_basic_info = clock_type::now();

  export_cluster_fill_moments(m_clusters, cluster_collection, cluster_order, moments_to_add, output_extra_moments);

  const auto after_moments = clock_type::now();

  if (time_measurements)
    {
      time_measurements[0] = time_cast(start, after_link_creation);
      time_measurements[1] = time_cast(after_link_creation, after_cell_processing);
      time_measurements[2] = time_cast(after_cell_processing, after_sorting);
      time_measurements[3] = time_cast(after_sorting, after_basic_info);
      time_measurements[4] = time_cast(after_basic_info, after_moments);
    }
}


void CaloRecGPU::EventDataHolder::returnAndExportClusters(void * p_cluster_collection,
                                                          const void * p_cell_collection_link,
                                                          const MomentsOptionsArray & moments_to_add,
                                                          bool sort_clusters,
                                                          const bool save_uncalibrated,
                                                          const bool output_extra_moments,
                                                          const std::vector<int> & extra_cells_to_fill,
                                                          size_t * time_measurements,
                                                          CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  xAOD::CaloClusterContainer * cluster_collection = static_cast<xAOD::CaloClusterContainer *>(p_cluster_collection);
  const DataLink<CaloCellContainer> & cell_collection_link = *(static_cast<const DataLink<CaloCellContainer> *>(p_cell_collection_link));

  const auto start = clock_type::now();

  m_clusters.allocate();

  CaloRecGPU::CUDA_Helpers::GPU_to_CPU_async(static_cast<ClusterBaseInfo *>(m_clusters),
                                             static_cast<const ClusterBaseInfo *>(m_clusters_dev),
                                             sizeof(ClusterBaseInfo), stream);

  CaloRecGPU::CUDA_Helpers::GPU_synchronize(stream);

  const auto after_first_transfer = clock_type::now();

  cluster_transfer_helper_cell_assignment(m_clusters, m_clusters, m_clusters_dev, CaloRecGPU::CUDA_Helpers::GPU_to_CPU_async, stream, cluster_collection->size());

  std::vector<std::unique_ptr<CaloClusterCellLink>> cell_links;

  export_cluster_initialize_links(m_clusters, cell_links, cell_collection_link, true);

  const auto after_link_creation = clock_type::now();

  CaloRecGPU::CUDA_Helpers::GPU_synchronize(stream);

  cluster_transfer_helper_basic_info(m_clusters, m_clusters, m_clusters_dev, CaloRecGPU::CUDA_Helpers::GPU_to_CPU_async, stream);

  export_cluster_process_cells(m_clusters, m_cell_info, cell_links, extra_cells_to_fill);

  const auto after_cell_processing = clock_type::now();

  std::vector<int> cluster_order;

  export_cluster_sort(m_clusters, cluster_order, sort_clusters);

  const auto after_sorting = clock_type::now();

  CaloRecGPU::CUDA_Helpers::GPU_synchronize(stream);

  cluster_transfer_helper_moments(m_clusters, m_clusters, m_clusters_dev, moments_to_add, CaloRecGPU::CUDA_Helpers::GPU_to_CPU_async, stream);

  export_cluster_fill_cells_and_basic_info(m_clusters, cluster_collection, cell_links, cluster_order, save_uncalibrated);

  const auto after_basic_info = clock_type::now();

  CaloRecGPU::CUDA_Helpers::GPU_synchronize(stream);

  export_cluster_fill_moments(m_clusters, cluster_collection, cluster_order, moments_to_add, output_extra_moments);

  const auto after_moments = clock_type::now();

  if (time_measurements)
    {
      time_measurements[0] = time_cast(start, after_first_transfer);
      time_measurements[1] = time_cast(after_first_transfer, after_link_creation);
      time_measurements[2] = time_cast(after_link_creation, after_cell_processing);
      time_measurements[3] = time_cast(after_cell_processing, after_sorting);
      time_measurements[4] = time_cast(after_sorting, after_basic_info);
      time_measurements[5] = time_cast(after_basic_info, after_moments);
    }
}

void CaloRecGPU::EventDataHolder::allocate(const bool also_GPU)
{
  m_cell_info.allocate();
  m_clusters.allocate();

  if (also_GPU)
    {
      m_cell_info_dev.allocate();
      m_clusters_dev.allocate();
    }
}

void CaloRecGPU::EventDataHolder::clear_GPU()
{
  m_cell_info_dev.clear();
  m_clusters_dev.clear();
}
