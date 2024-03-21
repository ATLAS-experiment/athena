//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "TopoAutomatonSplittingImpl.h"

#include "CaloIdentifier/LArNeighbours.h"
//It's just a struct.


#include "CLHEP/Units/SystemOfUnits.h"
//Probably will also work, given that it's just constexpr stuff.

#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>

#include <cooperative_groups.h>


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

constexpr static int FillNeighboursBlockSize = 128;

constexpr static int CountInferiorNeighsBlockSize = 256;
constexpr static int FindLocalMaximaBlockSize = 512;
constexpr static int FixClustersWithoutMaximaBlockSize = 512;

constexpr static int ExcludeMaximaPropagationBlockSize = 512;
constexpr static int ExcludeMaximaOnePropagationBlockSize = 512;
constexpr static int CleanUpSecondariesBlockSize = 512;


constexpr static int ClusterSplittingMainPropagationBlockSize = 1024;
constexpr static int PropagateSplitTagsBlockSize = 256;
constexpr static int HandleSplitTagChangesBlockSize = 256;

constexpr static int SumCellsBlockSize = 320;
constexpr static int CalculateCentroidsBlockSize = 512;
constexpr static int FinalCellsBlockSize = 512;

//These numbers are not at all optimized,
//just going from rough similarity to TAC operations
//(which themselves are not that optimised
// since they were last tested on a previous version...)

namespace TASHacks
{
  TopoAutomatonSplittingTemporaries * get_temporaries(EventDataHolder & edh)
  {
    return (TopoAutomatonSplittingTemporaries *) ((void *) ((ClusterMomentsArr *) edh.m_moments_dev));
  }
}

/******************************************************************************************
 * Determine the same-cluster neighbours of the cells and fill the pairs list accordingly.
 ******************************************************************************************/

namespace
{
  constexpr int LocalMaximaDetection = 0;
}

//Note: we might benefit from using shared memory
//      to hold the neighbours for each thread.
//      See how is register pressure...
//      (Probably not too bad, max. 255 per thread...)

static __global__
void fillNeighboursKernel(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                          const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                          const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                          const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const ClusterTag this_tag = cell_state_arr->clusterTag[cell];

      int neighbours[NMaxNeighbours];
      int num_normal = 0, num_extra = 0, num_next = 0, num_prev = 0, num_limited = 0, total_neighs = 0;

      constexpr int non_neighbour_mark = 0x100000;
      //Will mark neighbours that are not part of the same cluster;

      const bool is_limited = ( opts->limit_HECIW_and_FCal_neighs && geometry->is_HECIW_or_FCal(cell) )  ||
                              ( opts->limit_PS_neighs             && geometry->is_PS(cell)            );
      //The cells that have limited neighbours, for the split cluster growing part.
      //WARNING: the CPU version of the code does not limit PS neighbours ever, but we give additional freedom
      //         (even if it is disabled by default).

      if (this_tag.is_part_of_cluster())
        {
          const unsigned int limited_flags   = LArNeighbours::neighbourOption::nextInSamp & opts->neighbour_options;

          const unsigned int pre_next_flags  = LArNeighbours::neighbourOption::nextSuperCalo & opts->neighbour_options;

          const unsigned int prev_flags      = ( LArNeighbours::neighbourOption::prevSuperCalo |
                                                 LArNeighbours::neighbourOption::prevInSamp      ) & opts->neighbour_options;

          const unsigned int covered_flags   = limited_flags | pre_next_flags | prev_flags;

          const unsigned int remaining_flags = (~covered_flags) & opts->neighbour_options;

          num_limited = geometry->get_neighbours(limited_flags, cell, neighbours);

          num_next = num_limited + geometry->get_neighbours(pre_next_flags, cell, neighbours + num_limited);

          num_prev = geometry->get_neighbours(prev_flags, cell, neighbours + num_next);

          const int num_rest = geometry->get_neighbours(remaining_flags, cell, neighbours + num_next + num_prev);

          total_neighs = num_next + num_prev + num_rest;

          for (int i = 0; i < total_neighs; ++i)
            {
              const int neigh_ID = neighbours[i];
              const ClusterTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];
              if (neigh_tag.is_part_of_cluster() && this_tag.cluster_index() == neigh_tag.cluster_index())
                {
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
                  neighbours[i] |= non_neighbour_mark;
                }
            }
        }
      else
        {
          const unsigned int next_flags = ( LArNeighbours::neighbourOption::nextSuperCalo |
                                            LArNeighbours::neighbourOption::nextInSamp      ) & opts->neighbour_options;

          const unsigned int prev_flags = ( LArNeighbours::neighbourOption::prevSuperCalo |
                                            LArNeighbours::neighbourOption::prevInSamp      ) & opts->neighbour_options;

          num_next = geometry->get_neighbours(next_flags, cell, neighbours);
          num_prev = geometry->get_neighbours(prev_flags, cell, neighbours + num_next);
          total_neighs = num_next + num_prev;

          for (int i = 0; i < total_neighs; ++i)
            {
              neighbours[i] |= non_neighbour_mark;
            }
        }

      constexpr int WarpSize = 32;
      constexpr unsigned int full_mask = 0xFFFFFFFFU;
      const int intra_warp_index = threadIdx.x % WarpSize;

      int normal_prefix = num_normal;
      int next_prefix   = num_next;
      int prev_prefix   = num_prev;
      int extra_prefix  = num_extra;

      for (int i = 1; i < WarpSize; i *= 2)
        {
          const int other_normal = __shfl_down_sync (full_mask, normal_prefix, i) * (intra_warp_index + i < WarpSize);
          const int other_next   = __shfl_down_sync (full_mask, next_prefix,   i) * (intra_warp_index + i < WarpSize);
          const int other_prev   = __shfl_up_sync   (full_mask, prev_prefix,   i) * (intra_warp_index >= i);
          const int other_extra  = __shfl_up_sync   (full_mask, extra_prefix,  i) * (intra_warp_index >= i);

          normal_prefix += other_normal;
          next_prefix   += other_next;
          prev_prefix   += other_prev;
          extra_prefix  += other_extra;
        }


      int real_next_prefix = 0;
      int real_prev_prefix = 0;

      if (intra_warp_index <= 1)
        {
          real_next_prefix = __shfl_sync(0x00000003U, next_prefix, 0);
        }
      else if (intra_warp_index >= WarpSize - 2)
        {
          real_prev_prefix = __shfl_sync(0xC0000000U, prev_prefix, WarpSize - 1);
        }

      int base_normal_offset = 0;
      int base_next_offset   = 0;
      int base_prev_offset   = 0;
      int base_extra_offset  = 0;

      switch (intra_warp_index)
        {
          case 0:
            base_normal_offset = atomicAdd(&(temporaries->pairs.number_normal), normal_prefix);
            break;
          case 1:
            base_next_offset   = atomicAdd(&(temporaries->pairs.number_next),   real_next_prefix);
            break;
          case WarpSize - 2:
            base_prev_offset   = atomicAdd(&(temporaries->pairs.number_prev),   real_prev_prefix);
            break;
          case WarpSize - 1:
            base_extra_offset  = atomicAdd(&(temporaries->pairs.number_extra),  extra_prefix);
            break;
          default:
            break;

        }

      const int normal_offset = __shfl_sync(full_mask, base_normal_offset,            0) + normal_prefix - num_normal;
      const int next_offset   = __shfl_sync(full_mask, base_next_offset,              1) + next_prefix   - num_next;
      const int prev_offset   = __shfl_sync(full_mask, base_prev_offset,   WarpSize - 2) + prev_prefix   - num_prev;
      const int extra_offset  = __shfl_sync(full_mask, base_extra_offset,  WarpSize - 1) + extra_prefix  - num_extra;

      int normal_pair_index   = normal_offset;
      int next_pair_index     = TopoAutomatonSplittingTemporaries::PairsArr::s_intermediate_mark - next_offset - num_next;
      int prev_pair_index     = TopoAutomatonSplittingTemporaries::PairsArr::s_intermediate_mark + prev_offset;
      int extra_pair_index    = TopoAutomatonSplittingTemporaries::PairsArr::s_size - extra_offset - num_extra;

      for (int i = 0; i < num_limited; ++i)
        {
          const int neigh = neighbours[i];
          if (neigh < non_neighbour_mark)
            {
              temporaries->pairs.cellID[normal_pair_index] = neigh;
              temporaries->pairs.neighbourID[normal_pair_index] = cell;
              ++normal_pair_index;
            }
          temporaries->pairs.cellID[next_pair_index] = neigh & (~non_neighbour_mark);
          temporaries->pairs.neighbourID[next_pair_index] = cell;
          ++next_pair_index;
        }
      for (int i = num_limited; i < num_next; ++i)
        {
          const int neigh = neighbours[i];
          if (neigh < non_neighbour_mark)
            {
              int & pair_index = (is_limited ? extra_pair_index : normal_pair_index);
              temporaries->pairs.cellID[pair_index] = neigh;
              temporaries->pairs.neighbourID[pair_index] = cell;
              ++pair_index;
            }
          temporaries->pairs.cellID[next_pair_index] = neigh & (~non_neighbour_mark);
          temporaries->pairs.neighbourID[next_pair_index] = cell;
          ++next_pair_index;
        }
      for (int i = num_next; i < num_next + num_prev; ++i)
        {
          const int neigh = neighbours[i];
          if (neigh < non_neighbour_mark)
            {
              int & pair_index = (is_limited ? extra_pair_index : normal_pair_index);
              temporaries->pairs.cellID[pair_index] = neigh;
              temporaries->pairs.neighbourID[pair_index] = cell;
              ++pair_index;
            }
          temporaries->pairs.cellID[prev_pair_index] = neigh & (~non_neighbour_mark);
          temporaries->pairs.neighbourID[prev_pair_index] = cell;
          ++prev_pair_index;
        }
      for (int i = num_next + num_prev; i < total_neighs; ++i)
        {
          const int neigh = neighbours[i];
          if (neigh < non_neighbour_mark)
            {
              int & pair_index = (is_limited ? extra_pair_index : normal_pair_index);
              temporaries->pairs.cellID[pair_index] = neigh;
              temporaries->pairs.neighbourID[pair_index] = cell;
              ++pair_index;
            }
        }

      if (this_tag.is_part_of_cluster())
        {

          temporaries->get_cells_extra_array<LocalMaximaDetection, int>(cell) = num_normal + num_extra;
        }
      else
        {

          temporaries->get_cells_extra_array<LocalMaximaDetection, int>(cell) = -NMaxNeighbours;
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

  TopoAutomatonSplittingTemporaries * temps = TASHacks::get_temporaries(holder);

  cudaMemsetAsync(&(temps->pairs.number_normal), 0, sizeof(int), stream_to_use);
  cudaMemsetAsync(&(temps->pairs.number_next),   0, sizeof(int), stream_to_use);
  cudaMemsetAsync(&(temps->pairs.number_prev),   0, sizeof(int), stream_to_use);
  cudaMemsetAsync(&(temps->pairs.number_extra),  0, sizeof(int), stream_to_use);

  const CUDAKernelLaunchConfiguration config = optimizer.get_launch_configuration("TopoAutomatonSplitting", 0);

  fillNeighboursKernel <<< config.grid_x, config.block_x, 0, stream_to_use>>>(temps,
                                                                              holder.m_cell_state_dev,
                                                                              instance_data.m_geometry_dev,
                                                                              options.m_options_dev);

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
void countInferiorNeighsKernel(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                               const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                               const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                               const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts)
{
  const int num_normal_pairs = temporaries->pairs.number_normal;
  const int num_extra_pairs = temporaries->pairs.number_extra;
  const int start_extra_pairs = TopoAutomatonSplittingTemporaries::PairsArr::s_size - num_extra_pairs;
  const int num_total_pairs = num_normal_pairs + num_extra_pairs;

  const int thread_index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  for (int pair = thread_index; pair < num_total_pairs; pair += grid_size)
    {
      const int real_pair = ( pair >= num_normal_pairs ?
                              start_extra_pairs + pair - num_normal_pairs : pair);

      const int this_ID = temporaries->pairs.cellID[real_pair];
      const int neigh_ID = temporaries->pairs.neighbourID[real_pair];

      const int this_sampling = geometry->sampling(this_ID);
      const int neigh_sampling = geometry->sampling(neigh_ID);

      float this_energy = 0.f, neigh_energy = 0.f;

      if (!cell_info_arr->is_bad(this_ID, opts->treat_L1_predicted_as_good) && opts->uses_sampling(this_sampling))
        {
          this_energy = cell_info_arr->energy[this_ID];
          if (opts->use_absolute_energy)
            {
              this_energy = fabsf(this_energy);
            }
          else if (this_energy <= 0.f)
            {
              this_energy = 0.f;
            }
        }

      if (!cell_info_arr->is_bad(neigh_ID, opts->treat_L1_predicted_as_good) && opts->uses_sampling(neigh_sampling))
        {
          neigh_energy = cell_info_arr->energy[neigh_ID];
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
          temporaries->get_cells_extra_array<LocalMaximaDetection, int>(neigh_ID) = -NCaloCells;
          //No need to count, just to invalidate!
        }
    }
}

static __global__
void countNeighsDeferrerKernel(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                               const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                               const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                               const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts,
                               const int i_dimBlock)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      const int normal_pairs = temporaries->pairs.number_normal;
      const int extra_pairs  = temporaries->pairs.number_extra;

      const int num_total_pairs = normal_pairs + extra_pairs;

      const int i_dimGrid = Helpers::int_ceil_div(num_total_pairs, i_dimBlock);

#if CUDA_CAN_USE_TAIL_LAUNCH
      countInferiorNeighsKernel <<< i_dimGrid, i_dimBlock, 0, cudaStreamTailLaunch>>>(temporaries,
                                                                                      cell_info_arr,
                                                                                      geometry,
                                                                                      opts);
#else
      countInferiorNeighsKernel <<< i_dimGrid, i_dimBlock>>>(temporaries,
                                                             cell_info_arr,
                                                             geometry,
                                                             opts);
#endif
    }
}

static __global__
void findLocalMaximaKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                           Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                           Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                           const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                           const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                           const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const ClusterTag this_tag = cell_state_arr->clusterTag[cell];

      if (this_tag.is_part_of_cluster())
        {
          const int this_sampling = geometry->sampling(cell);

          float cell_energy = 0.f;
          const float raw_cell_energy = cell_info_arr->energy[cell];

          if (!cell_info_arr->is_bad(cell, opts->treat_L1_predicted_as_good) && opts->uses_sampling(this_sampling))
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

          const int num_neighs = temporaries->get_cells_extra_array<LocalMaximaDetection, int>(cell);

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

          if (is_maximum)
            {
              const int original_cluster = this_tag.cluster_index();

              const int new_cluster = atomicAdd(&(clusters_arr->number), 1);

              const TASTag new_tag = TASTag::make_maximum_tag(new_cluster, __float_as_uint(raw_cell_energy), is_primary);

              cell_state_arr->clusterTag[cell] = new_tag;

              clusters_arr->seedCellID[new_cluster] = cell;

              clusters_arr->seedCellID[original_cluster] = -1;

              temporaries->original_cluster_map[new_cluster] = original_cluster;

            }
          else
            {
              const int this_cluster = this_tag.cluster_index();

              cell_state_arr->clusterTag[cell] = TASTag::make_cluster_cell_tag(this_cluster);
            }
        }
      else
        {
          cell_state_arr->clusterTag[cell] = TASTag::make_invalid_tag();
        }
    }
}

static __global__
void fixClustersWithoutMaximaKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                    Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                    const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = gridDim.x * blockDim.x;

  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      TASTag this_tag = cell_state_arr->clusterTag[cell];

      if (this_tag.is_part_of_splitter_cluster())
        {
          const int cluster_index = this_tag.index();

          if (this_tag.is_secondary())
            {
              temporaries->secondary_array[cell] = this_tag.update_index(cell);
              temporaries->tertiary_array[cell] = this_tag.update_index(cell);
              //So that we get the correct secondary ordering.
            }
          else
            {
              temporaries->secondary_array[cell] = TASTag::secondary_maxima_eliminator();
              temporaries->tertiary_array[cell] = TASTag::secondary_maxima_eliminator();
            }

          temporaries->cell_to_cluster_map[cell] = cluster_index;
        }
      else if (this_tag.is_valid())
        //It'll be part of an original cluster, given how we've assigned the tags.
        {
          const int cluster_index = this_tag.index();

          if (clusters_arr->seedCellID[cluster_index] >= 0)
            //This means the cluster is not split.
            {
              TASTag new_tag = TASTag::make_original_cluster_tag(cluster_index);
              cell_state_arr->clusterTag[cell] = new_tag;

              temporaries->secondary_array[cell] = TASTag::make_invalid_tag();
              temporaries->tertiary_array[cell] = TASTag::make_invalid_tag();
            }
          else
            //This means the tag belongs to a cluster to be split
            {
              temporaries->secondary_array[cell] = TASTag::make_invalid_tag();
              temporaries->tertiary_array[cell]  = TASTag::make_invalid_tag();
            }

          temporaries->cell_to_cluster_map[cell] = cluster_index;
          temporaries->original_cluster_map[cluster_index] = cluster_index;
        }
      else
        {
          temporaries->secondary_array[cell] = TASTag::make_invalid_tag();
          temporaries->tertiary_array[cell]  = TASTag::make_invalid_tag();
          temporaries->cell_to_cluster_map[cell] = -1;
        }
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

  TopoAutomatonSplittingTemporaries * temps = TASHacks::get_temporaries(holder);


  const CUDAKernelLaunchConfiguration cfg_neigh_count = optimizer.get_launch_configuration("TopoAutomatonSplitting", 1);
  const CUDAKernelLaunchConfiguration cfg_find_maxima = optimizer.get_launch_configuration("TopoAutomatonSplitting", 2);
  const CUDAKernelLaunchConfiguration cfg_fix_non_max = optimizer.get_launch_configuration("TopoAutomatonSplitting", 3);

  if (optimizer.use_minimal_kernel_sizes() && optimizer.can_use_dynamic_parallelism())
    {

      countNeighsDeferrerKernel <<< 1, 1, 0, stream_to_use>>>(temps,
                                                              holder.m_cell_info_dev,
                                                              instance_data.m_geometry_dev,
                                                              options.m_options_dev,
                                                              cfg_neigh_count.block_x);
    }
  else
    {
      countInferiorNeighsKernel <<< cfg_neigh_count.grid_x, cfg_neigh_count.block_x, 0, stream_to_use>>>(temps,
                                                                                                         holder.m_cell_info_dev,
                                                                                                         instance_data.m_geometry_dev,
                                                                                                         options.m_options_dev);
    }


  findLocalMaximaKernel <<< cfg_find_maxima.grid_x, cfg_find_maxima.block_x, 0, stream_to_use>>>(holder.m_cell_state_dev,
                                                                                                 holder.m_clusters_dev,
                                                                                                 temps,
                                                                                                 holder.m_cell_info_dev,
                                                                                                 instance_data.m_geometry_dev,
                                                                                                 options.m_options_dev);

  fixClustersWithoutMaximaKernel <<< cfg_fix_non_max.grid_x, cfg_fix_non_max.block_x, 0, stream_to_use>>>(holder.m_cell_state_dev,
                                                                                                          temps,
                                                                                                          holder.m_clusters_dev);

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
                                     Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries)
{
  const int this_ID = temporaries->pairs.cellID[pair];
  const int neigh_ID = temporaries->pairs.neighbourID[pair];

  tag_type * array = ( pair < TopoAutomatonSplittingTemporaries::PairsArr::s_intermediate_mark ?
                       temporaries->secondary_array :
                       temporaries->tertiary_array    );

  const TASTag this_tag = array[this_ID];
  const TASTag neigh_tag = array[neigh_ID];

  if (this_tag.is_secondary_maxima_eliminator() || this_tag.is_secondary_maximum_seed())
    {
      if (atomicMax(&(array[neigh_ID]), this_tag) < this_tag)
        {
          temporaries->continue_flag = 1;
        }
    }
}


static __global__
void secondaryMaximaCooperativeKernel(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries)
{
  cooperative_groups::grid_group grid = cooperative_groups::this_grid();

  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int grid_size = grid.size();
  //Using the "legacy" version for the time being
  //due to CUDA toolkit version availability...

  const int next_number = temporaries->pairs.number_next;
  const int pair_start  = TopoAutomatonSplittingTemporaries::PairsArr::s_intermediate_mark - next_number;
  const int pair_number = temporaries->pairs.number_prev + next_number;

  //int counter = 0;

  while (!temporaries->stop_flag)
    {
      for (int pair = index; pair < pair_number; pair += grid_size)
        {
          const int pair_index = pair_start + pair;
          propagate_secondary_maxima_pair(pair_index, temporaries);
        }

      grid.sync();

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

      grid.sync();

      //++counter;
    }

  //if (index == 0)
  //  {
  //    printf("SECONDARY SPLITTING: %16d\n", counter);
  //  }

}

static __global__
void checkForMaximaExclusionTermination(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                        const int i_dimBlock, const int i_dimGrid);

static __global__
void propagateForMaximaExclusionKernel(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                       const int i_dimBlock, const int i_dimGrid)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  const int next_number = temporaries->pairs.number_next;
  const int pair_start  = TopoAutomatonSplittingTemporaries::PairsArr::s_intermediate_mark - next_number;
  const int pair_number = temporaries->pairs.number_prev + next_number;

  for (int pair = index; pair < pair_number; pair += grid_size)
    {
      const int pair_index = pair_start + pair;

      propagate_secondary_maxima_pair(pair_index, temporaries);

    }
#if CUDA_CAN_USE_TAIL_LAUNCH
  if (index == grid_size - 1)
    {
      checkForMaximaExclusionTermination <<< 1, 1, 0, cudaStreamTailLaunch>>>(temporaries, i_dimBlock, i_dimGrid);
    }
#endif
}

static __global__
void checkForMaximaExclusionTermination(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                        const int i_dimBlock, const int i_dimGrid)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      if (temporaries->continue_flag)
        {
          temporaries->continue_flag = 0;

#if CUDA_CAN_USE_TAIL_LAUNCH
          propagateForMaximaExclusionKernel <<< i_dimGrid, i_dimBlock, 0, cudaStreamTailLaunch>>>(temporaries, i_dimBlock, i_dimGrid);

#endif
        }
#if !CUDA_CAN_USE_TAIL_LAUNCH
      else /*if (!temporaries->continue_flag)*/
        {
          temporaries->stop_flag = 1;
        }
#endif
    }
}

static __global__
void excludeSecondaryMaximaDefer(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                 const int i_dimBlock, int i_dimGrid)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      if (i_dimGrid < 0)
        //We want the minimum grid size.
        {
          const int pair_number = temporaries->pairs.number_prev + temporaries->pairs.number_next;

          i_dimGrid = Helpers::int_ceil_div(pair_number, i_dimBlock);

        }

#if CUDA_CAN_USE_TAIL_LAUNCH
      propagateForMaximaExclusionKernel <<< i_dimGrid, i_dimBlock, 0, cudaStreamTailLaunch>>>(temporaries, i_dimBlock, i_dimGrid);
#else
      //int counter = 0;

      while (!temporaries->stop_flag)
        {
          propagateForMaximaExclusionKernel <<< i_dimGrid, i_dimBlock>>>(temporaries, i_dimBlock, i_dimGrid);
          checkForMaximaExclusionTermination <<< 1, 1>>>(temporaries, i_dimBlock, i_dimGrid);

          //++counter;
        }

      //printf("SECONDARY SPLITTING: %16d\n", counter);
#endif
    }
}

static __global__
void cleanUpSecondariesKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                              Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                              Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const TASTag original_tag  = cell_state_arr->clusterTag[cell];
      const TASTag tag_from_next = temporaries->secondary_array[cell];
      const TASTag tag_from_prev = temporaries->tertiary_array[cell];

      TASTag final_tag = original_tag;

      if (original_tag.is_part_of_splitter_cluster())
        {
          if (original_tag.is_secondary())
            {
              if (tag_from_next.index() == cell && tag_from_prev.index() == cell)
                {
                  final_tag = final_tag.set_primary();
                  final_tag = final_tag.update_index(cell);
                }
              else
                {
                  const int original_index = original_tag.index();
                  const int original_cluster = temporaries->original_cluster_map[original_index];
                  final_tag = TASTag::make_cluster_cell_tag(original_cluster);
                  clusters_arr->seedCellID[original_index] = -1;
                  temporaries->cell_to_cluster_map[cell] = original_cluster;
                }
            }
          else
            {
              final_tag = final_tag.update_index(cell);
            }
        }

      cell_state_arr->clusterTag[cell] = final_tag;
      temporaries->secondary_array[cell] = final_tag;
    }
}

void TASplitting::excludeSecondaryMaxima(EventDataHolder & holder,
                                         const ConstantDataHolder & instance_data,
                                         const TASOptionsHolder & options,
                                         const IGPUKernelSizeOptimizer & optimizer,
                                         const bool synchronize,
                                         CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  TopoAutomatonSplittingTemporaries * temps = TASHacks::get_temporaries(holder);

  if (options.m_options->valid_sampling_secondary != 0)
    {
      cudaMemsetAsync((&temps->continue_flag), 0, sizeof(int), stream_to_use);
      cudaMemsetAsync((&temps->stop_flag), 0, sizeof(int), stream_to_use);

      if (optimizer.can_use_cooperative_groups())
        {

          const CUDAKernelLaunchConfiguration cfg_secprop = optimizer.get_launch_configuration("TopoAutomatonSplitting", 4);
          void * propagate_args[] = { &temps };

          cudaLaunchCooperativeKernel((void *) secondaryMaximaCooperativeKernel,
                                      cfg_secprop.grid_x, cfg_secprop.block_x,
                                      propagate_args, 0, stream_to_use);

        }
      else if (optimizer.can_use_dynamic_parallelism())
        {
          const CUDAKernelLaunchConfiguration cfg_secprop = optimizer.get_launch_configuration("TopoAutomatonSplitting", 5);

          excludeSecondaryMaximaDefer <<< 1, 1, 0, stream_to_use>>>(temps,
                                                                    cfg_secprop.block_x,
                                                                    (optimizer.use_minimal_kernel_sizes() ? -1 : cfg_secprop.grid_x));
        }
      else
        {
          throw std::runtime_error("The GPU must support either cooperative grid launches "
                                   "or dynamic parallelism for the algorithm to work properly.");
          //Well, technically, we could code something up
          //(launch, say, 64 iterations, since the maximum
          // I have seen for ttbar is around ~9
          // and iterations are idempotent after the
          // stopping criterion has been reached),
          //but the added complexity and the fact that
          //architectures without dynamic parallelism
          //either are or soon will be deprecated
          //mean that it's not worth the effort.
        }
    }

  const CUDAKernelLaunchConfiguration cfg_cleanup = optimizer.get_launch_configuration("TopoAutomatonSplitting", 6);

  cleanUpSecondariesKernel <<< cfg_cleanup.grid_x, cfg_cleanup.block_x, 0, stream_to_use>>>(holder.m_cell_state_dev, holder.m_clusters_dev, temps);

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
                         Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                         const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                         const bool counter_select,
                         const bool share_cells)
{
  const int this_ID = temporaries->pairs.cellID[pair];
  const int neigh_ID = temporaries->pairs.neighbourID[pair];

  const TASTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];

  if (!neigh_tag.is_part_of_splitter_cluster())
    {
      return;
    }

  TASTag prop_tag = neigh_tag.propagate();

  const TASTag old_tag = cell_state_arr->clusterTag[this_ID];

  if ( share_cells                           &&
       old_tag.is_part_of_splitter_cluster() &&
       !old_tag.is_shared()                  &&
       !old_tag.is_primary()                 &&
       !neigh_tag.is_shared()                    )
    {
      const int old_cluster = temporaries->cell_to_cluster_map[this_ID];
      const int new_cluster = temporaries->cell_to_cluster_map[neigh_ID];

      const int old_counter = old_tag.counter();
      const int new_counter = prop_tag.counter();

      if (old_counter == new_counter && old_cluster != new_cluster)
        {
          prop_tag = old_tag.prepare_for_sharing(prop_tag);
          atomicMax(&(temporaries->reset_counters[counter_select]), new_counter);
          temporaries->continue_flag = 1;
        }
    }
  else if (neigh_tag.is_shared() && !neigh_tag.is_primary() && neigh_tag.counter() > 0x7FF)
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
                             prop_tag.is_primary()                           ||
                             (!prop_tag.is_shared() && old_tag.is_shared())     ))
    {
      atomicMax(&(temporaries->secondary_array[this_ID]), prop_tag);
      temporaries->continue_flag = 1;
    }
}

static __device__
void update_cell_tag(const int cell,
                     Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                     Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                     const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                     const bool counter_select)
{
  TASTag new_tag = temporaries->secondary_array[cell];

  if (!new_tag.is_part_of_splitter_cluster())
    {
      return;
    }

  const int desired_counter = temporaries->reset_counters[counter_select];

  const TASTag old_tag = cell_state_arr->clusterTag[cell];

  const int cell_from = new_tag.index();
  const int new_cluster = temporaries->cell_to_cluster_map[cell_from];

  if (new_tag.counter() < desired_counter ||
      (old_tag.is_part_of_splitter_cluster() && old_tag.counter() < desired_counter) )
    {
      const int original_cluster = temporaries->original_cluster_map[new_cluster & 0xFFFFU];

      new_tag = TASTag::make_cluster_cell_tag(original_cluster);
      temporaries->cell_to_cluster_map[cell] = original_cluster;

      cell_state_arr->clusterTag[cell] = new_tag;
      temporaries->secondary_array[cell] = new_tag;
      return;
    }

  if (new_tag == old_tag)
    {
      return;
    }


  if ( new_tag.is_shared() &&  new_tag.is_primary() &&
       !old_tag.is_shared() && !old_tag.is_primary()    )
    {
      new_tag = new_tag.update_counter(old_tag.counter());
      const int old_cluster = temporaries->cell_to_cluster_map[cell];
      const unsigned int min_index = min(new_cluster, old_cluster);
      const unsigned int max_index = max(new_cluster, old_cluster);
      temporaries->cell_to_cluster_map[cell] = (max_index << 16) | min_index;
    }
  else
    {
      temporaries->cell_to_cluster_map[cell] = new_cluster;
    }

  const float cell_energy = cell_info_arr->energy[cell];
  new_tag = new_tag.update_cell(cell, __float_as_uint(cell_energy));

  cell_state_arr->clusterTag[cell] = new_tag;
  temporaries->secondary_array[cell] = new_tag;
}

static __global__
void clusterSplittingMainCooperativeKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                           Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                           const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                           const bool share_cells)
{
  cooperative_groups::grid_group grid = cooperative_groups::this_grid();

  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  const int num_pairs = temporaries->pairs.number_normal;

  const int grid_size = grid.size();
  //Using the "legacy" version for the time being
  //due to CUDA toolkit version availability...

  bool counter_select = false;

  //int counter = 0;

  while (!temporaries->stop_flag)
    {
      for (int pair = index; pair < num_pairs; pair += grid_size)
        {
          propagate_main_pair(pair, temporaries, cell_state_arr, counter_select, share_cells);
        }

      grid.sync();

      for (int cell = index; cell < NCaloCells; cell += grid_size)
        {
          update_cell_tag(cell, cell_state_arr, temporaries, cell_info_arr, counter_select);
        }

      if (index == 0)
        {
          if (!temporaries->continue_flag)
            {
              temporaries->stop_flag = 1;
            }
          else
            {
              temporaries->reset_counters[!counter_select] = 0;
              temporaries->continue_flag = 0;
            }
        }

      //++counter;
      
      counter_select = !counter_select;

      grid.sync();

    }

  //if (index == 0)
  //  {
  //    printf("SPLITTING: %16d\n", counter);
  //  }

}

namespace
{
  struct kernel_sizes
  {
    int main_prop, tag_change;
  };
}


static __global__
void handleSplitterTagChangesAndTerminationKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                                  Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                                  const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                                  const bool counter_select,
                                                  const bool share_cells,
                                                  const kernel_sizes blocks,
                                                  const kernel_sizes grids);

static __global__
void propagateSplitterTagsKernel(const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                 Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                 const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                 const bool counter_select,
                                 const bool share_cells ,
                                 const kernel_sizes blocks,
                                 const kernel_sizes grids)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int num_pairs = temporaries->pairs.number_normal;

  for (int pair = index; pair < num_pairs; pair += grid_size)
    {
      propagate_main_pair(pair, temporaries, cell_state_arr, counter_select, share_cells);
    }

#if CUDA_CAN_USE_TAIL_LAUNCH
  if (index == grid_size - 1)
    {

      handleSplitterTagChangesAndTerminationKernel <<< grids.tag_change, blocks.tag_change, 0, cudaStreamTailLaunch>>>(cell_state_arr,
                                                                                                                       temporaries,
                                                                                                                       cell_info_arr,
                                                                                                                       counter_select,
                                                                                                                       share_cells,
                                                                                                                       blocks, grids);
    }
#endif
}

static __global__
void handleSplitterTagChangesAndTerminationKernel(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                                  Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                                  const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                                  const bool counter_select,
                                                  const bool share_cells,
                                                  const kernel_sizes blocks,
                                                  const kernel_sizes grids)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      update_cell_tag(cell, cell_state_arr, temporaries, cell_info_arr, counter_select);
    }

  if (index == grid_size - 1)
    //Will be called with just 1 thread, but...
    {
      temporaries->reset_counters[!counter_select] = 0;
      if (temporaries->continue_flag)
        {
          temporaries->continue_flag = 0;
#if CUDA_CAN_USE_TAIL_LAUNCH

          propagateSplitterTagsKernel <<< grids.main_prop, blocks.main_prop, 0, cudaStreamTailLaunch>>>(cell_state_arr,
                                                                                                        temporaries,
                                                                                                        cell_info_arr,
                                                                                                        !counter_select,
                                                                                                        share_cells,
                                                                                                        blocks, grids);
#endif
        }
#if !CUDA_CAN_USE_TAIL_LAUNCH
      else /*if (!temporaries->continue_flag)*/
        {
          temporaries->stop_flag = 1;
        }
#endif
    }
}



static __global__
void clusterSplittingMainDefer(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                               Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                               const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                               const bool share_cells,
                               const kernel_sizes blocks, kernel_sizes grids)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      if (grids.main_prop < 0)
        //We want the minimum grid size.
        {
          grids.main_prop =  Helpers::int_ceil_div(temporaries->pairs.number_normal, blocks.main_prop);
        }

#if CUDA_CAN_USE_TAIL_LAUNCH

      propagateSplitterTagsKernel <<< grids.main_prop, blocks.main_prop, 0, cudaStreamTailLaunch>>>(cell_state_arr,
                                                                                                    temporaries,
                                                                                                    cell_info_arr,
                                                                                                    0,
                                                                                                    share_cells,
                                                                                                    blocks, grids);
#else

      //int counter = 0;

      bool counter_select = false;

      while (!temporaries->stop_flag)
        {
          propagateSplitterTagsKernel <<< grids.main_prop, blocks.main_prop>>>(cell_state_arr,
                                                                               temporaries,
                                                                               cell_info_arr,
                                                                               counter_select,
                                                                               share_cells,
                                                                               blocks, grids);

          handleSplitterTagChangesAndTerminationKernel <<< grids.tag_change, blocks.tag_change>>>(cell_state_arr,
                                                                                                  temporaries,
                                                                                                  cell_info_arr,
                                                                                                  counter_select,
                                                                                                  share_cells,
                                                                                                  blocks, grids);

          counter_select = !counter_select;
          //++counter;
        }
      //printf("SPLITTING: %16d\n", counter);
#endif
    }
}


void TASplitting::splitClusterGrowing(EventDataHolder & holder,
                                      const ConstantDataHolder & instance_data,
                                      const TASOptionsHolder & options,
                                      const IGPUKernelSizeOptimizer & optimizer,
                                      const bool synchronize,
                                      CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  TopoAutomatonSplittingTemporaries * temps = TASHacks::get_temporaries(holder);

  cudaMemsetAsync(&(temps->continue_flag),  0, sizeof(int),     stream_to_use);
  cudaMemsetAsync(&(temps->reset_counters), 0, sizeof(int) * 2, stream_to_use);
  cudaMemsetAsync(&(temps->stop_flag),      0, sizeof(int),     stream_to_use);

  if (optimizer.can_use_cooperative_groups())
    {
      const CUDAKernelLaunchConfiguration cfg_mainprop = optimizer.get_launch_configuration("TopoAutomatonSplitting", 7);

      void * main_args[] = { &holder.m_cell_state_dev, &temps,
                             &holder.m_cell_info_dev,
                             (void *) & (options.m_options->share_border_cells)
                           };

      cudaLaunchCooperativeKernel((void *) clusterSplittingMainCooperativeKernel,
                                  cfg_mainprop.grid_x, cfg_mainprop.block_x,
                                  main_args, 0, stream_to_use);

    }
  else if (optimizer.can_use_dynamic_parallelism())
    {
      const CUDAKernelLaunchConfiguration cfg_mainprop    = optimizer.get_launch_configuration("TopoAutomatonSplitting", 8);
      const CUDAKernelLaunchConfiguration cfg_tagchange   = optimizer.get_launch_configuration("TopoAutomatonSplitting", 9);

      kernel_sizes blocks, grids;

      blocks.main_prop    = cfg_mainprop.block_x;
      blocks.tag_change   = cfg_tagchange.block_x;

      grids.main_prop    = optimizer.use_minimal_kernel_sizes() ? -1 : cfg_mainprop.grid_x;
      grids.tag_change   = cfg_tagchange.grid_x;

      clusterSplittingMainDefer <<< 1, 1, 0, stream_to_use>>>(holder.m_cell_state_dev,
                                                              temps,
                                                              holder.m_cell_info_dev,
                                                              options.m_options->share_border_cells,
                                                              blocks, grids);
    }
  else
    {
      throw std::runtime_error("The GPU must support either cooperative grid launches "
                               "or dynamic parallelism for the algorithm to work properly.");
      //Well, technically, we could code something up
      //(launch, say, 64 iterations, since the maximum
      // I have seen for ttbar is around ~40
      // and iterations are idempotent after the
      // stopping criterion has been reached),
      //but the added complexity and the fact that
      //architectures without dynamic parallelism
      //either are or soon will be deprecated
      //mean that it's not worth the effort.
    }

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}




/******************************************************************************************
 * Calculate the cell weights (only if indeed using shared_cells).
 ******************************************************************************************/

namespace
{
  constexpr int clusterprop_abs_E = 0;
  constexpr int clusterprop_E = 1;
  constexpr int clusterprop_x = 2;
  constexpr int clusterprop_y = 3;
  constexpr int clusterprop_z = 4;
}


static __global__
void sumCellsForCentroidKernel( Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                const Helpers::CUDA_kernel_object<GeometryArr> geometry)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const TASTag tag = cell_state_arr->clusterTag[cell];
      if (tag.is_part_of_splitter_cluster() && !tag.is_shared())
        {
          const int cluster = temporaries->cell_to_cluster_map[cell];

          const float energy = cell_info_arr->energy[cell];
          const float abs_energy = fabsf(energy);
          const float x = geometry->x[cell];
          const float y = geometry->y[cell];
          const float z = geometry->z[cell];

          atomicAdd( &( temporaries->get_cluster_extra_array<clusterprop_abs_E, float>(cluster) ), abs_energy    );
          atomicAdd( &( temporaries->get_cluster_extra_array<clusterprop_E, float>(cluster) ), energy        );

          atomicAdd( &( temporaries->get_cluster_extra_array<clusterprop_x, float>(cluster) ), x * abs_energy );
          atomicAdd( &( temporaries->get_cluster_extra_array<clusterprop_y, float>(cluster) ), y * abs_energy );
          atomicAdd( &( temporaries->get_cluster_extra_array<clusterprop_z, float>(cluster) ), z * abs_energy );

        }
    }
}



static __global__
void calculateCentroidsKernel(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                              const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{

  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  const int cluster_number = clusters_arr->number;

  for (int cluster = index; cluster < cluster_number; cluster += grid_size)
    {

      const float abs_energy = temporaries->get_cluster_extra_array<clusterprop_abs_E, float>(cluster);

      if (abs_energy > 0)
        {
          const float rev_abs_E = 1.0f / abs_energy;

          temporaries->get_cluster_extra_array<clusterprop_x, float>(cluster) *= rev_abs_E; // x

          temporaries->get_cluster_extra_array<clusterprop_y, float>(cluster) *= rev_abs_E; // y

          temporaries->get_cluster_extra_array<clusterprop_z, float>(cluster) *= rev_abs_E; // z
        }
    }
}

static __global__
void calculateCentroidsKernelDeferKernel(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                         const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                         const int i_dimBlock)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      const int i_dimGrid = Helpers::int_ceil_div(clusters_arr->number, i_dimBlock);

#if CUDA_CAN_USE_TAIL_LAUNCH
      calculateCentroidsKernel <<< i_dimGrid, i_dimBlock, 0, cudaStreamTailLaunch>>>(temporaries, clusters_arr);
#else
      calculateCentroidsKernel <<< i_dimGrid, i_dimBlock>>>(temporaries, clusters_arr);
#endif
    }
}

static __global__
void assignFinalCellsKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                             Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                             const Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                             const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                             const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const TASTag tag = cell_state_arr->clusterTag[cell];
      if (tag.is_part_of_splitter_cluster())
        {
          if (opts->share_border_cells && tag.is_shared())
            {
              const uint32_t shared_clusters_packed = temporaries->cell_to_cluster_map[cell];
              const int cluster_1 = shared_clusters_packed & 0xFFFFU;
              const int cluster_2 = (shared_clusters_packed >> 16) & 0xFFFFU;

              const float cell_x = geometry->x[cell];
              const float cell_y = geometry->y[cell];
              const float cell_z = geometry->z[cell];


              const float delta_x_1 = cell_x - temporaries->get_cluster_extra_array<clusterprop_x, float>(cluster_1);
              const float delta_x_2 = cell_x - temporaries->get_cluster_extra_array<clusterprop_x, float>(cluster_2);

              const float delta_y_1 = cell_y - temporaries->get_cluster_extra_array<clusterprop_y, float>(cluster_1);
              const float delta_y_2 = cell_y - temporaries->get_cluster_extra_array<clusterprop_y, float>(cluster_2);

              const float delta_z_1 = cell_z - temporaries->get_cluster_extra_array<clusterprop_z, float>(cluster_1);
              const float delta_z_2 = cell_z - temporaries->get_cluster_extra_array<clusterprop_z, float>(cluster_2);


              const float d_1 = sqrtf(delta_x_1 * delta_x_1 + delta_y_1 * delta_y_1 + delta_z_1 * delta_z_1);

              const float d_2 = sqrtf(delta_x_2 * delta_x_2 + delta_y_2 * delta_y_2 + delta_z_2 * delta_z_2);

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

              float E_1 = temporaries->get_cluster_extra_array<clusterprop_E, float>(cluster_1);

              float E_2 = temporaries->get_cluster_extra_array<clusterprop_E, float>(cluster_2);

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

              float weight = E_1 / (E_1 + E_2 * r);
              float rev_weight = E_2 / (E_2 + E_1 * r_reverse);

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
                  cell_state_arr->clusterTag[cell] = ClusterTag::make_tag(cluster_1, __float_as_uint(rev_weight), cluster_2);
                }
              else if (weight == 0.5f)
                {
                  const int max_cluster = cluster_1 > cluster_2 ? cluster_1 : cluster_2;
                  const int min_cluster = cluster_1 > cluster_2 ? cluster_2 : cluster_1;
                  cell_state_arr->clusterTag[cell] = ClusterTag::make_tag(max_cluster, __float_as_uint(weight), min_cluster);
                }
              else /*if (weight < 0.5f)*/
                {
                  cell_state_arr->clusterTag[cell] = ClusterTag::make_tag(cluster_2, __float_as_uint(weight), cluster_1);
                }
            }
          else
            {
              const int this_cluster = temporaries->cell_to_cluster_map[cell];
              cell_state_arr->clusterTag[cell] = ClusterTag::make_tag(this_cluster);
            }
        }
      else if (tag.is_non_assigned_part_of_split_cluster())
        {
          const int this_cluster = tag.index();
          cell_state_arr->clusterTag[cell] = ClusterTag::make_tag(this_cluster);
          //Cells that are part of a pre-splitter cluster get added to the "same"?

          atomicMax(&(clusters_arr->seedCellID[this_cluster]), cell);
          //Not the seed cell, but just a consistent way of marking this cluster as still valid...
        }
      else if (tag.is_part_of_original_cluster())
        {
          const int this_cluster = tag.index();
          cell_state_arr->clusterTag[cell] = ClusterTag::make_tag(this_cluster);
        }
      else
        {
          cell_state_arr->clusterTag[cell] = ClusterTag::make_invalid_tag();
        }
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

  TopoAutomatonSplittingTemporaries * temps = TASHacks::get_temporaries(holder);

  if (options.m_options->share_border_cells)
    {
      cudaMemsetAsync(temps->secondary_array, 0, sizeof(tag_type) * NCaloCells, cudaStreamPerThread);

      const CUDAKernelLaunchConfiguration cfg_sumcells = optimizer.get_launch_configuration("TopoAutomatonSplitting", 10);
      const CUDAKernelLaunchConfiguration cfg_centroid = optimizer.get_launch_configuration("TopoAutomatonSplitting", 11);

      sumCellsForCentroidKernel <<< cfg_sumcells.grid_x, cfg_sumcells.block_x, 0, stream_to_use>>>(temps,
                                                                                                   holder.m_cell_state_dev,
                                                                                                   holder.m_cell_info_dev,
                                                                                                   instance_data.m_geometry_dev);
      if (optimizer.use_minimal_kernel_sizes())
        {
          calculateCentroidsKernelDeferKernel <<< 1, 1, 0, stream_to_use>>>(temps, holder.m_clusters_dev, cfg_centroid.block_x);
        }
      else
        {
          calculateCentroidsKernel <<< cfg_centroid.grid_x, cfg_centroid.block_x, 0, stream_to_use>>>(temps, holder.m_clusters_dev);
        }

    }

  const CUDAKernelLaunchConfiguration cfg_finalize = optimizer.get_launch_configuration("TopoAutomatonSplitting", 12);

  assignFinalCellsKernel <<< cfg_finalize.grid_x, cfg_finalize.block_x, 0, stream_to_use>>>(holder.m_cell_state_dev,
                                                                                            holder.m_clusters_dev,
                                                                                            temps,
                                                                                            instance_data.m_geometry_dev,
                                                                                            options.m_options_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/*******************************************************************************************************************************/

void TASplitting::register_kernels(IGPUKernelSizeOptimizer & optimizer)
{

  void * kernels[] = { (void *) fillNeighboursKernel,
                       (void *) countInferiorNeighsKernel,
                       (void *) findLocalMaximaKernel,
                       (void *) fixClustersWithoutMaximaKernel,
                       (void *) secondaryMaximaCooperativeKernel,
                       (void *) propagateForMaximaExclusionKernel,
                       (void *) cleanUpSecondariesKernel,
                       (void *) clusterSplittingMainCooperativeKernel,
                       (void *) propagateSplitterTagsKernel,
                       (void *) handleSplitterTagChangesAndTerminationKernel,
                       (void *) sumCellsForCentroidKernel,
                       (void *) calculateCentroidsKernel,
                       (void *) assignFinalCellsKernel
                     };

  int blocksizes[] = { FillNeighboursBlockSize,
                       CountInferiorNeighsBlockSize,
                       FindLocalMaximaBlockSize,
                       FixClustersWithoutMaximaBlockSize,
                       ExcludeMaximaPropagationBlockSize,
                       ExcludeMaximaOnePropagationBlockSize,
                       CleanUpSecondariesBlockSize,
                       ClusterSplittingMainPropagationBlockSize,
                       PropagateSplitTagsBlockSize,
                       HandleSplitTagChangesBlockSize,
                       SumCellsBlockSize,
                       CalculateCentroidsBlockSize,
                       FinalCellsBlockSize
                     };

  int  gridsizes[] = { Helpers::int_ceil_div(NCaloCells, FillNeighboursBlockSize),
                       Helpers::int_ceil_div(NMaxPairs + NMaxPairs / 2, CountInferiorNeighsBlockSize),
                       Helpers::int_ceil_div(NCaloCells, FindLocalMaximaBlockSize),
                       Helpers::int_ceil_div(NCaloCells, FixClustersWithoutMaximaBlockSize),
                       IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch,
                       Helpers::int_ceil_div(NMaxPairs, ExcludeMaximaOnePropagationBlockSize),
                       Helpers::int_ceil_div(NCaloCells, CleanUpSecondariesBlockSize),
                       IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch,
                       Helpers::int_ceil_div(NMaxPairs, PropagateSplitTagsBlockSize),
                       Helpers::int_ceil_div(NCaloCells, HandleSplitTagChangesBlockSize),
                       Helpers::int_ceil_div(NCaloCells, SumCellsBlockSize),
                       Helpers::int_ceil_div(NMaxClusters, CalculateCentroidsBlockSize),
                       Helpers::int_ceil_div(NCaloCells, FinalCellsBlockSize)
                     };

  int   maxsizes[] = { NCaloCells,
                       NMaxPairs + NMaxPairs / 2,
                       NCaloCells,
                       NCaloCells,
                       NMaxPairs,
                       NMaxPairs,
                       NCaloCells,
                       std::max(NMaxPairs, NCaloCells),
                       NMaxPairs,
                       NCaloCells,
                       NCaloCells,
                       NMaxClusters,
                       NCaloCells
                     };

  optimizer.register_kernels("TopoAutomatonSplitting", 13, kernels, blocksizes, gridsizes, maxsizes);

}