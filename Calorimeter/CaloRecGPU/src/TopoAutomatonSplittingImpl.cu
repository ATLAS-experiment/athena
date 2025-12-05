//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_INCLUDE_ITERATION_COUNTERS

  #define CALORECGPU_INCLUDE_ITERATION_COUNTERS 0

#endif

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "TopoAutomatonSplittingImpl.h"

#ifndef CALORECGPU_USE_INDIVIDUAL_TEMPORARY_ARRAYS

#define CALORECGPU_TEMP_STRUCT_TO_USE TASTemps

#endif

#include "TemporaryHelpers.h"

#include "IterateUntilCondition.h"

#include "CaloIdentifier/LArNeighbours.h"
//It's just a struct.

#include "CLHEP/Units/SystemOfUnits.h"
//Probably will also work, given that it's just constexpr stuff.

#include <cstring>
#include <cmath>
#include <cassert>
#include <iostream>
#include <stdio.h>

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

using namespace CaloRecGPU;
using namespace TASplitting;

void TASplitting::TASOptionsHolder::sendToGPU(const bool clear_CPU)
{
  m_options_dev = m_options;
  if (clear_CPU)
    {
      m_options.clear();
    }
}

namespace TASTemporaries
{
  struct TASTemps;
  
  CALORECGPU_TEMPWRAPPER(secondary_array, secondary_tag_array);
  
  CALORECGPU_TEMPWRAPPER(tertiary_array,   tertiary_tag_array);
  //Indexed by hash_ID rather than cell index
  CALORECGPU_TEMPCELLARR_1(cell_to_new_cluster_map, engCalibEMB0, engCalibEME0, engCalibTileG3, unsigned int);

  static_assert(static_cast<unsigned int>(NMaxClusters - 1) * static_cast<unsigned int>(NMaxClusters + 1) <= std::numeric_limits<unsigned int>::max());

  CALORECGPU_TEMPCELLARR_1(cell_to_old_cluster_map, engCalibDeadL, engCalibDeadM, engCalibDeadT, int);

  CALORECGPU_TEMPARR_1(old_to_new_cluster_map, engCalibOutT, int);

  CALORECGPU_TEMPARR_1(new_seed_cells, engCalibOutM, int);

  CALORECGPU_TEMPARR_1(cluster_E,      engCalibTot, float);
  CALORECGPU_TEMPARR_1(cluster_E_corr, engCalibOutL, float);

  CALORECGPU_TEMPARR_1(cluster_abs_E,      eta2CaloFrame, float);
  CALORECGPU_TEMPARR_1(cluster_abs_E_corr, phi2CaloFrame, float);

  CALORECGPU_TEMPARR_1(cluster_X,      eta1CaloFrame, float);
  CALORECGPU_TEMPARR_1(cluster_X_corr, phi1CaloFrame, float);

  CALORECGPU_TEMPARR_1(cluster_Y,      etaCaloFrame, float);
  CALORECGPU_TEMPARR_1(cluster_Y_corr, phiCaloFrame, float);

  CALORECGPU_TEMPARR_1(cluster_Z,       vertexFraction, float);
  CALORECGPU_TEMPARR_1(cluster_Z_corr, nVertexFraction, float);

  CALORECGPU_TEMPARR_1(old_cluster_validity, nExtraCellSampling, int);

  CALORECGPU_TEMPCELLARR_1(local_maxima_detection, hadWeight, OOCweight, DMweight, int);
  //This has to be an int despite technically fitting in a char
  //since non-atomic 32 bit read/writes would be okay.
  
  CALORECGPU_TEMPBIGARR_3(pairs_1, energyPerSample, maxEPerSample, maxPhiPerSample, int);
  CALORECGPU_TEMPBIGARR_3(pairs_2, maxEtaPerSample, etaPerSample, phiPerSample, int);
  //Main pairs proper stored from 0 to num_main_pairs,
  //extra pairs for maxima stored in reverse, from NExactPairs - num_extra_pairs to NExactPairs,
  //next pairs stored from NExactPairs to NExactPairs + num_next_pairs,
  //prev pairs stored in reverse, from 2 * NExactPairs - num_prev_pairs to 2 * NExactPairs.
  //
  // 0                              NExactPairs                       2 * NExactPairs
  // ^                                   ^                                   ^
  // |                                   |                                   |
  // +-----------------------------------+-----------------------------------+
  // |                                   |                                   |
  // |normal pairs>          <extra pairs|next pairs>             <prev pairs|
  //
  

  CALORECGPU_TEMPVAR(num_main_pairs,  firstPhi, 0, int);
  CALORECGPU_TEMPVAR(num_extra_pairs, firstPhi, 1, int);

  CALORECGPU_TEMPVAR(num_next_pairs, firstEta, 0, int);
  CALORECGPU_TEMPVAR(num_prev_pairs, firstEta, 1, int);

  CALORECGPU_TEMPVAR(continue_flag,             secondR, 0, int);
  CALORECGPU_TEMPVAR(stop_flag,            secondLambda, 0, int);

  CALORECGPU_TEMPVAR(num_new_clusters, deltaPhi, 0, int);
  CALORECGPU_TEMPVAR(num_final_clusters, deltaTheta, 0, int);
  
  //We will only use the two first, but easier to declare this as array...
  CALORECGPU_TEMPARR_1(reset_counters, deltaAlpha, int);

  CALORECGPU_TEMPVAR(iterate_storage, centerX, 0, IterateUntilCondition::Storage);

#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
  CALORECGPU_TEMPVAR(total_primary,  time, 0, int);
  CALORECGPU_TEMPVAR(total_secondary, time, 1, int);
  CALORECGPU_TEMPVAR(eliminated_primary, time, 2, int);
  CALORECGPU_TEMPVAR(eliminated_secondary, time, 3, int);
#endif
    
  struct TASTemps
  {
    int continue_flag;
    int stop_flag;
    
    int num_main_pairs;
    int num_extra_pairs;
    int num_next_pairs;
    int num_prev_pairs;

    int num_new_clusters;
    int num_final_clusters;

    int total_primary;
    int total_secondary;
    int eliminated_primary;
    int eliminated_secondary;

    int reset_counters                   [NMaxClusters];
    
    int old_to_new_cluster_map           [NMaxClusters];
    int new_seed_cells                   [NMaxClusters];
    
    float cluster_E                      [NMaxClusters];
    float cluster_E_corr                 [NMaxClusters];
    float cluster_abs_E                  [NMaxClusters];
    float cluster_abs_E_corr             [NMaxClusters];
    float cluster_X                      [NMaxClusters];
    float cluster_X_corr                 [NMaxClusters];
    float cluster_Y                      [NMaxClusters];
    float cluster_Y_corr                 [NMaxClusters];
    float cluster_Z                      [NMaxClusters];
    float cluster_Z_corr                 [NMaxClusters];
    
    int old_cluster_validity             [NMaxClusters];
    
             int cell_to_old_cluster_map [NCaloCells];
    unsigned int cell_to_new_cluster_map [NCaloCells];

    int local_maxima_detection           [NCaloCells];
    
    int pairs_1                          [2 * NExactPairs];
    int pairs_2                          [2 * NExactPairs];

    IterateUntilCondition::Storage iterate_storage;
  };
  

}

constexpr static int FillNeighboursBlockSize = 512;

constexpr static int CountInferiorNeighsBlockSize = 512;
constexpr static int FindLocalMaximaBlockSize = 512;

constexpr static int ExcludeMaximaPropagationBlockSize = 1024;

constexpr static int ClusterSplittingMainPropagationBlockSize = 1024;

constexpr static int SumCellsBlockSize = 512;
constexpr static int CalculateCentroidsBlockSize = 512;
constexpr static int FinalCellsBlockSize = 512;

//These numbers are not at all optimized,
//just going from rough similarity to TAC operations
//(which themselves are not that optimised
// since they were last tested on a previous version...)

/******************************************************************************************
 * Determine the same-cluster neighbours of the cells and fill the pairs list accordingly.
 ******************************************************************************************/

static __global__
void fillMainExtraNeighboursKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                   const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                   const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                   const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts,
                                   const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  const int num_cells = cell_info_arr->number;

  constexpr int WarpSize = 32;
      
  for (int cell = index; (cell/WarpSize) * WarpSize < num_cells; cell += grid_size)
    {    
      int neighbours[NMaxNeighbours];
      int num_normal = 0, num_extra = 0, num_limited = 0, total_neighs = 0;

      const int hash_ID = (cell < num_cells ? cell_info_arr->get_hash_ID(cell, assume_complete_cells) : -1);
      const ClusterTag this_tag = (cell < num_cells ? clusters_arr->cells.tags[cell] : 0);

      const bool is_limited = ( hash_ID < 0                                                              )  ||
                              ( opts->limit_HECIW_and_FCal_neighs && geometry->is_HECIW_or_FCal(hash_ID) )  ||
                              ( opts->limit_PS_neighs             && geometry->is_PS(hash_ID)            );
      //The cells that have limited neighbours, for the split cluster growing part.
      //WARNING: the CPU version of the code does not limit PS neighbours ever, but we give additional freedom
      //         (even if it is disabled by default).
      //The hash_ID < 0 part is just to skip the checks if we already know the cell is not part of the collection...
                      
      if (hash_ID >= 0 && this_tag.is_part_of_cluster())
        {
          const unsigned int limited_flags   = LArNeighbours::neighbourOption::nextInSamp & opts->neighbour_options;

          const unsigned int remaining_flags = (~limited_flags) & opts->neighbour_options;

          num_limited = geometry->get_neighbours(limited_flags, hash_ID, neighbours);

          const int num_rest = geometry->get_neighbours(remaining_flags, hash_ID, neighbours + num_limited);

          total_neighs = num_limited + num_rest;

          for (int i = 0; i < total_neighs; ++i)
            {
              const int neigh_hash_ID = neighbours[i];
              const int neigh_cell = cell_info_arr->get_cell_with_hash_ID(neigh_hash_ID, assume_complete_cells);

              if (neigh_cell >= 0)
                {
                  const ClusterTag neigh_tag = clusters_arr->cells.tags[neigh_cell];
                  if (neigh_tag.is_part_of_cluster() && this_tag.cluster_index() == neigh_tag.cluster_index())
                    {
                      neighbours[i] = neigh_cell;
                      if (is_limited && i >= num_limited)
                        {
                          ++num_extra;
                        }
                      else
                        {
                          ++num_normal;
                        }
                    }
                  else
                    {
                      neighbours[i] = -1;
                    }
                }
              else
                {
                  neighbours[i] = -1;
                }
            }
        }

      constexpr unsigned int full_mask = 0xFFFFFFFFU;
      const int intra_warp_index = threadIdx.x % WarpSize;

      int normal_prefix = num_normal;
      int extra_prefix  = num_extra;
      
      for (int i = 1; i < WarpSize; i *= 2)
        {
          const int other_normal = __shfl_down_sync (full_mask, normal_prefix, i) * (intra_warp_index + i < WarpSize);
          const int other_extra  = __shfl_up_sync   (full_mask, extra_prefix,  i) * (intra_warp_index >= i);

          normal_prefix += other_normal;
          extra_prefix  += other_extra;
        }

      int base_normal_offset = 0;
      int base_extra_offset  = 0;

      switch (intra_warp_index)
        {
          case 0:
            base_normal_offset = atomicAdd(TASTemporaries::num_main_pairs_ptr(clusters_arr),  normal_prefix);
            break;
          case WarpSize - 1:
            base_extra_offset  = atomicAdd(TASTemporaries::num_extra_pairs_ptr(clusters_arr), extra_prefix);
            break;
          default:
            break;
        }

      const int overall_normal_offset = __shfl_sync(full_mask, base_normal_offset,           0);
      const int overall_extra_offset  = __shfl_sync(full_mask, base_extra_offset, WarpSize - 1);
      
      const int normal_offset = overall_normal_offset + normal_prefix - num_normal;
      const int extra_offset  = overall_extra_offset  + extra_prefix  - num_extra;

      int normal_pair_index   = normal_offset;
      int extra_pair_index    = NExactPairs - extra_offset - num_extra;

      for (int i = 0; i < num_limited; ++i)
        {
          const int neigh = neighbours[i];

          if (neigh >= 0)
            {
              TASTemporaries::pairs_1(clusters_arr, normal_pair_index) = neigh;
              TASTemporaries::pairs_2(clusters_arr, normal_pair_index) = cell;
              ++normal_pair_index;
            }
        }
      for (int i = num_limited; i < total_neighs; ++i)
        {
          const int neigh = neighbours[i];

          if (neigh >= 0)
            {
              int & pair_index = (is_limited ? extra_pair_index : normal_pair_index);
              TASTemporaries::pairs_1(clusters_arr, pair_index) = neigh;
              TASTemporaries::pairs_2(clusters_arr, pair_index) = cell;
              ++pair_index;
            }
        }

      if (hash_ID >= 0 && this_tag.is_part_of_cluster())
        {
          TASTemporaries::local_maxima_detection(clusters_arr, cell) = num_normal + num_extra;
        }
      else if (cell < num_cells)
        {
          TASTemporaries::local_maxima_detection(clusters_arr, cell) = -NMaxNeighbours;
        }
    }
}

static __global__
void fillNextPrevNeighboursKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                  const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                  const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                  const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts,
                                  const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  const int num_cells = cell_info_arr->number;

  constexpr int WarpSize = 32;
      
  for (int cell = index; (cell/WarpSize) * WarpSize < num_cells; cell += grid_size)
    {    
      int neighbours[NMaxNeighbours];
      int num_next = 0, num_prev = 0, total_neighs = 0;
      int real_num_next = 0, real_num_prev = 0;

      const int hash_ID = (cell < num_cells ? cell_info_arr->get_hash_ID(cell, assume_complete_cells) : -1);
      
      if (hash_ID >= 0)
        {
          const unsigned int next_flags = ( LArNeighbours::neighbourOption::nextSuperCalo |
                                            LArNeighbours::neighbourOption::nextInSamp      ) & opts->neighbour_options;

          const unsigned int prev_flags = ( LArNeighbours::neighbourOption::prevSuperCalo |
                                            LArNeighbours::neighbourOption::prevInSamp      ) & opts->neighbour_options;

          num_next = geometry->get_neighbours(next_flags, hash_ID, neighbours);
          num_prev = geometry->get_neighbours(prev_flags, hash_ID, neighbours + num_next);
          total_neighs = num_next + num_prev;

          for (int i = 0; i < total_neighs; ++i)
            {
              const int neigh_hash_ID = neighbours[i];
              const int neigh_cell = cell_info_arr->get_cell_with_hash_ID(neigh_hash_ID, assume_complete_cells);
              if (neigh_cell >= 0)
                {
                  neighbours[i] = neigh_cell;
              
                  if (i < num_next)
                    {
                      ++real_num_next;
                    }
                  else if (i >= num_next && i < num_next + num_prev)
                    {
                      ++real_num_prev;
                    }
                }
              else
                {
                  neighbours[i] = -1;
                }
            }
        }

      constexpr unsigned int full_mask = 0xFFFFFFFFU;
      const int intra_warp_index = threadIdx.x % WarpSize;

      int next_prefix   = real_num_next;
      int prev_prefix   = real_num_prev;
      
      for (int i = 1; i < WarpSize; i *= 2)
        {
          const int other_next   = __shfl_down_sync (full_mask, next_prefix,   i) * (intra_warp_index + i < WarpSize);
          const int other_prev   = __shfl_up_sync   (full_mask, prev_prefix,   i) * (intra_warp_index >= i);

          next_prefix   += other_next;
          prev_prefix   += other_prev;
        }

      int base_next_offset   = 0;
      int base_prev_offset   = 0;

      switch (intra_warp_index)
        {
          case 0:
            base_next_offset   = atomicAdd(TASTemporaries::num_next_pairs_ptr(clusters_arr),  next_prefix);
            break;
          case WarpSize - 1:
            base_prev_offset   = atomicAdd(TASTemporaries::num_prev_pairs_ptr(clusters_arr),  prev_prefix);
            break;
          default:
            break;
        }

      const int overall_next_offset   = __shfl_sync(full_mask, base_next_offset,             0);
      const int overall_prev_offset   = __shfl_sync(full_mask, base_prev_offset,  WarpSize - 1);
      
      const int next_offset   = overall_next_offset   + next_prefix   - real_num_next;
      const int prev_offset   = overall_prev_offset   + prev_prefix   - real_num_prev;

      int next_pair_index     = NExactPairs + next_offset;
      int prev_pair_index     = 2 * NExactPairs - prev_offset - real_num_prev;

      for (int i = 0; i < num_next; ++i)
        {
          const int neigh = neighbours[i];

          if (neigh >= 0)
            {
              TASTemporaries::pairs_1(clusters_arr, next_pair_index) = neigh;
              TASTemporaries::pairs_2(clusters_arr, next_pair_index) = cell;
              ++next_pair_index;
            }
        }
      for (int i = num_next; i < total_neighs; ++i)
        {
          const int neigh = neighbours[i];

          if (neigh >= 0)
            {
              TASTemporaries::pairs_1(clusters_arr, prev_pair_index) = neigh;
              TASTemporaries::pairs_2(clusters_arr, prev_pair_index) = cell;
              ++prev_pair_index;
            }
        }
    }
}

void TASplitting::fillNeighbours(EventDataHolder & holder,
                                 const ConstantDataHolder & instance_data,
                                 const TASOptionsHolder & options,
                                 const IGPUKernelSizeOptimizer & optimizer,
                                 const bool synchronize,
                                 CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  if (holder.m_clusters.valid())
    {
      assert(!holder.m_clusters->has_cells_per_cluster());
      //We assume the information is in tag-based mode.
    }

  cudaMemsetAsync(TASTemporaries::num_main_pairs_ptr (holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
  cudaMemsetAsync(TASTemporaries::num_extra_pairs_ptr(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
  cudaMemsetAsync(TASTemporaries::num_next_pairs_ptr (holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
  cudaMemsetAsync(TASTemporaries::num_prev_pairs_ptr (holder.m_clusters_dev), 0, sizeof(int), stream_to_use);

  const CUDAKernelLaunchConfiguration config_main_extra = optimizer.get_launch_configuration("TopoAutomatonSplitting", 0);
  const CUDAKernelLaunchConfiguration config_next_prev  = optimizer.get_launch_configuration("TopoAutomatonSplitting", 1);

  fillMainExtraNeighboursKernel <<< config_main_extra.grid_x, config_main_extra.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                                             holder.m_cell_info_dev,
                                                                                                             instance_data.m_geometry_dev,
                                                                                                             options.m_options_dev,
                                                                                                             holder.m_cell_info->complete);

  fillNextPrevNeighboursKernel <<< config_next_prev.grid_x, config_next_prev.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                                          holder.m_cell_info_dev,
                                                                                                          instance_data.m_geometry_dev,
                                                                                                          options.m_options_dev,
                                                                                                          holder.m_cell_info->complete);
  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}


/******************************************************************************************
 * Determine the local maxima and initialize the cell arrays appropriately.
 ******************************************************************************************/

static __global__
void countInferiorNeighsKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                               const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                               const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                               const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts,
                               const bool assume_complete_cells)
{
  const int num_normal_pairs = TASTemporaries::num_main_pairs(clusters_arr);
  const int num_extra_pairs = TASTemporaries::num_extra_pairs(clusters_arr);
  const int start_extra_pairs = NExactPairs - num_extra_pairs;
  const int num_total_pairs = num_normal_pairs + num_extra_pairs;

  const int thread_index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  for (int pair = thread_index; pair < num_total_pairs; pair += grid_size)
    {
      const int real_pair = ( pair >= num_normal_pairs ?
                              start_extra_pairs + pair - num_normal_pairs : pair);

      const int this_cell = TASTemporaries::pairs_1(clusters_arr, real_pair);
      const int neigh_cell = TASTemporaries::pairs_2(clusters_arr, real_pair);

      const int this_hash_ID = cell_info_arr->get_hash_ID(this_cell, assume_complete_cells);
      const int neigh_hash_ID = cell_info_arr->get_hash_ID(neigh_cell, assume_complete_cells);

      const int this_sampling = geometry->sampling(this_hash_ID);
      const int neigh_sampling = geometry->sampling(neigh_hash_ID);

      float this_energy = 0.f, neigh_energy = 0.f;

      if (!cell_info_arr->is_bad(this_cell, opts->treat_L1_predicted_as_good, assume_complete_cells) && opts->uses_sampling(this_sampling))
        {
          this_energy = cell_info_arr->energy[this_cell];
          if (opts->use_absolute_energy)
            {
              this_energy = fabsf(this_energy);
            }
          else if (this_energy <= 0.f)
            {
              this_energy = 0.f;
            }
        }

      if (!cell_info_arr->is_bad(neigh_cell, opts->treat_L1_predicted_as_good, assume_complete_cells) && opts->uses_sampling(neigh_sampling))
        {
          neigh_energy = cell_info_arr->energy[neigh_cell];
          if (opts->use_absolute_energy)
            {
              neigh_energy = fabsf(neigh_energy);
            }
          else if (neigh_energy <= 0.f)
            {
              neigh_energy = 0.f;
            }          
        }

      bool is_max_neig = neigh_energy > this_energy;

      if (opts->uses_primary_sampling(neigh_sampling))
        {
          if (!opts->uses_primary_sampling(this_sampling) && opts->uses_secondary_sampling(this_sampling))
            {
              is_max_neig = true;
            }
        }

      if (!is_max_neig)
        {
         TASTemporaries::local_maxima_detection(clusters_arr, neigh_cell) = -NCaloCells;
        }
    }

  if (thread_index == 0)
    {
      TASTemporaries::num_new_clusters(clusters_arr) = 0;
      
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      TASTemporaries::total_primary(clusters_arr) = 0;
      TASTemporaries::total_secondary(clusters_arr) = 0;
      TASTemporaries::eliminated_primary(clusters_arr) = 0;
      TASTemporaries::eliminated_secondary(clusters_arr) = 0;
#endif
    }
}

static __global__
void findLocalMaximaKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                           const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                           const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                           const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts,
                           const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cell = index; cell < cell_info_arr->number; cell += grid_size)
    {
      if (cell < clusters_arr->number)
        {
          const int one_cluster_index = cell;
          TASTemporaries::old_cluster_validity(clusters_arr, one_cluster_index) = 0;
          //The least bad place to initialize this for later...
        }

      const ClusterTag this_tag = clusters_arr->cells.tags[cell];
      const int cell_hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);

      if (this_tag.is_part_of_cluster())
        {
          const int this_sampling = geometry->sampling(cell_hash_ID);

          float cell_energy = 0.f;
          const float raw_cell_energy = cell_info_arr->energy[cell];

          if (!cell_info_arr->is_bad(cell, opts->treat_L1_predicted_as_good, assume_complete_cells) && opts->uses_sampling(this_sampling))
            {
              cell_energy = raw_cell_energy;
              if (opts->use_absolute_energy)
                {
                  cell_energy = fabsf(cell_energy);
                }
              else if (cell_energy <= 0.f)
                {
                  cell_energy = 0.f;
                }
            }

          const int num_neighs = TASTemporaries::local_maxima_detection(clusters_arr, cell);

          bool is_primary = false, is_maximum = false;

          if (/*num_neighs >= 0 && */ num_neighs >= opts->min_num_cells && cell_energy >= opts->min_maximum_energy)
            {
              if (opts->uses_primary_sampling(this_sampling))
                {
                  is_maximum = true;
                  is_primary = true;
                }
              else if (opts->uses_secondary_sampling(this_sampling))
                {
                  is_maximum = true;
                  is_primary = false;
                }
            }

          const int original_cluster = this_tag.cluster_index();

          if (is_maximum)
            {              
              const TASTag new_tag = TASTag::make_maximum_tag(cell_hash_ID, __float_as_uint(raw_cell_energy), is_primary);

              clusters_arr->cells.tags[cell] = new_tag;

              if (opts->valid_sampling_secondary != 0)
                //If we have any sampling marked as giving rise
                //to secondary maxima, we will proceed with
                //secondary maxima elimination, so we should
                //set things up properly.
                {
                  if (is_primary)
                    {
                      const int new_cluster = atomicAdd(TASTemporaries::num_new_clusters_ptr(clusters_arr), 1);

                      TASTemporaries::cell_to_new_cluster_map(clusters_arr, cell_hash_ID) = new_cluster;

                      TASTemporaries::new_seed_cells(clusters_arr, new_cluster) = cell;
                  
                      TASTemporaries::secondary_array(clusters_arr, cell) = TASTag::secondary_maxima_eliminator();
                      TASTemporaries::tertiary_array(clusters_arr, cell)  = TASTag::secondary_maxima_eliminator();
                    }
                  else
                    {
                      TASTemporaries::secondary_array(clusters_arr, cell) = new_tag;
                      TASTemporaries::tertiary_array(clusters_arr, cell)  = new_tag;
                    }
                }
              else
                {
                  const int new_cluster = atomicAdd(TASTemporaries::num_new_clusters_ptr(clusters_arr), 1);

                  TASTemporaries::cell_to_new_cluster_map(clusters_arr, cell_hash_ID) = new_cluster;

                  TASTemporaries::new_seed_cells(clusters_arr, new_cluster) = cell;
                  //No secondary elimination means
                  //all the maxima will give rise
                  //to final clusters, we can
                  //update the table here.

                  TASTemporaries::secondary_array(clusters_arr, cell) = new_tag;
                  TASTemporaries::tertiary_array(clusters_arr, cell)  = 0;
                }
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
              if (is_primary)
                {
                  atomicAdd(TASTemporaries::total_primary_ptr(clusters_arr), 1);   
                }
              else
                {
                  atomicAdd(TASTemporaries::total_secondary_ptr(clusters_arr), 1);
                }
#endif
            }
          else
            {
              TASTemporaries::cell_to_new_cluster_map(clusters_arr, cell_hash_ID) = 0xFFFFFFFFU;

              const TASTag new_tag = TASTag::make_non_split_cluster_tag();

              clusters_arr->cells.tags[cell]                      = new_tag;
              TASTemporaries::secondary_array(clusters_arr, cell) = new_tag;
              TASTemporaries::tertiary_array(clusters_arr, cell)  = 0;
            }

          TASTemporaries::cell_to_old_cluster_map(clusters_arr, cell) = original_cluster;
        }
      else
        {
          TASTemporaries::cell_to_old_cluster_map(clusters_arr, cell) = -1;
          TASTemporaries::cell_to_new_cluster_map(clusters_arr, cell_hash_ID) = 0xFFFFFFFFU;

          const TASTag new_tag = TASTag::make_invalid_tag();

          clusters_arr->cells.tags[cell]                      = new_tag;
          TASTemporaries::secondary_array(clusters_arr, cell) = new_tag;
          TASTemporaries::tertiary_array(clusters_arr, cell)  = 0;
        }
    }

  if (index == 0)
    {
      TASTemporaries::stop_flag(clusters_arr) = 0;
      TASTemporaries::continue_flag(clusters_arr) = 0;
      TASTemporaries::reset_counters(clusters_arr, 0) = 0;
      TASTemporaries::reset_counters(clusters_arr, 1) = 0;
      
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      printf("Pairing: %d | %d | %d | %d: [%d %d[ [%d %d[ [%d %d[ [%d %d[\n",
             TASTemporaries::num_main_pairs(clusters_arr),
             TASTemporaries::num_extra_pairs(clusters_arr),
             TASTemporaries::num_next_pairs(clusters_arr),
             TASTemporaries::num_prev_pairs(clusters_arr),
             0, TASTemporaries::num_main_pairs(clusters_arr),
             NExactPairs - TASTemporaries::num_extra_pairs(clusters_arr), NExactPairs,
             NExactPairs, NExactPairs + TASTemporaries::num_next_pairs(clusters_arr),
             2 * NExactPairs - TASTemporaries::num_prev_pairs(clusters_arr), 2 * NExactPairs);        
#endif
    }
}

void TASplitting::findLocalMaxima(EventDataHolder & holder,
                                  const ConstantDataHolder & instance_data,
                                  const TASOptionsHolder & options,
                                  const IGPUKernelSizeOptimizer & optimizer,
                                  const bool synchronize,
                                  CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration cfg_neigh_count = optimizer.get_launch_configuration("TopoAutomatonSplitting", 2);
  const CUDAKernelLaunchConfiguration cfg_find_maxima = optimizer.get_launch_configuration("TopoAutomatonSplitting", 3);

  countInferiorNeighsKernel <<< cfg_neigh_count.grid_x, cfg_neigh_count.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                                     holder.m_cell_info_dev,
                                                                                                     instance_data.m_geometry_dev,
                                                                                                     options.m_options_dev,
                                                                                                     holder.m_cell_info->complete);


  findLocalMaximaKernel <<< cfg_find_maxima.grid_x, cfg_find_maxima.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                                 holder.m_cell_info_dev,
                                                                                                 instance_data.m_geometry_dev,
                                                                                                 options.m_options_dev,
                                                                                                 holder.m_cell_info->complete);
  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/*****************************************************************************
 * Delete secondary maxima according to the criteria on the CPU version.
 ******************************************************************************/

static __device__
void propagate_secondary_maxima_pair(const int pair,
                                     const bool is_prev,
                                     ClusterInfoArr * clusters_arr)
{
  const int this_index  = TASTemporaries::pairs_1(clusters_arr, pair);
  const int neigh_index = TASTemporaries::pairs_2(clusters_arr, pair);
  
  const TASTag this_tag = (is_prev ? TASTemporaries::tertiary_array(clusters_arr, this_index) : TASTemporaries::secondary_array(clusters_arr, this_index));
  tag_type * neigh_tag_ptr = (is_prev ? &TASTemporaries::tertiary_array(clusters_arr, neigh_index) : &TASTemporaries::secondary_array(clusters_arr, neigh_index));

  if (this_tag.is_secondary_maxima_eliminator() || this_tag.is_part_of_splitter_cluster())
    {
      if (atomicMax(neigh_tag_ptr, this_tag) < this_tag)
        {
          TASTemporaries::continue_flag(clusters_arr) = 1;
        }
    }
}

static __device__
void clean_up_secondary_tags(const int cell,
                             ClusterInfoArr * clusters_arr,
                             const CellInfoArr * cell_info_arr)
{
  const TASTag original_tag  = clusters_arr->cells.tags[cell];

  if (original_tag.is_part_of_splitter_cluster())
    {
      if (original_tag.is_primary_maximum())
        {
          TASTemporaries::secondary_array(clusters_arr, cell) = original_tag;
          
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
          atomicAdd(TASTemporaries::eliminated_primary_ptr(clusters_arr), 1);   
#endif
        }
      else
        {
          const TASTag tag_from_next = TASTemporaries::secondary_array(clusters_arr, cell);
          const TASTag tag_from_prev = TASTemporaries::tertiary_array(clusters_arr, cell);

          if (tag_from_next == original_tag && tag_from_prev == original_tag)
            //This maximum was not eliminated.
            {
              const int new_cluster = atomicAdd(TASTemporaries::num_new_clusters_ptr(clusters_arr), 1);

              TASTemporaries::cell_to_new_cluster_map(clusters_arr, original_tag.index()) = new_cluster;

              TASTemporaries::new_seed_cells(clusters_arr, new_cluster) = cell;

              const TASTag new_tag = original_tag.unset_secondary();
              //No distinction between primary and secondary maxima
              //for the actual tag propagation.

              clusters_arr->cells.tags[cell]                      = new_tag;
              TASTemporaries::secondary_array(clusters_arr, cell) = new_tag;
              
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
              atomicAdd(TASTemporaries::eliminated_secondary_ptr(clusters_arr), 1);
#endif
            }
          else
            {
              const TASTag new_tag = TASTag::make_non_split_cluster_tag();

              clusters_arr->cells.tags[cell]                      = new_tag;
              TASTemporaries::secondary_array(clusters_arr, cell) = new_tag;
            }
        }
    }
  else 
    {
      TASTemporaries::secondary_array(clusters_arr, cell) = original_tag;
    }
}

namespace
{
  struct SecondaryPropagationCondition
  {
    int num_pairs_next;
    int num_pairs_prev;
    int num_pairs_total;

#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
    int counter;
#endif

    __device__ bool operator() (const unsigned int /*grid_dim*/,
                                const unsigned int /*grid_index*/,
                                ClusterInfoArr * clusters_arr,
                                const CellInfoArr * /*cell_info_arr*/) const
    {
      return TASTemporaries::stop_flag(clusters_arr);
    }
  };

  struct BeforeSecondaryPropagation
  {
    __device__ void operator() (const unsigned int /*grid_dim*/,
                                const unsigned int grid_index,
                                SecondaryPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr,
                                const CellInfoArr * /*cell_info_arr*/) const
    {
      condition.num_pairs_next = TASTemporaries::num_next_pairs(clusters_arr);
      condition.num_pairs_prev = TASTemporaries::num_prev_pairs(clusters_arr);
      condition.num_pairs_total = condition.num_pairs_next + condition.num_pairs_prev;
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      condition.counter = 0;
#endif
    }
  };

  struct SecondaryPropagation
  {
    __device__ void operator() (const unsigned int grid_dim,
                                const unsigned int grid_index,
                                SecondaryPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr,
                                const CellInfoArr * /*cell_info_arr*/) const
    {
      const int index = grid_index * blockDim.x + threadIdx.x;
      const int grid_size = grid_dim * blockDim.x;

      constexpr int start_next_pairs = NExactPairs;
      const int start_prev_pairs = 2 * NExactPairs - condition.num_pairs_prev;

      for (int pair = index; pair < condition.num_pairs_total; pair += grid_size)
        {
          const bool is_prev = (pair >= condition.num_pairs_next);
          
          const int real_pair = (is_prev ? (start_prev_pairs + pair - condition.num_pairs_next) : (start_next_pairs + pair));

          propagate_secondary_maxima_pair(real_pair, is_prev, clusters_arr);
        }
    }
  };

  struct SecondaryPropagationChecker
  {
    __device__ void operator() (const unsigned int /*grid_dim*/,
                                const unsigned int grid_index,
                                [[maybe_unused]] SecondaryPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr,
                                const CellInfoArr * /*cell_info_arr*/) const
    {
      if (grid_index == 0 && threadIdx.x == 0)
        {
          if (!TASTemporaries::continue_flag(clusters_arr))
            {
              TASTemporaries::stop_flag(clusters_arr) = 1;
            }
          else
            {
              TASTemporaries::continue_flag(clusters_arr) = 0;
            }

#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
          ++condition.counter;
#endif
        }
    }
  };

  struct SecondaryPropagationFinalizer
  {
    __device__ void operator() (const unsigned int grid_dim,
                                const unsigned int grid_index,
                                [[maybe_unused]] SecondaryPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr,                              
                                const CellInfoArr * cell_info_array) const
    {
      const int index = grid_index * blockDim.x + threadIdx.x;
      const int grid_size = grid_dim * blockDim.x;

      if (index == 0)
        {
          TASTemporaries::stop_flag(clusters_arr) = 0;
          TASTemporaries::continue_flag(clusters_arr) = 0;

#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
          printf("SECONDARY SPLITTING: %16d\n", condition.counter);
#endif
        }

      for (int cell = index; cell < clusters_arr->number_cells; cell += grid_size)
        {
          clean_up_secondary_tags(cell, clusters_arr, cell_info_array);
        }
    }
  };
}

using SecondaryIterationHolder = IterateUntilCondition::Holder<SecondaryPropagationCondition,
                                                               BeforeSecondaryPropagation,
                                                               SecondaryPropagationFinalizer,
                                                               SecondaryPropagation,
                                                               SecondaryPropagationChecker>;

void TASplitting::excludeSecondaryMaxima(EventDataHolder & holder,
                                         const ConstantDataHolder & instance_data,
                                         const TASOptionsHolder & options,
                                         const IGPUKernelSizeOptimizer & optimizer,
                                         const bool synchronize,
                                         CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  if (options.m_options->valid_sampling_secondary != 0)
    {

      const CUDAKernelLaunchConfiguration cfg_iter = (optimizer.can_use_cooperative_groups() ?
                                                      optimizer.get_launch_configuration("TopoAutomatonSplitting", 4) :
                                                      optimizer.get_launch_configuration("TopoAutomatonSplitting", 5));

      SecondaryIterationHolder iterations;

      iterations.execute(optimizer.can_use_cooperative_groups(),
                         cfg_iter.grid_x, cfg_iter.block_x, 0, stream_to_use,
                         TASTemporaries::iterate_storage_ptr(holder.m_clusters_dev),
                         static_cast<ClusterInfoArr *>(holder.m_clusters_dev),
                         static_cast<CellInfoArr *>(holder.m_cell_info_dev));

    }
  else
    {
      //Do nothing as we already handled the initialization properly.
    }

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}


/******************************************************************************************
 * Propagate the new tags and create the final clusters.
 ******************************************************************************************/

static __device__
void propagate_main_pair(const int pair,
                         ClusterInfoArr * clusters_arr,
                         const bool counter_select,
                         const bool use_shared_cells)
{
  const int this_index  = TASTemporaries::pairs_1(clusters_arr, pair);
  const int neigh_index = TASTemporaries::pairs_2(clusters_arr, pair);

  const TASTag neigh_tag = clusters_arr->cells.tags[neigh_index];

  if (!neigh_tag.is_part_of_splitter_cluster())
    {
      return;
    }

  TASTag prop_tag = neigh_tag.propagate();

  const TASTag old_tag = clusters_arr->cells.tags[this_index];

  const bool can_share = ( use_shared_cells                      &&
                           old_tag.is_part_of_splitter_cluster() &&
                           !old_tag.is_shared()                  &&
                           !old_tag.is_first()                   &&
                           !neigh_tag.is_shared()                    );

  if (can_share && old_tag.counter() == prop_tag.counter())
    {
      const unsigned int old_cluster = TASTemporaries::cell_to_new_cluster_map(clusters_arr, old_tag.index());
      const unsigned int new_cluster = TASTemporaries::cell_to_new_cluster_map(clusters_arr, neigh_tag.index());

      if(old_cluster != new_cluster)
        {
          prop_tag = old_tag.prepare_for_sharing(prop_tag);
          atomicMax(TASTemporaries::reset_counters_ptr(clusters_arr, counter_select), old_tag.counter());
          TASTemporaries::continue_flag(clusters_arr) = 1;
        }
    }
  else if (neigh_tag.is_shared() && !neigh_tag.is_first() && neigh_tag.counter() > 0x7FF)
    {
      prop_tag = prop_tag.update_counter(0x7FF);
      //Shared cells after the original ones
      //are not ordered by the propagation step
      //of the original shared cell.
      //Assuming less than 2^11 = 2048 propagation steps
      //before making a shared cell seems safe-ish?
    }
  
  if (old_tag < prop_tag && (!old_tag.is_part_of_splitter_cluster()          ||
                             prop_tag.counter() > old_tag.counter()          ||
                             prop_tag.is_first()                             ||
                             (!prop_tag.is_shared() && old_tag.is_shared())     ))

    {
      atomicMax(&TASTemporaries::secondary_array(clusters_arr, this_index), prop_tag);
      TASTemporaries::continue_flag(clusters_arr) = 1;
    }
}

static __device__
void update_cell_tag(const int cell,
                     ClusterInfoArr * clusters_arr,
                     const CellInfoArr * cell_info_arr,
                     const bool counter_select,
                     const bool use_shared_cells,
                     const bool assume_complete_cells)
{
  TASTag new_tag = TASTemporaries::secondary_array(clusters_arr, cell);

  if (!new_tag.is_part_of_splitter_cluster())
    {
      return;
    }

  const int cell_hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);

  if (cell_hash_ID < 0)
    {
      return;
    }
  
  const int desired_counter = TASTemporaries::reset_counters(clusters_arr, counter_select);
  
  const TASTag old_tag = clusters_arr->cells.tags[cell];

  if (new_tag.counter() < desired_counter || (old_tag.is_part_of_splitter_cluster() && old_tag.counter() < desired_counter))
    {
      clusters_arr->cells.tags[cell]                      = TASTag::make_non_split_cluster_tag();
      TASTemporaries::secondary_array(clusters_arr, cell) = TASTag::make_non_split_cluster_tag();

      return;
    }

  if (new_tag == old_tag)
    {
      return;
    }

  const unsigned int new_cluster = TASTemporaries::cell_to_new_cluster_map(clusters_arr, new_tag.index());
  
  if ( use_shared_cells && old_tag.is_part_of_splitter_cluster() &&
       new_tag.is_shared() &&  new_tag.is_first()                &&
      !old_tag.is_shared() && !old_tag.is_first()                   )
    {
      const unsigned int old_cluster = TASTemporaries::cell_to_new_cluster_map(clusters_arr, old_tag.index());
      
      const unsigned int final_assignment = (max(new_cluster & 0xFFFFU, old_cluster & 0xFFFFU) << 16) | (min(new_cluster & 0xFFFFU, old_cluster & 0xFFFFU));
      
      new_tag = new_tag.update_counter(old_tag.counter() - 1);
      
      TASTemporaries::cell_to_new_cluster_map(clusters_arr, cell_hash_ID) = final_assignment;
    }
  else
    {      
      TASTemporaries::cell_to_new_cluster_map(clusters_arr, cell_hash_ID) = new_cluster;
    }

  const float cell_energy = cell_info_arr->energy[cell];
  new_tag = new_tag.update_cell(cell_hash_ID, __float_as_uint(cell_energy));
  
  clusters_arr->cells.tags[cell]                      = new_tag;
  TASTemporaries::secondary_array(clusters_arr, cell) = new_tag;
}

namespace
{
  struct MainPropagationCondition
  {
    bool counter_select;

    bool assume_complete_cells;
    
    int num_pairs;
    int cells_number;

#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
    int counter;
#endif

    __device__ bool operator() (const unsigned int /*grid_dim*/,
                                const unsigned int /*grid_index*/,
                                ClusterInfoArr * clusters_arr,
                                const CellInfoArr * /*cell_info_arr*/,
                                const bool /*use_shared_cells*/)
    {
      return TASTemporaries::stop_flag(clusters_arr);
    }
  };

  struct BeforeMainPropagation
  {
    __device__ void operator() (const unsigned int /*grid_dim*/,
                                const unsigned int grid_index,
                                MainPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr,
                                const CellInfoArr * cell_info_arr,
                                const bool /*use_shared_cells*/) const
    {
      condition.num_pairs = TASTemporaries::num_main_pairs(clusters_arr);
      condition.cells_number = clusters_arr->number_cells;
      
      condition.counter_select = false;

      condition.assume_complete_cells = cell_info_arr->complete;
      
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      condition.counter = 0;
      if (threadIdx.x == 0 && grid_index == 0)
        {
          printf("Split Clusters: %d (%d %d | %d %d)\n",
                 TASTemporaries::num_new_clusters(clusters_arr),
                 TASTemporaries::total_primary(clusters_arr),
                 TASTemporaries::total_secondary(clusters_arr),
                 TASTemporaries::eliminated_primary(clusters_arr),
                 TASTemporaries::eliminated_secondary(clusters_arr)                 
                 );
        }
#endif
    }
  };

  struct AfterMainPropagation
  {
    __device__ void operator() (const unsigned int grid_dim,
                                const unsigned int grid_index,
                                [[maybe_unused]] MainPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr,
                                const CellInfoArr * /*cell_info_arr*/,
                                const bool /*use_shared_cells*/) const
    {
      const int index = grid_index * blockDim.x + threadIdx.x;
      const int grid_size = grid_dim * blockDim.x;

      
      for (int cluster = index; cluster < TASTemporaries::num_new_clusters(clusters_arr); cluster += grid_size)
        {
          TASTemporaries::cluster_E(clusters_arr, cluster) = 0.f;
          TASTemporaries::cluster_E_corr(clusters_arr, cluster) = 0.f;
          TASTemporaries::cluster_abs_E(clusters_arr, cluster) = 0.f;
          TASTemporaries::cluster_abs_E_corr(clusters_arr, cluster) = 0.f;
          TASTemporaries::cluster_X(clusters_arr, cluster) = 0.f;
          TASTemporaries::cluster_X_corr(clusters_arr, cluster) = 0.f;
          TASTemporaries::cluster_Y(clusters_arr, cluster) = 0.f;
          TASTemporaries::cluster_Y_corr(clusters_arr, cluster) = 0.f;
          TASTemporaries::cluster_Z(clusters_arr, cluster) = 0.f;
          TASTemporaries::cluster_Z_corr(clusters_arr, cluster) = 0.f;
        }
      
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      if (index == 0)
        {
          printf("SPLITTING: %16d\n", condition.counter);
        }
#endif
    }
  };

  struct MainPropagation
  {
    __device__ void operator() (const unsigned int grid_dim,
                                const unsigned int grid_index,
                                MainPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr,
                                const CellInfoArr * /*cell_info_arr*/,
                                const bool use_shared_cells) const
    {
      const int index = grid_index * blockDim.x + threadIdx.x;
      const int grid_size = grid_dim * blockDim.x;

      for (int pair = index; pair < condition.num_pairs; pair += grid_size)
        {
          propagate_main_pair(pair, clusters_arr, condition.counter_select, use_shared_cells);
        }
    }
  };

  struct CellUpdate
  {
    __device__ void operator() (const unsigned int grid_dim,
                                const unsigned int grid_index,
                                MainPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr,
                                const CellInfoArr * cell_info_arr,
                                const bool use_shared_cells) const
    {
      const int index = grid_index * blockDim.x + threadIdx.x;
      const int grid_size = grid_dim * blockDim.x;

      for (int cell = index; cell < condition.cells_number; cell += grid_size)
        {
          update_cell_tag(cell, clusters_arr, cell_info_arr, condition.counter_select, use_shared_cells, condition.assume_complete_cells);
        }

      condition.counter_select = !condition.counter_select;
      
      if (index == 0)
        {
          if (!TASTemporaries::continue_flag(clusters_arr))
            {
              TASTemporaries::stop_flag(clusters_arr) = 1;
            }
          else
            {
              TASTemporaries::continue_flag(clusters_arr) = 0;
              TASTemporaries::reset_counters(clusters_arr, condition.counter_select) = 0;
            }
        }

      
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      ++condition.counter;
#endif
    }
  };
}

using MainIterationHolder = IterateUntilCondition::Holder<MainPropagationCondition,
                                                          BeforeMainPropagation,
                                                          AfterMainPropagation,
                                                          MainPropagation,
                                                          CellUpdate>;

void TASplitting::splitClusterGrowing(EventDataHolder & holder,
                                      const ConstantDataHolder & instance_data,
                                      const TASOptionsHolder & options,
                                      const IGPUKernelSizeOptimizer & optimizer,
                                      const bool synchronize,
                                      CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration cfg_iter = (optimizer.can_use_cooperative_groups() ?
                                                  optimizer.get_launch_configuration("TopoAutomatonSplitting", 6) :
                                                  optimizer.get_launch_configuration("TopoAutomatonSplitting", 7));


  MainIterationHolder iterations;

  iterations.execute(optimizer.can_use_cooperative_groups(),
                     cfg_iter.grid_x, cfg_iter.block_x, 0, stream_to_use,
                     TASTemporaries::iterate_storage_ptr(holder.m_clusters_dev),
                     static_cast<ClusterInfoArr *>(holder.m_clusters_dev),
                     static_cast<const CellInfoArr *>(holder.m_cell_info_dev),
                     options.m_options->share_border_cells);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/******************************************************************************************
 * Calculate the cell weights (only if indeed using shared_cells) and finalize clusters.
 ******************************************************************************************/


static __global__
void sumCellsForCentroidKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                               const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                               const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                               const bool use_shared_cells,
                               const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cell = index; cell < clusters_arr->number_cells; cell += grid_size)
    {
      const TASTag tag = clusters_arr->cells.tags[cell];

      if (tag.is_part_of_splitter_cluster() && use_shared_cells && !tag.is_shared())
        {
          const int hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);
          
          const unsigned int cluster = TASTemporaries::cell_to_new_cluster_map(clusters_arr, hash_ID);

          const float energy = cell_info_arr->energy[cell];
          const float abs_energy = fabsf(energy);
          const float x = geometry->x[hash_ID];
          const float y = geometry->y[hash_ID];
          const float z = geometry->z[hash_ID];

          Helpers::device_kahan_babushka_neumaier(TASTemporaries::cluster_E_ptr(clusters_arr, cluster),
                                                  TASTemporaries::cluster_E_corr_ptr(clusters_arr, cluster),
                                                  energy);
                                         
          Helpers::device_kahan_babushka_neumaier(TASTemporaries::cluster_abs_E_ptr(clusters_arr, cluster),
                                                  TASTemporaries::cluster_abs_E_corr_ptr(clusters_arr, cluster),
                                                  abs_energy);
                                         
          Helpers::device_kahan_babushka_neumaier(TASTemporaries::cluster_X_ptr(clusters_arr, cluster),
                                                  TASTemporaries::cluster_X_corr_ptr(clusters_arr, cluster),
                                                  abs_energy * x);
                                         
          Helpers::device_kahan_babushka_neumaier(TASTemporaries::cluster_Y_ptr(clusters_arr, cluster),
                                                  TASTemporaries::cluster_Y_corr_ptr(clusters_arr, cluster),
                                                  abs_energy * y);
                                         
          Helpers::device_kahan_babushka_neumaier(TASTemporaries::cluster_Z_ptr(clusters_arr, cluster),
                                                  TASTemporaries::cluster_Z_corr_ptr(clusters_arr, cluster),
                                                  abs_energy * z);
        }
      else if (!tag.is_part_of_splitter_cluster() && tag.is_valid())
        {
          const int old_cluster = TASTemporaries::cell_to_old_cluster_map(clusters_arr, cell);

          TASTemporaries::old_cluster_validity(clusters_arr, old_cluster) = 1;
        }
    }

  if (index == 0)
    {
      TASTemporaries::num_final_clusters(clusters_arr) = TASTemporaries::num_new_clusters(clusters_arr);
    }
}

constexpr static int mark_revalidated_clusters = 0x40000000;
//This marks clusters for which we don't have a seed cell
//(to be calculated in the second cell pass).
//Obviously higher than the maximum number of clusters we could ever have.

static __global__
void calculateCentroidsKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const bool use_shared_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  const int original_cluster_number = clusters_arr->number;
  const int new_cluster_number      = TASTemporaries::num_new_clusters(clusters_arr);

  const int total_number_to_handle = original_cluster_number + new_cluster_number;

  for (int cluster = index; cluster < total_number_to_handle; cluster += grid_size)
    {
      if (cluster < original_cluster_number)
        {
          const bool cluster_valid = (TASTemporaries::old_cluster_validity(clusters_arr, cluster) > 0);
          if (cluster_valid)
            {
              const int new_index = atomicAdd(TASTemporaries::num_final_clusters_ptr(clusters_arr), 1);

              const int old_seed_cell = clusters_arr->seedCellIndex[cluster];

              TASTemporaries::old_to_new_cluster_map(clusters_arr, cluster) = new_index | (mark_revalidated_clusters * (old_seed_cell < 0));

              TASTemporaries::new_seed_cells(clusters_arr, new_index) = old_seed_cell;
            }
        }
      else if (use_shared_cells)
        {
          const int real_cluster = cluster - original_cluster_number;
          
          TASTemporaries::cluster_E(clusters_arr, real_cluster) += TASTemporaries::cluster_E_corr(clusters_arr, real_cluster);

          const float abs_energy = TASTemporaries::cluster_abs_E(clusters_arr, real_cluster) + TASTemporaries::cluster_abs_E_corr(clusters_arr, real_cluster);

          const float rev_abs_E = (abs_energy > 0.f ? 1.0f / abs_energy : 1.0f);
          //Could there be a better floating point algorithm for (a + b)/(c + d)?
          //https://stackoverflow.com/a/38933262, but I'm not sure if it is worth it here...

          TASTemporaries::cluster_X(clusters_arr, real_cluster) = (TASTemporaries::cluster_X(clusters_arr, real_cluster) +
                                                                   TASTemporaries::cluster_X_corr(clusters_arr, real_cluster)) * rev_abs_E;
                                                              
          TASTemporaries::cluster_Y(clusters_arr, real_cluster) = (TASTemporaries::cluster_Y(clusters_arr, real_cluster) +
                                                                   TASTemporaries::cluster_Y_corr(clusters_arr, real_cluster)) * rev_abs_E;
                                                              
          TASTemporaries::cluster_Z(clusters_arr, real_cluster) = (TASTemporaries::cluster_Z(clusters_arr, real_cluster) +
                                                                   TASTemporaries::cluster_Z_corr(clusters_arr, real_cluster)) * rev_abs_E;
        }
    }
}

static __global__
void assignFinalCellsKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts,
                            const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  const int final_num_clusters = TASTemporaries::num_final_clusters(clusters_arr);

  for (int cell = index; cell < clusters_arr->number_cells; cell += grid_size)
    {
      if (cell < final_num_clusters)
        {
          const int cluster_index_for_update = cell;

          const int new_seed_cell = TASTemporaries::new_seed_cells(clusters_arr, cluster_index_for_update);

          if (new_seed_cell < 0)
            {
              atomicMax(&(clusters_arr->seedCellIndex[cluster_index_for_update]), new_seed_cell);
              //To make sure the cases where the seeds are updated from other cells work properly.
            }
          else
            {
              clusters_arr->seedCellIndex[cluster_index_for_update] = new_seed_cell;
            }
        }

      const TASTag tag = clusters_arr->cells.tags[cell];
      
      if (tag.is_part_of_splitter_cluster())
        {
          if (opts->share_border_cells && tag.is_shared())
            {
              const int hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);
              
              const uint32_t shared_clusters_packed = TASTemporaries::cell_to_new_cluster_map(clusters_arr, hash_ID);
              const int cluster_1 = shared_clusters_packed & 0xFFFFU;
              const int cluster_2 = (shared_clusters_packed >> 16) & 0xFFFFU;

              const float cell_x = geometry->x[hash_ID];
              const float cell_y = geometry->y[hash_ID];
              const float cell_z = geometry->z[hash_ID];


              const float delta_x_1 = cell_x - TASTemporaries::cluster_X(clusters_arr, cluster_1);
              const float delta_x_2 = cell_x - TASTemporaries::cluster_X(clusters_arr, cluster_2);

              const float delta_y_1 = cell_y - TASTemporaries::cluster_Y(clusters_arr, cluster_1);
              const float delta_y_2 = cell_y - TASTemporaries::cluster_Y(clusters_arr, cluster_2);

              const float delta_z_1 = cell_z - TASTemporaries::cluster_Z(clusters_arr, cluster_1);
              const float delta_z_2 = cell_z - TASTemporaries::cluster_Z(clusters_arr, cluster_2);

              const float d_1 = norm3df(delta_x_1, delta_y_1, delta_z_1);

              const float d_2 = norm3df(delta_x_2, delta_y_2, delta_z_2);

              float r_exp = (d_1 - d_2) / opts->EM_shower_scale;

              if (r_exp > 10)
                {
                  r_exp = 10;
                }
              else if (r_exp < -10)
                {
                  r_exp = -10;
                }

              const float r = expf(r_exp);
              const float r_reverse = expf(-r_exp);

              float E_1 = TASTemporaries::cluster_E(clusters_arr, cluster_1);
              float E_2 = TASTemporaries::cluster_E(clusters_arr, cluster_2);

              if (opts->use_absolute_energy)
                {
                  E_1 = fabsf(E_1);
                  E_2 = fabsf(E_2);
                }

              if (E_1 <= 0)
                {
                  E_1 = 1.0f * CLHEP::MeV;
                }
              if (E_2 <= 0)
                {
                  E_2 = 1.0f * CLHEP::MeV;
                }

              float weight = E_1 / fmaf(r, E_2, E_1);
              float rev_weight = E_2 / fmaf(r_reverse, E_1, E_2);
              //This could be made even more precise, maybe?

              //Optimization opportunity:
              //I think w_1 > w_2 is satisfied by 0 < r < E1/E2,
              //so we could save some of the computation
              //at the cost of slightly complicating the logic
              //(since we need to deal with the reverse weight
              // and ensure we always use the most accurate value).

              if (__float_as_uint(weight) == 0)
                {
                  weight == __uint_as_float(1);
                }

              if (__float_as_uint(rev_weight) == 0)
                {
                  rev_weight == __uint_as_float(1);
                }

              //This is just so that shared clusters
              //always show up as shared clusters.
              //A denormal weight is... negligible for physics.

              if (weight > 0.5f)
                {
                  clusters_arr->cells.tags[cell] = ClusterTag::make_tag(cluster_1, __float_as_uint(rev_weight), cluster_2);
                }
              else if (weight == 0.5f)
                {
                  const int max_cluster = cluster_1 > cluster_2 ? cluster_1 : cluster_2;
                  const int min_cluster = cluster_1 > cluster_2 ? cluster_2 : cluster_1;
                  clusters_arr->cells.tags[cell] = ClusterTag::make_tag(max_cluster, __float_as_uint(weight), min_cluster);
                }
              else /*if (weight < 0.5f)*/
                {
                  clusters_arr->cells.tags[cell] = ClusterTag::make_tag(cluster_2, __float_as_uint(weight), cluster_1);
                }
            }
          else
            {
              const int hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);
              const int this_cluster = TASTemporaries::cell_to_new_cluster_map(clusters_arr, hash_ID);
              clusters_arr->cells.tags[cell] = ClusterTag::make_tag(this_cluster);
            }
        }
      else if (tag.is_valid())
        {
          const int this_old_cluster = TASTemporaries::cell_to_old_cluster_map(clusters_arr, cell);

          const int this_new_cluster_plus_mark = TASTemporaries::old_to_new_cluster_map(clusters_arr, this_old_cluster);

          const int this_new_cluster = this_new_cluster_plus_mark & (~mark_revalidated_clusters);

          clusters_arr->cells.tags[cell] = ClusterTag::make_tag(this_new_cluster);

          if (this_new_cluster_plus_mark & mark_revalidated_clusters)
            {
              atomicMax(&(clusters_arr->seedCellIndex[this_new_cluster]), cell);
              //Not the seed cell, but just a consistent way of marking this cluster as still valid...
              //CPU version is also not particularly deterministic here.
              //Plus, this is a very pathological case where not all cells
              //in an old cluster get assigned to a local maximum,
              //which could ultimately lead to the unphysical situation
              //where we have non-connected portions of the same cluster...
            }
        }
      else
        {
          clusters_arr->cells.tags[cell] = ClusterTag::make_invalid_tag();
        }
    }

  if (index == 0)
    {
      clusters_arr->state = ClusterInformationState::Tags;
      clusters_arr->has_deleted_clusters = false;

      assert(final_num_clusters <= NMaxClusters);
      
      clusters_arr->number = final_num_clusters;
      
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      printf("SPLITTING Final Clusters: %d (%d)\n", final_num_clusters, TASTemporaries::num_new_clusters(clusters_arr));
#endif
    }
}


void TASplitting::cellWeightingAndFinalization(EventDataHolder & holder,
                                               const ConstantDataHolder & instance_data,
                                               const TASOptionsHolder & options,
                                               const IGPUKernelSizeOptimizer & optimizer,
                                               const bool synchronize,
                                               CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration cfg_sumcells = optimizer.get_launch_configuration("TopoAutomatonSplitting",  8);
  const CUDAKernelLaunchConfiguration cfg_centroid = optimizer.get_launch_configuration("TopoAutomatonSplitting",  9);
  const CUDAKernelLaunchConfiguration cfg_finalize = optimizer.get_launch_configuration("TopoAutomatonSplitting", 10);
     
  sumCellsForCentroidKernel <<< cfg_sumcells.grid_x, cfg_sumcells.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                               holder.m_cell_info_dev,
                                                                                               instance_data.m_geometry_dev,
                                                                                               options.m_options->share_border_cells,
                                                                                               holder.m_cell_info->complete);
      
  calculateCentroidsKernel <<< cfg_centroid.grid_x, cfg_centroid.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                              options.m_options->share_border_cells);

  assignFinalCellsKernel <<< cfg_finalize.grid_x, cfg_finalize.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                            holder.m_cell_info_dev,
                                                                                            instance_data.m_geometry_dev,
                                                                                            options.m_options_dev,
                                                                                            holder.m_cell_info->complete);
  
  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
  
  if (holder.m_clusters.valid())
    {
      holder.m_clusters->state = ClusterInformationState::Tags;
    }
}

/*******************************************************************************************************************************/

void TASplitting::register_kernels(IGPUKernelSizeOptimizer & optimizer)
{

  void * kernels[] = { (void *) fillMainExtraNeighboursKernel,
                       (void *) fillNextPrevNeighboursKernel,
                       (void *) countInferiorNeighsKernel,
                       (void *) findLocalMaximaKernel,
                       (void *) IterateUntilCondition::cooperative_kernel
                       < SecondaryIterationHolder,
                       ClusterInfoArr *,
                       const CellInfoArr *>,
                       (void *) IterateUntilCondition::normal_kernel
                       < SecondaryIterationHolder,
                       ClusterInfoArr *,
                       const CellInfoArr *>,
                       (void *) IterateUntilCondition::cooperative_kernel
                       < MainIterationHolder,
                       ClusterInfoArr *,
                       const CellInfoArr *,
                       bool >,
                       (void *) IterateUntilCondition::normal_kernel
                       < MainIterationHolder,
                       ClusterInfoArr *,
                       const CellInfoArr *,
                       bool >,
                       (void *) sumCellsForCentroidKernel,
                       (void *) calculateCentroidsKernel,
                       (void *) assignFinalCellsKernel
                     };

  int blocksizes[] = { FillNeighboursBlockSize,
                       FillNeighboursBlockSize,
                       CountInferiorNeighsBlockSize,
                       FindLocalMaximaBlockSize,
                       ExcludeMaximaPropagationBlockSize,
                       ExcludeMaximaPropagationBlockSize,
                       ClusterSplittingMainPropagationBlockSize,
                       ClusterSplittingMainPropagationBlockSize,
                       SumCellsBlockSize,
                       CalculateCentroidsBlockSize,
                       FinalCellsBlockSize
                     };

  int  gridsizes[] = { Helpers::int_ceil_div(NCaloCells, FillNeighboursBlockSize),
                       Helpers::int_ceil_div(NCaloCells, FillNeighboursBlockSize),
                       Helpers::int_ceil_div(NExactPairs, CountInferiorNeighsBlockSize),
                       Helpers::int_ceil_div(NCaloCells, FindLocalMaximaBlockSize),
                       IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch,
                       IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch,
                       IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch,
                       IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch,
                       Helpers::int_ceil_div(NCaloCells, SumCellsBlockSize),
                       Helpers::int_ceil_div(NMaxClusters, CalculateCentroidsBlockSize),
                       Helpers::int_ceil_div(NCaloCells, FinalCellsBlockSize)
                     };

  int   maxsizes[] = { NCaloCells,
                       NCaloCells,
                       NExactPairs,
                       NCaloCells,
                       std::max(NExactPairs, NCaloCells),
                       std::max(NExactPairs, NCaloCells),
                       std::max(NExactPairs, NCaloCells),
                       std::max(NExactPairs, NCaloCells),
                       NCaloCells,
                       NMaxClusters,
                       NCaloCells
                     };

  optimizer.register_kernels("TopoAutomatonSplitting", 11, kernels, blocksizes, gridsizes, maxsizes);

}
