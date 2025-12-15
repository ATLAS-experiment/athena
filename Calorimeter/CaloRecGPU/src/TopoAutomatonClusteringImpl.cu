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
#include "TopoAutomatonClusteringImpl.h"

#ifndef CALORECGPU_USE_INDIVIDUAL_TEMPORARY_ARRAYS

#define CALORECGPU_TEMP_STRUCT_TO_USE TACTemps

#endif

#include "TemporaryHelpers.h"

#include "CaloIdentifier/LArNeighbours.h"
//It's just a struct.

#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <cstddef>
#include <stdexcept>

#include "FPHelpers.h"

#include "IterateUntilCondition.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

using namespace CaloRecGPU;
using namespace TAGrowing;

void TAGrowing::TACOptionsHolder::sendToGPU(const bool clear_CPU)
{
  m_options_dev = m_options;
  if (clear_CPU)
    {
      m_options.clear();
    }
}

namespace TACTemporaries
{
  struct TACTemps;
  
  CALORECGPU_TEMPWRAPPER(secondary_array, secondary_tag_array);
  
  CALORECGPU_TEMPWRAPPER(merge_table,      tertiary_tag_array);

  CALORECGPU_TEMPBIGARR_3(pairs_1, energyPerSample, maxEPerSample, maxPhiPerSample, int);
  CALORECGPU_TEMPBIGARR_3(pairs_2, maxEtaPerSample, etaPerSample, phiPerSample, int);
  
  constexpr static int start_terminal_pairs = NExactPairs;
  
  CALORECGPU_TEMPCELLARR_1(seed_to_cluster_table, etaCaloFrame, phiCaloFrame, eta1CaloFrame, int);
  
  CALORECGPU_TEMPVAR(num_seedgrow_pairs, firstPhi, 0, int);
  CALORECGPU_TEMPVAR(num_term_pairs,     firstEta, 0, int);

  CALORECGPU_TEMPVAR(continue_flag,      secondR, 0, int);
  CALORECGPU_TEMPVAR(stop_flag,     secondLambda, 0, int);

  CALORECGPU_TEMPVAR(iterate_storage, deltaPhi, 0, IterateUntilCondition::Storage);
  
  struct TACTemps
  {
    int continue_flag;
    int stop_flag;
    
    int num_seedgrow_pairs;
    int num_term_pairs;
    
    int seed_to_cluster_table[NCaloCells];
    
    int pairs_1[2 * NExactPairs];
    int pairs_2[2 * NExactPairs];

    IterateUntilCondition::Storage iterate_storage;
  };
}

constexpr static int SignalToNoiseBlockSize = 512;

constexpr static int CellPairsBlockSize = 512;

constexpr static int ClusterGrowingMainPropagationBlockSize = 1024;

constexpr static int CreateClustersBlockSize = 512;
constexpr static int FinalizeTagsBlockSize = 512;

/******************************************************************************
 * Kernel to calculate the signal-to-noise ratio of cell energy deposition,
 * classify seed, growing, terminal cells and create the clusters for the seeds.
 ******************************************************************************/

static __global__
void signalToNoiseKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                         const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                         const Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                         const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                         const Helpers::CUDA_kernel_object<TopoAutomatonOptions> opts,
                         const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  const int num_cells = cell_info_arr->number;

  for (int cell = index; cell < num_cells; cell += grid_size)
    {
      const int hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);

      const int cell_sampling = (hash_ID >= 0 ? geometry->sampling(hash_ID) : 0);

      if (hash_ID < 0 || !cell_info_arr->is_valid(cell, assume_complete_cells) || !opts->uses_calorimeter_by_sampling(cell_sampling))
        {
          clusters_arr->cells.tags[cell] = TACTag::make_invalid_tag();
          TACTemporaries::secondary_array(clusters_arr, cell) = TACTag::make_invalid_tag();
          continue;
        }

      const float cellEnergy = cell_info_arr->energy[cell];

      float sigNoiseRatio = 0.00001f;
      //It's what's done in the CPU implementation...
      if (!cell_info_arr->is_bad(cell, opts->treat_L1_predicted_as_good, assume_complete_cells))
        {
          const int gain = cell_info_arr->gain[cell];

          float cellNoise = 0.f;

          if (opts->use_two_gaussian && geometry->is_tile(hash_ID))
            {
              cellNoise = noise_arr->get_double_gaussian_noise(hash_ID, gain, cellEnergy);
            }
          else
            {
              cellNoise = noise_arr->get_noise(hash_ID, gain);
            }

          if (isfinite(cellNoise) && cellNoise > 0.0f)
            {
              sigNoiseRatio = cellEnergy / cellNoise;
            }
        }

      const float absRatio = fabsf(sigNoiseRatio);

      bool can_be_seed = (opts->abs_seed ? absRatio : sigNoiseRatio) > opts->seed_threshold;
      bool can_be_grow = (opts->abs_grow ? absRatio : sigNoiseRatio) > opts->grow_threshold;
      bool can_be_term = (opts->abs_terminal ? absRatio : sigNoiseRatio) > opts->terminal_threshold;

      if (can_be_seed && opts->use_time_cut && (!opts->keep_significant_cells || sigNoiseRatio <= opts->snr_threshold_for_keeping_cells))
        {
          if (!cell_info_arr->passes_time_cut(*geometry, cell, opts->time_threshold, opts->use_crosstalk, opts->crosstalk_delta, assume_complete_cells))
            {
              can_be_seed = false;
              if (opts->completely_exclude_cut_seeds)
                {
                  can_be_grow = false;
                  can_be_term = false;
                }
            }
        }

      if (can_be_seed && opts->uses_seed_sampling(cell_sampling))
        {
          const unsigned int SNR_pattern = __float_as_uint(opts->abs_seed ? absRatio : sigNoiseRatio);
          //In principle, we would expect
          //the seed threshold to always be positive,
          //so we could use absRatio by default.
          //However, since we can support
          //also the most general case
          //with total-ordered floating points,
          //why not do it?

          const unsigned int ordered_SNR_pattern = FloatingPointHelpers::StandardFloat::template to_total_ordering<uint32_t>(SNR_pattern);

          const TACTag tag = TACTag::make_seed_tag(hash_ID, ordered_SNR_pattern, can_be_grow);
          //As per the CPU algorithm,
          //if a cell does not pass the grow threshold
          //(which can happen if seeds are being evaluated
          // as absolute value while growing cells are not),
          //the clusters cannot be merged. Somehow.

          clusters_arr->cells.tags[cell] = tag;
          TACTemporaries::secondary_array(clusters_arr, cell) = tag;

          TACTemporaries::merge_table(clusters_arr, hash_ID) = tag.clear_no_merge_flag();
          //The assignment of the seed cell is independent of the
          //"no merging through non-growing seeds" edge case

        }
      else if (can_be_grow)
        {
          clusters_arr->cells.tags[cell] = TACTag::make_grow_tag();
          TACTemporaries::secondary_array(clusters_arr, cell) = TACTag::make_grow_tag();
        }
      else if (can_be_term)
        {
          clusters_arr->cells.tags[cell] = TACTag::make_terminal_tag();
          TACTemporaries::secondary_array(clusters_arr, cell) = TACTag::make_terminal_tag();
        }
      else //is invalid for propagation
        {
          clusters_arr->cells.tags[cell] = TACTag::make_invalid_tag();
          TACTemporaries::secondary_array(clusters_arr, cell) = TACTag::make_invalid_tag();
        }
    }

  if (index == 0)
    {
      clusters_arr->state = ClusterInformationState::None;
      clusters_arr->has_deleted_clusters = 0;
      clusters_arr->number = 0;
      clusters_arr->number_cells = num_cells;
      TACTemporaries::num_seedgrow_pairs(clusters_arr) = 0;
      TACTemporaries::num_term_pairs(clusters_arr) = 0;
    }
}


void TAGrowing::signalToNoise(EventDataHolder & holder,
                              const ConstantDataHolder & instance_data,
                              const TACOptionsHolder & options,
                              const IGPUKernelSizeOptimizer & optimizer,
                              const bool synchronize,
                              CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration config = optimizer.get_launch_configuration("TopoAutomatonGrowing", 0);

  signalToNoiseKernel <<< config.grid_x, config.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                             holder.m_cell_info_dev,
                                                                             instance_data.m_cell_noise_dev,
                                                                             instance_data.m_geometry_dev,
                                                                             options.m_options_dev,
                                                                             holder.m_cell_info->complete);
  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}


/******************************************************************************
 * Kernel to generate the cell pairs for the growing algorithm.
 ******************************************************************************/

static __global__
void cellPairsKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                     const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                     const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                     const Helpers::CUDA_kernel_object<TopoAutomatonOptions> opts,
                     const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  const int num_cells = cell_info_arr->number;

  constexpr int WarpSize = 32;
 
  for (int cell = index; (cell/WarpSize) * WarpSize < num_cells; cell += grid_size)
    {
      
      int neighbours[NMaxNeighbours];

      int num_seedgrow_neighs = 0, num_term_neighs = 0, num_total_neighs = 0;


      constexpr unsigned int grow_seed_neighbour_mark = 0x100000;
      constexpr unsigned int term_neighbour_mark      = 0x200000;
      //Mark growing or terminal neighbours.
      constexpr unsigned int clear_flags_mask = ~(grow_seed_neighbour_mark | term_neighbour_mark);
          
      if (cell < num_cells)
        {
          const TACTag this_tag = clusters_arr->cells.tags[cell];

          if (this_tag.is_grow_or_seed())
            {
              const int hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);

              const bool is_limited = ( opts->limit_HECIW_and_FCal_neighs && geometry->is_HECIW_or_FCal(hash_ID) ) ||
                ( opts->limit_PS_neighs             && geometry->is_PS(hash_ID)            );

              const unsigned int limited_flags  = LArNeighbours::neighbourOption::nextInSamp & opts->neighbour_options;

              const unsigned int neighbour_option = (is_limited ? limited_flags : opts->neighbour_options);

              num_total_neighs = geometry->get_neighbours(neighbour_option, hash_ID, neighbours);

              for (int i = 0; i < num_total_neighs; ++i)
                {
                  const int neigh_hash_ID = neighbours[i];
                  const int neigh_cell = cell_info_arr->get_cell_with_hash_ID(neigh_hash_ID, assume_complete_cells);
                  if (neigh_cell >= 0)
                    {
                      neighbours[i] = neigh_cell;
                      const TACTag neigh_tag = clusters_arr->cells.tags[neigh_cell];
                      if (neigh_tag.is_grow_or_seed())
                        {
                          neighbours[i] |= grow_seed_neighbour_mark;
                          ++num_seedgrow_neighs;
                        }
                      else if (neigh_tag.is_non_assigned_terminal())
                        {
                          neighbours[i] |= term_neighbour_mark;
                          ++num_term_neighs;
                        }
                    }
                  else
                    {
                      neighbours[i] = -1;
                    }
                }
            }
        }
      
      constexpr unsigned int full_mask = 0xFFFFFFFFU;

      int seedgrow_prefix = num_seedgrow_neighs;
      int term_prefix = num_term_neighs;

      const int intra_warp_index = threadIdx.x % WarpSize;

      for (int i = 1; i < WarpSize; i *= 2)
        {
          const int other_seedgrow = __shfl_down_sync(full_mask, seedgrow_prefix, i) * (intra_warp_index + i < WarpSize);
          const int other_term     = __shfl_up_sync  (full_mask, term_prefix,     i) * (intra_warp_index >= i);

          seedgrow_prefix += other_seedgrow;
          term_prefix     += other_term;
        }
      
      int base_seedgrow_index = 0;
      int base_term_index = 0;

      switch (intra_warp_index)
        {
          case 0:
            base_seedgrow_index = atomicAdd(TACTemporaries::num_seedgrow_pairs_ptr(clusters_arr), seedgrow_prefix);
            break;
          case WarpSize - 1:
            base_term_index     = atomicAdd(TACTemporaries::num_term_pairs_ptr(clusters_arr), term_prefix);
            break;
          default:
            break;
        }

      const int seedgrow_offset = __shfl_sync(full_mask, base_seedgrow_index,            0) + seedgrow_prefix - num_seedgrow_neighs;
      const int term_offset     = __shfl_sync(full_mask, base_term_index,     WarpSize - 1) + term_prefix     - num_term_neighs;

      int seedgrow_pair_index   = seedgrow_offset;
      int term_pair_index       = TACTemporaries::start_terminal_pairs + term_offset;

      for (int i = 0; i < num_total_neighs; ++i)
        {
          const int neigh = neighbours[i];
          if (neigh < 0)
            {
              continue;
            }
          else if (neigh & grow_seed_neighbour_mark)
            {
              TACTemporaries::pairs_1(clusters_arr, seedgrow_pair_index) = neigh & clear_flags_mask;
              TACTemporaries::pairs_2(clusters_arr, seedgrow_pair_index) = cell;
              ++seedgrow_pair_index;
            }
          else if (neigh & term_neighbour_mark)
            {
              TACTemporaries::pairs_1(clusters_arr, term_pair_index) = neigh & clear_flags_mask;
              TACTemporaries::pairs_2(clusters_arr, term_pair_index) = cell;
              ++term_pair_index;
            }
        }
    }

  if (index == 0)
    {
      TACTemporaries::stop_flag(clusters_arr) = 0;
      TACTemporaries::continue_flag(clusters_arr) = 0;
    }
}

void TAGrowing::cellPairs(EventDataHolder & holder,
                          const ConstantDataHolder & instance_data,
                          const TACOptionsHolder & options,
                          const IGPUKernelSizeOptimizer & optimizer,
                          const bool synchronize,
                          CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration config = optimizer.get_launch_configuration("TopoAutomatonGrowing", 1);

  cellPairsKernel <<< config.grid_x, config.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
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



/******************************************************************************
 * Series of kernels for the growing algorithm!
 ******************************************************************************/

static __device__
void propagate_through_pair_main(const int pair,
                                 ClusterInfoArr * clusters_arr)
{
  const int this_cell = TACTemporaries::pairs_1(clusters_arr, pair);
  const int neigh_cell = TACTemporaries::pairs_2(clusters_arr, pair);

  const TACTag neigh_tag = clusters_arr->cells.tags[neigh_cell];

  const TACTag prop_tag = neigh_tag.propagate();

  const TACTag this_old_tag = clusters_arr->cells.tags[this_cell];

  if (this_old_tag.is_part_of_cluster() && neigh_tag.is_part_of_cluster() && this_old_tag.can_merge())
    //If the cell was already part of a cluster,
    //we must merge the two of them.
    //Else, we keep growing.
    {
      const int this_seed_idx = this_old_tag.index();
      const int neigh_seed_idx = neigh_tag.index();
      if (this_seed_idx != neigh_seed_idx)
        {
          const tag_type this_merge = TACTemporaries::merge_table(clusters_arr, this_seed_idx);
          const tag_type neigh_merge = TACTemporaries::merge_table(clusters_arr, neigh_seed_idx);

          if (this_merge != neigh_merge)
            {
              if (this_merge > neigh_merge)
                {
                  atomicMax(&(TACTemporaries::merge_table(clusters_arr, neigh_seed_idx)), this_merge);
                }
              else /*if (this_merge < neigh_merge)*/
                {
                  atomicMax(&(TACTemporaries::merge_table(clusters_arr, this_seed_idx)), neigh_merge);
                }
              TACTemporaries::continue_flag(clusters_arr) = 1;
            }
        }
    }
  else if (!this_old_tag.is_part_of_cluster() && neigh_tag.is_part_of_cluster())
    {
      TACTemporaries::continue_flag(clusters_arr) = 1;
      atomicMax(&(TACTemporaries::secondary_array(clusters_arr, this_cell)), prop_tag);
    }
}

static __device__
void propagate_through_pair_terminal(const int pair,
                                     ClusterInfoArr * clusters_arr)
{
  const int this_cell = TACTemporaries::pairs_1(clusters_arr, pair);
  const int neigh_cell = TACTemporaries::pairs_2(clusters_arr, pair);

  const TACTag neigh_tag = clusters_arr->cells.tags[neigh_cell];

  if (neigh_tag.is_part_of_cluster())
    {
      atomicMax(&(TACTemporaries::secondary_array(clusters_arr, this_cell)), neigh_tag.propagate());
    }
}

namespace
{
  struct MainPropagationCondition
  {
    int num_pairs_main;
    int num_pairs_term;
    int cells_number;

#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
    int counter;
#endif

    __device__ bool operator() (const unsigned int /*grid_dim*/,
                                const unsigned int /*grid_index*/,
                                ClusterInfoArr * clusters_arr) const
    {
      return TACTemporaries::stop_flag(clusters_arr);
    }
  };

  struct BeforePropagation
  {
    __device__ void operator() (const unsigned int /*grid_dim*/,
                                const unsigned int /*grid_index*/,
                                MainPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr) const
    {
      condition.num_pairs_main = TACTemporaries::num_seedgrow_pairs(clusters_arr);
      condition.num_pairs_term = TACTemporaries::num_term_pairs(clusters_arr);
      condition.cells_number = clusters_arr->number_cells;
#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      condition.counter = 0;
#endif
    }
  };

  struct MainPropagation
  {
    __device__ void operator() (const unsigned int grid_dim,
                                const unsigned int grid_index,
                                MainPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr) const
    {
      const int index = grid_index * blockDim.x + threadIdx.x;
      const int grid_size = grid_dim * blockDim.x;

      for (int pair = index; pair < condition.num_pairs_main; pair += grid_size)
        {
          propagate_through_pair_main(pair, clusters_arr);
        }
    }
  };

  struct CopyBack
  {
    __device__ void operator() (const unsigned int grid_dim,
                                const unsigned int grid_index,
                                MainPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr) const
    {
      const int index = grid_index * blockDim.x + threadIdx.x;
      const int grid_size = grid_dim * blockDim.x;

      for (int cell = index; cell < condition.cells_number; cell += grid_size)
        {
          clusters_arr->cells.tags[cell] = TACTemporaries::secondary_array(clusters_arr, cell);
        }

      if (index == 0)
        {
          if (!TACTemporaries::continue_flag(clusters_arr))
            {
              TACTemporaries::stop_flag(clusters_arr) = 1;
            }
          else
            {
              TACTemporaries::continue_flag(clusters_arr) = 0;
            }
        }

#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      ++condition.counter;
#endif
    }
  };

  struct AfterPropagation
  {
    __device__ void operator() (const unsigned int grid_dim,
                                const unsigned int grid_index,
                                MainPropagationCondition & condition,
                                ClusterInfoArr * clusters_arr) const
    {
      const int index = grid_index * blockDim.x + threadIdx.x;
      const int grid_size = grid_dim * blockDim.x;

#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      if (index == 0)
        {
          printf("GROWING: %16d\n", condition.counter);
        }
#endif

      for (int pair = index; pair < condition.num_pairs_term; pair += grid_size)
        {
          propagate_through_pair_terminal(TACTemporaries::start_terminal_pairs + pair, clusters_arr);
        }
    }
  };
}

static __global__
void createClustersKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                          const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  const int num_cells = clusters_arr->number_cells;

  for (int cell = index; cell < num_cells; cell += grid_size)
    {
      TACTag tag = TACTemporaries::secondary_array(clusters_arr, cell);

      if (tag.is_part_of_cluster())
        {
          const int this_index = tag.index();

          const TACTag after_merge_tag = TACTemporaries::merge_table(clusters_arr, this_index);
          const int after_merge_index = after_merge_tag.index();

          if (after_merge_index == cell_info_arr->get_hash_ID(cell, assume_complete_cells))
            //If the hash ID matches the after-merge index,
            //this is the seed cell of the cluster.
            {
              const int cluster_index = atomicAdd(&clusters_arr->number, 1);

              assert(cluster_index < NMaxClusters);

              TACTemporaries::seed_to_cluster_table(clusters_arr, after_merge_index) = cluster_index;

              clusters_arr->seedCellIndex[cluster_index] = cell;
            }
          else
            {
              tag = tag.set_index(after_merge_index);
            }
        }

      clusters_arr->cells.tags[cell] = tag;
    }
}

static __global__
void finalizeTagsKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  const int num_cells = clusters_arr->number_cells;

  for (int cell = index; cell < num_cells; cell += grid_size)
    {
      const TACTag old_tag = clusters_arr->cells.tags[cell];

      if (old_tag.is_part_of_cluster())
        {
          clusters_arr->cells.tags[cell] = ClusterTag::make_tag(TACTemporaries::seed_to_cluster_table(clusters_arr, old_tag.index()));
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

      assert(clusters_arr->number <= NMaxClusters);

#if CALORECGPU_INCLUDE_ITERATION_COUNTERS
      printf("GROWING Final Clusters: %d\n", clusters_arr->number);
#endif
    }
}

using ClusterGrowingIterationsHolder = IterateUntilCondition::Holder<MainPropagationCondition,
                                                                     BeforePropagation,
                                                                     AfterPropagation,
                                                                     MainPropagation,
                                                                     CopyBack>;

void TAGrowing::clusterGrowing(EventDataHolder & holder,
                               const ConstantDataHolder & instance_data,
                               const TACOptionsHolder & options,
                               const IGPUKernelSizeOptimizer & optimizer,
                               const bool synchronize,
                               CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{

  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration cfg_iter = (optimizer.can_use_cooperative_groups() ?
                                                  optimizer.get_launch_configuration("TopoAutomatonGrowing", 2) :
                                                  optimizer.get_launch_configuration("TopoAutomatonGrowing", 3));
  const CUDAKernelLaunchConfiguration cfg_clusters = optimizer.get_launch_configuration("TopoAutomatonGrowing", 4);
  const CUDAKernelLaunchConfiguration cfg_finalize = optimizer.get_launch_configuration("TopoAutomatonGrowing", 5);

  ClusterGrowingIterationsHolder iterations;

  iterations.execute(optimizer.can_use_cooperative_groups(), cfg_iter.grid_x, cfg_iter.block_x, 0, stream_to_use,
                     TACTemporaries::iterate_storage_ptr(holder.m_clusters_dev), static_cast<ClusterInfoArr *>(holder.m_clusters_dev));


  createClustersKernel <<< cfg_clusters.grid_x, cfg_clusters.block_x, 0, stream_to_use >>>(holder.m_clusters_dev, holder.m_cell_info_dev, holder.m_cell_info->complete);

  finalizeTagsKernel <<< cfg_finalize.grid_x, cfg_finalize.block_x, 0, stream_to_use>>>(holder.m_clusters_dev);

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

void TAGrowing::register_kernels(IGPUKernelSizeOptimizer & optimizer)
{
  void * kernels[] = { (void *) signalToNoiseKernel,
                       (void *) cellPairsKernel,
                       (void *) IterateUntilCondition::cooperative_kernel
                       < ClusterGrowingIterationsHolder,
                       ClusterInfoArr * >,
                       (void *) IterateUntilCondition::normal_kernel
                       < ClusterGrowingIterationsHolder,
                       ClusterInfoArr * >,
                       (void *) createClustersKernel,
                       (void *) finalizeTagsKernel
                     };

  int blocksizes[] = { SignalToNoiseBlockSize,
                       CellPairsBlockSize,
                       ClusterGrowingMainPropagationBlockSize,
                       ClusterGrowingMainPropagationBlockSize,
                       CreateClustersBlockSize,
                       FinalizeTagsBlockSize
                     };

  int  gridsizes[] = { Helpers::int_ceil_div(NCaloCells, SignalToNoiseBlockSize),
                       Helpers::int_ceil_div(NCaloCells, CellPairsBlockSize),
                       IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch,
                       IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch,
                       Helpers::int_ceil_div(NCaloCells, CreateClustersBlockSize),
                       Helpers::int_ceil_div(NCaloCells, FinalizeTagsBlockSize)
                     };

  int   maxsizes[] = { NCaloCells,
                       NCaloCells,
                       NExactPairs,
                       NExactPairs,
                       NCaloCells,
                       NCaloCells
                     };

  optimizer.register_kernels("TopoAutomatonGrowing", 6, kernels, blocksizes, gridsizes, maxsizes);
}
