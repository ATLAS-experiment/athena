//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "TopoAutomatonClusteringImpl.h"

#include "CaloIdentifier/LArNeighbours.h"
//It's just a struct.

#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <cstddef>
#include <stdexcept>

#include "FPHelpers.h"


#include <cooperative_groups.h>

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

using namespace CaloRecGPU;
using namespace TAGrowing;

void TAGrowing::TACOptionsHolder::allocate()
{
  m_options.allocate();
  m_options_dev.allocate();
}

void TAGrowing::TACOptionsHolder::sendToGPU(const bool clear_CPU)
{
  m_options_dev = m_options;
  if (clear_CPU)
    {
      m_options.clear();
    }
}

namespace TACHacks
{
  TopoAutomatonGrowingTemporaries * get_temporaries(EventDataHolder & edh)
  {
    return (TopoAutomatonGrowingTemporaries *) ((void *) ((ClusterMomentsArr *) edh.m_moments_dev));
  }
}

constexpr static int SignalToNoiseBlockSize = 512;

constexpr static int CellPairsBlockSize = 256;

constexpr static int ClusterGrowingMainPropagationBlockSize = 1024;

constexpr static int ClusterGrowingPropagationBlockSize = 256;
constexpr static int ClusterGrowingCopyAndCheckBlockSize = 512;
constexpr static int ClusterGrowingTerminalPropagationBlockSize = 256;

constexpr static int ClusterGrowingSeedCellAssignmentBlockSize = 256;
constexpr static int ClusterGrowingFinalizationBlockSize = 512;


/******************************************************************************
 * Kernel to calculate the signal-to-noise ratio of cell energy deposition,
 * classify seed, growing, terminal cells and create the clusters for the seeds.
 ******************************************************************************/

static __global__
void signalToNoiseKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                         Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                         Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries,
                         const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                         const Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                         const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                         const Helpers::CUDA_kernel_object<TopoAutomatonOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const int cell_sampling = geometry->sampling(cell);
      const float cellEnergy = cell_info_arr->energy[cell];

      if (!cell_info_arr->is_valid(cell) || !opts->uses_calorimeter_by_sampling(cell_sampling))
        {
          cell_state_arr->clusterTag[cell] = TACTag::make_invalid_tag();
          temporaries->secondary_array[cell] = TACTag::make_invalid_tag();
          continue;
        }

      float sigNoiseRatio = 0.00001f;
      //It's what's done in the CPU implementation...
      if (!cell_info_arr->is_bad(cell, opts->treat_L1_predicted_as_good))
        {
          const int gain = cell_info_arr->gain[cell];

          float cellNoise = 0.f;

          if (opts->use_two_gaussian && geometry->is_tile(cell))
            {
              cellNoise = noise_arr->get_double_gaussian_noise(cell, gain, cellEnergy);
            }
          else
            {
              cellNoise = noise_arr->get_noise(cell, gain);
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
          if (!cell_info_arr->passes_time_cut(*geometry, cell, opts->time_threshold, opts->use_crosstalk, opts->crosstalk_delta))
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
          const int n = atomicAdd(&(clusters_arr->number), 1);

          const unsigned int SNR_pattern = __float_as_int(opts->abs_seed ? absRatio : sigNoiseRatio);
          //In principle, we would expect
          //the seed threshold to always be positive,
          //so we could use absRatio by default.
          //However, since we can support
          //also the most general case
          //with total-ordered floating points,
          //why not do it?

          const unsigned int ordered_SNR_pattern = FloatingPointHelpers::StandardFloat::template to_total_ordering<uint32_t>(SNR_pattern);

          const TACTag tag = TACTag::make_seed_tag(cell, ordered_SNR_pattern, can_be_grow);
          //As per the CPU algorithm,
          //if a cell does not pass the grow threshold
          //(which can happen if seeds are being evaluated
          // as absolute value while growing cells are not),
          //the clusters cannot be merged. Somehow.

          cell_state_arr->clusterTag[cell] = tag;
          temporaries->secondary_array[cell] = tag;

          temporaries->cell_to_cluster_map[cell] = n;

          unsigned long long int snr_and_cell = ordered_SNR_pattern;

          snr_and_cell = (snr_and_cell << 32) | cell;

          temporaries->seed_cell_table[n] = snr_and_cell;
        }
      else if (can_be_grow)
        {
          cell_state_arr->clusterTag[cell] = TACTag::make_grow_tag();
          temporaries->secondary_array[cell] = TACTag::make_grow_tag();
        }
      else if (can_be_term)
        {
          cell_state_arr->clusterTag[cell] = TACTag::make_terminal_tag();
          temporaries->secondary_array[cell] = TACTag::make_terminal_tag();
        }
      else //is invalid for propagation
        {
          cell_state_arr->clusterTag[cell] = TACTag::make_invalid_tag();
          temporaries->secondary_array[cell] = TACTag::make_invalid_tag();
        }
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
  cudaMemsetAsync(&(holder.m_clusters_dev->number), 0, sizeof(holder.m_clusters_dev->number), stream_to_use);

  const CUDAKernelLaunchConfiguration config = optimizer.get_launch_configuration("TopoAutomatonGrowing", 0);

  signalToNoiseKernel <<< config.grid_x, config.block_x, 0, stream_to_use>>>(holder.m_cell_state_dev,
                                                                             holder.m_clusters_dev,
                                                                             TACHacks::get_temporaries(holder),
                                                                             holder.m_cell_info_dev,
                                                                             instance_data.m_cell_noise_dev,
                                                                             instance_data.m_geometry_dev,
                                                                             options.m_options_dev);
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
void cellPairsKernel(Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries,
                     const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                     const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                     const Helpers::CUDA_kernel_object<TopoAutomatonOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const TACTag this_tag = cell_state_arr->clusterTag[cell];

      int neighbours[NMaxNeighbours];

      int num_seedgrow_neighs = 0, num_term_neighs = 0, num_total_neighs = 0;

      constexpr unsigned int grow_seed_neighbour_mark = 0x100000;
      constexpr unsigned int term_neighbour_mark      = 0x200000;
      //Mark growing or terminal neighbours.
      constexpr unsigned int clear_flags_mask = ~(grow_seed_neighbour_mark | term_neighbour_mark);

      if (this_tag.is_grow_or_seed())
        {
          const bool is_limited = ( opts->limit_HECIW_and_FCal_neighs && geometry->is_HECIW_or_FCal(cell) ) ||
                                  ( opts->limit_PS_neighs             && geometry->is_PS(cell)            );

          const unsigned int limited_flags  = LArNeighbours::neighbourOption::nextInSamp & opts->neighbour_options;

          const unsigned int neighbour_option = (is_limited ? limited_flags : opts->neighbour_options);

          num_total_neighs = geometry->get_neighbours(neighbour_option, cell, neighbours);

          for (int i = 0; i < num_total_neighs; ++i)
            {
              const int neigh_ID = neighbours[i];
              const TACTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];
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
        }

      constexpr int WarpSize = 32;
      constexpr unsigned int full_mask = 0xFFFFFFFFU;

      int seedgrow_prefix = num_seedgrow_neighs;
      int term_prefix = num_term_neighs;

      const int intra_warp_index = threadIdx.x % WarpSize;

      for (int i = 1; i < WarpSize; i *= 2)
        {
          const int other_seedgrow = __shfl_down_sync(full_mask, seedgrow_prefix, i) * (intra_warp_index + i < WarpSize);
          const int other_term     = __shfl_down_sync(full_mask, term_prefix,     i) * (intra_warp_index + i < WarpSize);

          seedgrow_prefix += other_seedgrow;
          term_prefix     += other_term;
        }

      int real_term_prefix = 0;

      if (intra_warp_index <= 1)
        {
          real_term_prefix = __shfl_sync(0x00000003U, term_prefix, 0);
        }

      int base_seedgrow_index = 0;
      int base_term_index = 0;

      switch (intra_warp_index)
        {
          case 0:
            base_seedgrow_index = atomicAdd(&(temporaries->seedgrow_pairs.number), seedgrow_prefix);
            break;
          case 1:
            base_term_index     = atomicAdd(&(temporaries->term_pairs.number), real_term_prefix);
            break;
          default:
            break;
        }

      const int seedgrow_offset = __shfl_sync(full_mask, base_seedgrow_index, 0) + seedgrow_prefix - num_seedgrow_neighs;
      const int term_offset     = __shfl_sync(full_mask, base_term_index,     1) + term_prefix     - num_term_neighs;

      int seedgrow_pair_index   = seedgrow_offset;
      int term_pair_index       = term_offset;

      for (int i = 0; i < num_total_neighs; ++i)
        {
          const int neigh = neighbours[i];
          if (neigh & grow_seed_neighbour_mark)
            {
              temporaries->seedgrow_pairs.cellID[seedgrow_pair_index] = neigh & clear_flags_mask;
              temporaries->seedgrow_pairs.neighbourID[seedgrow_pair_index] = cell;
              ++seedgrow_pair_index;
            }
          else if (neigh & term_neighbour_mark)
            {
              temporaries->term_pairs.cellID[term_pair_index] = neigh & clear_flags_mask;
              temporaries->term_pairs.neighbourID[term_pair_index] = cell;
              ++term_pair_index;
            }
        }
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

  TopoAutomatonGrowingTemporaries * temps = TACHacks::get_temporaries(holder);

  cudaMemsetAsync(&(temps->seedgrow_pairs.number), 0, sizeof(int), stream_to_use);
  cudaMemsetAsync(&(temps->term_pairs.number),     0, sizeof(int), stream_to_use);

  const CUDAKernelLaunchConfiguration config = optimizer.get_launch_configuration("TopoAutomatonGrowing", 1);

  cellPairsKernel <<< config.grid_x, config.block_x, 0, stream_to_use>>>(temps,
                                                                         holder.m_cell_state_dev,
                                                                         instance_data.m_geometry_dev,
                                                                         options.m_options_dev);

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
                                 Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                 Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries)
{
  //Maybe we should go back to the old idea of per-cell iteration
  //since each thread is anyway evaluating multiple things?

  const int this_ID = temporaries->seedgrow_pairs.cellID[pair];
  const int neigh_ID = temporaries->seedgrow_pairs.neighbourID[pair];

  const TACTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];

  const TACTag prop_tag = neigh_tag.propagate();

  const TACTag this_old_tag = cell_state_arr->clusterTag[this_ID];

  if (this_old_tag.is_part_of_cluster() && neigh_tag.is_part_of_cluster() && this_old_tag.can_merge())
    //If the cell was already part of a cluster,
    //we must merge the two of them.
    //Else, we keep growing.
    {
      const int this_seed_idx = this_old_tag.index();
      const int neigh_seed_idx = neigh_tag.index();
      if (this_seed_idx != neigh_seed_idx)
        {
          const int this_cluster_index = temporaries->cell_to_cluster_map[this_seed_idx];
          const int neigh_cluster_index = temporaries->cell_to_cluster_map[neigh_seed_idx];
          if (this_cluster_index != neigh_cluster_index)
            {
              if (this_cluster_index > neigh_cluster_index)
                {
                  atomicMax(&(temporaries->cell_to_cluster_map[neigh_seed_idx]), this_cluster_index);
                  atomicMax(&(temporaries->seed_cell_table[this_cluster_index]),
                            temporaries->seed_cell_table[neigh_cluster_index]    );
                }
              else /* if (neigh_cluster_index > this_cluster_index) */
                {
                  atomicMax(&(temporaries->cell_to_cluster_map[this_seed_idx]), neigh_cluster_index);
                  atomicMax(&(temporaries->seed_cell_table[neigh_cluster_index]),
                            temporaries->seed_cell_table[this_cluster_index]    );
                }
              temporaries->continue_flag = 1;
            }
        }
    }
  else if (!this_old_tag.is_part_of_cluster() && neigh_tag.is_part_of_cluster())
    {
      temporaries->continue_flag = 1;
      atomicMax(&(temporaries->secondary_array[this_ID]), prop_tag);
    }
}


static __device__
void propagate_through_pair_terminal(const int pair,
                                     Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                     Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries)
{
  const int this_ID = temporaries->term_pairs.cellID[pair];
  const int neigh_ID = temporaries->term_pairs.neighbourID[pair];

  const TACTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];

  atomicMax(&(temporaries->secondary_array[this_ID]), neigh_tag.propagate());
}


static __global__
void clusterGrowingMainCooperativeKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                         Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries)
{
  cooperative_groups::grid_group grid = cooperative_groups::this_grid();

  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int num_pairs_main = temporaries->seedgrow_pairs.number;
  const int num_pairs_term = temporaries->term_pairs.number;

  const int grid_size = grid.size();
  //Using the "legacy" version for the time being
  //due to CUDA toolkit version availability...

  //int counter = 0;

  while (!temporaries->stop_flag)
    {
      for (int pair = index; pair < num_pairs_main; pair += grid_size)
        {
          propagate_through_pair_main(pair, cell_state_arr, temporaries);
        }

      grid.sync();

      for (int cell = index; cell < NCaloCells; cell += grid_size)
        {
          cell_state_arr->clusterTag[cell] = temporaries->secondary_array[cell];
        }

      if (index == 0)
        {
          if (!temporaries->continue_flag)
            {
              temporaries->stop_flag = 1;
            }
          else
            {
              temporaries->continue_flag = 0;
            }
        }

      //++counter;

      grid.sync();
    }

  //if (index == 0)
  //  {
  //    printf("GROWING: %16d\n", counter);
  //  }

  for (int pair = index; pair < num_pairs_term; pair += grid_size)
    {
      propagate_through_pair_terminal(pair, cell_state_arr, temporaries);
    }

}

static __global__
void assignSeedCellsKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                           const Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int clusters_number = clusters_arr->number;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cluster = index; cluster < clusters_number; cluster += grid_size)
    {
      const unsigned long long int SNR_and_cell = temporaries->seed_cell_table[cluster];
      const int cell = SNR_and_cell & 0xFFFFFU;
      clusters_arr->seedCellID[cluster] = cell;
    }
}

static __global__
void finalizeClusterAttributionKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                      const Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const TACTag old_tag = temporaries->secondary_array[cell];

      if (old_tag.is_part_of_cluster())
        {
          cell_state_arr->clusterTag[cell] = ClusterTag::make_tag(temporaries->cell_to_cluster_map[old_tag.index()]);
        }
      else
        {
          cell_state_arr->clusterTag[cell] = ClusterTag::make_invalid_tag();
        }
    }
}


namespace
{
  struct kernel_sizes
  {
    int neigh_prop, copy_and_check, term_prop, seed_assign;
  };
}

static __global__
void propagateNeighboursKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                               Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries,
                               Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                               const kernel_sizes blocks, const kernel_sizes grids);

static __global__
void propagateTerminalsKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                              Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries,
                              Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                              const kernel_sizes blocks, const kernel_sizes grids);

static __global__
void copyTagsAndCheckTerminationKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                       Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries,
                                       Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                       const kernel_sizes blocks, const kernel_sizes grids);

static __global__
void propagateNeighboursKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                               Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries,
                               Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                               const kernel_sizes blocks, const kernel_sizes grids)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int num_pairs = temporaries->seedgrow_pairs.number;

  for (int pair = index; pair < num_pairs; pair += grid_size)
    {
      propagate_through_pair_main(pair, cell_state_arr, temporaries);
    }
#if CUDA_CAN_USE_TAIL_LAUNCH
  if (index == grid_size - 1)
    {
      copyTagsAndCheckTerminationKernel <<< grids.copy_and_check, blocks.copy_and_check, 0, cudaStreamTailLaunch>>>(cell_state_arr,
                                                                                                                    temporaries,
                                                                                                                    clusters_arr,
                                                                                                                    blocks, grids);
    }
#endif
}


__global__ static
void copyTagsAndCheckTerminationKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                       Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries,
                                       Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                       const kernel_sizes blocks, const kernel_sizes grids)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      cell_state_arr->clusterTag[cell] = temporaries->secondary_array[cell];
    }

  if (index == grid_size - 1)
    {
#if CUDA_CAN_USE_TAIL_LAUNCH
      if (temporaries->continue_flag)
        {
          temporaries->continue_flag = 0;

          propagateNeighboursKernel <<< grids.neigh_prop, blocks.neigh_prop, 0, cudaStreamTailLaunch>>>(cell_state_arr,
                                                                                                        temporaries,
                                                                                                        clusters_arr,
                                                                                                        blocks, grids);
        }
      else
        {

          propagateTerminalsKernel <<< grids.term_prop, blocks.term_prop, 0, cudaStreamTailLaunch>>>(cell_state_arr,
                                                                                                   temporaries,
                                                                                                   clusters_arr,
                                                                                                   blocks, grids);

        }
#else
      if (!(temporaries->continue_flag))
        {
          temporaries->stop_flag = 1;
        }
      else
        {
          temporaries->continue_flag = 0;
        }
#endif
    }

}

__global__ static
void propagateTerminalsKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                              Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries,
                              Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                              const kernel_sizes blocks, const kernel_sizes grids)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int num_pairs = temporaries->term_pairs.number;
  for (int pair = index; pair < num_pairs; pair += grid_size)
    {
      propagate_through_pair_terminal(pair, cell_state_arr, temporaries);
    }
#if CUDA_CAN_USE_TAIL_LAUNCH
  if (index == grid_size - 1)
    {
      assignSeedCellsKernel <<< grids.seed_assign, blocks.seed_assign, 0, cudaStreamTailLaunch>>>(clusters_arr, temporaries);
    }
#endif
}

__global__ static
void clusterGrowingMainDefer(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                             Helpers::CUDA_kernel_object<TopoAutomatonGrowingTemporaries> temporaries,
                             Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                             const kernel_sizes blocks, kernel_sizes grids)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      if (grids.neigh_prop < 0)
        //We want the minimum grid size.
        {
          grids.neigh_prop = Helpers::int_ceil_div(temporaries->seedgrow_pairs.number, blocks.neigh_prop);
          grids.term_prop = Helpers::int_ceil_div(temporaries->term_pairs.number, blocks.term_prop);
          grids.seed_assign = Helpers::int_ceil_div(clusters_arr->number, blocks.seed_assign);
        }

#if CUDA_CAN_USE_TAIL_LAUNCH

      propagateNeighboursKernel <<< grids.neigh_prop, blocks.neigh_prop, 0, cudaStreamTailLaunch>>>(cell_state_arr,
                                                                                                    temporaries,
                                                                                                    clusters_arr,
                                                                                                    blocks, grids);

#else

      //int counter = 0;

      while (!temporaries->stop_flag)
        {
          propagateNeighboursKernel <<< grids.neigh_prop, blocks.neigh_prop>>>(cell_state_arr,
                                                                               temporaries,
                                                                               clusters_arr,
                                                                               blocks, grids);

          copyTagsAndCheckTerminationKernel <<< grids.copy_and_check, blocks.copy_and_check>>>(cell_state_arr,
                                                                                               temporaries,
                                                                                               clusters_arr,
                                                                                               blocks, grids);

          //++counter;

        }

      //printf("GROWING: %16d\n", counter);

      propagateTerminalsKernel <<< grids.term_prop, blocks.term_prop>>>(cell_state_arr,
                                                                        temporaries,
                                                                        clusters_arr,
                                                                        blocks, grids);
      assignSeedCellsKernel <<< grids.seed_assign, blocks.seed_assign>>>(clusters_arr, temporaries);

#endif
    }
}

void TAGrowing::clusterGrowing(EventDataHolder & holder,
                               const ConstantDataHolder & instance_data,
                               const TACOptionsHolder & options,
                               const IGPUKernelSizeOptimizer & optimizer,
                               const bool synchronize,
                               CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{

  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  TopoAutomatonGrowingTemporaries * temps = TACHacks::get_temporaries(holder);

  cudaMemsetAsync(&(temps->continue_flag), 0, sizeof(int), stream_to_use);
  cudaMemsetAsync(&(temps->stop_flag), 0, sizeof(int), stream_to_use);


  if (optimizer.can_use_cooperative_groups())
    {
      const CUDAKernelLaunchConfiguration cfg_iter = optimizer.get_launch_configuration("TopoAutomatonGrowing", 2);
      const CUDAKernelLaunchConfiguration cfg_seed = optimizer.get_launch_configuration("TopoAutomatonGrowing", 3);

      void * main_args[] = { &holder.m_cell_state_dev, &temps };

      cudaLaunchCooperativeKernel((void *) clusterGrowingMainCooperativeKernel,
                                  cfg_iter.grid_x, cfg_iter.block_x,
                                  main_args, 0, stream_to_use);

      assignSeedCellsKernel <<< cfg_seed.grid_x, cfg_seed.block_x, 0, stream_to_use>>>(holder.m_clusters_dev, temps);

    }
  else if (optimizer.can_use_dynamic_parallelism())
    {
      const CUDAKernelLaunchConfiguration cfg_seed  = optimizer.get_launch_configuration("TopoAutomatonGrowing", 3);
      const CUDAKernelLaunchConfiguration cfg_neigh = optimizer.get_launch_configuration("TopoAutomatonGrowing", 4);
      const CUDAKernelLaunchConfiguration cfg_check = optimizer.get_launch_configuration("TopoAutomatonGrowing", 5);
      const CUDAKernelLaunchConfiguration cfg_term  = optimizer.get_launch_configuration("TopoAutomatonGrowing", 6);

      kernel_sizes grids, blocks;

      if (optimizer.use_minimal_kernel_sizes())
        {
          grids.neigh_prop     = -1;
          grids.term_prop      = -1;
          grids.seed_assign    = -1;
        }
      else
        {
          grids.neigh_prop     = cfg_neigh.grid_x;
          grids.term_prop      = cfg_term.grid_x;
          grids.seed_assign    = cfg_seed.grid_x;
        }

      grids.copy_and_check = cfg_check.grid_x;

      blocks.neigh_prop     = cfg_neigh.block_x;
      blocks.copy_and_check = cfg_check.block_x;
      blocks.term_prop      = cfg_term.block_x;
      blocks.seed_assign    = cfg_seed.block_x;

      clusterGrowingMainDefer <<< 1, 1, 0, stream_to_use>>>(holder.m_cell_state_dev, temps, holder.m_clusters_dev, blocks, grids);
    }
  else
    {
      throw std::runtime_error("The GPU must support either cooperative grid launches "
                               "or dynamic parallelism for the algorithm to work properly.");
      //Well, technically, we could code something up
      //(launch, say, 64 iterations, since the maximum
      // I have seen for ttbar is around ~30
      // and iterations are idempotent after the
      // stopping criterion has been reached),
      //but the added complexity and the fact that
      //architectures without dynamic parallelism
      //either are or soon will be deprecated
      //mean that it's not worth the effort.
    }

  const CUDAKernelLaunchConfiguration cfg_final = optimizer.get_launch_configuration("TopoAutomatonGrowing", 7);

  finalizeClusterAttributionKernel <<< cfg_final.grid_x, cfg_final.block_x, 0, stream_to_use>>>(holder.m_cell_state_dev, temps);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/*******************************************************************************************************************************/

void TAGrowing::register_kernels(IGPUKernelSizeOptimizer & optimizer)
{
  void * kernels[] = { (void *) signalToNoiseKernel,
                       (void *) cellPairsKernel,
                       (void *) clusterGrowingMainCooperativeKernel,
                       (void *) assignSeedCellsKernel,
                       (void *) propagateNeighboursKernel,
                       (void *) copyTagsAndCheckTerminationKernel,
                       (void *) propagateTerminalsKernel,
                       (void *) finalizeClusterAttributionKernel
                     };

  int blocksizes[] = { SignalToNoiseBlockSize,
                       CellPairsBlockSize,
                       ClusterGrowingMainPropagationBlockSize,
                       ClusterGrowingSeedCellAssignmentBlockSize,
                       ClusterGrowingPropagationBlockSize,
                       ClusterGrowingCopyAndCheckBlockSize,
                       ClusterGrowingTerminalPropagationBlockSize,
                       ClusterGrowingFinalizationBlockSize
                     };

  int  gridsizes[] = { Helpers::int_ceil_div(NCaloCells, SignalToNoiseBlockSize),
                       Helpers::int_ceil_div(NCaloCells, CellPairsBlockSize),
                       IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch,
                       Helpers::int_ceil_div(NMaxClusters, ClusterGrowingSeedCellAssignmentBlockSize),
                       Helpers::int_ceil_div(NMaxPairs, ClusterGrowingPropagationBlockSize),
                       Helpers::int_ceil_div(NCaloCells, ClusterGrowingCopyAndCheckBlockSize),
                       Helpers::int_ceil_div(NMaxPairs, ClusterGrowingTerminalPropagationBlockSize),
                       Helpers::int_ceil_div(NCaloCells, ClusterGrowingFinalizationBlockSize)
                     };

  int   maxsizes[] = { NCaloCells,
                       NCaloCells,
                       std::max(NMaxPairs, NCaloCells),
                       NMaxClusters,
                       NMaxPairs,
                       NCaloCells,
                       NMaxPairs,
                       NCaloCells
                     };

  optimizer.register_kernels("TopoAutomatonGrowing", 8, kernels, blocksizes, gridsizes, maxsizes);
}