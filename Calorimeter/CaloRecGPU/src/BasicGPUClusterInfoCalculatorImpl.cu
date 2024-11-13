//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "BasicGPUClusterInfoCalculatorImpl.h"

#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

using namespace CaloRecGPU;
using namespace BasicClusterInfoCalculator;

/**********************************************************************************/
constexpr static int SeedCellPropertiesBlockSize = 512;

constexpr static int CalculateClusterInfoBlockSize = 320;
constexpr static int FinalizeClusterInfoBlockSize = 256;
constexpr static int ClearInvalidCellsBlockSize = 512;

/**********************************************************************************/

__global__ static
void seedCellPropertiesKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                              Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temporaries,
                              const Helpers::CUDA_kernel_object<GeometryArr> geometry)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int cluster_number = clusters_arr->number;
  for (int cluster = index; cluster < cluster_number; cluster += grid_size)
    {
      clusters_arr->clusterEnergy[cluster] = 0.f;
      clusters_arr->clusterEt[cluster] = 0.f;
      clusters_arr->clusterEta[cluster] = 0.f;
      clusters_arr->clusterPhi[cluster] = 0.f;
      const int seed_cell = clusters_arr->seedCellID[cluster];
      if (seed_cell >= 0)
        {
          temporaries->seedCellPhi[cluster] = geometry->phi[seed_cell];
        }
      else
        {
          temporaries->seedCellPhi[cluster] = 0.f;
        }
    }
}

__global__ static
void seedCellPropertiesDeferKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                   Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temporaries,
                                   const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                   const int i_dimBlock)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    {
      const int cluster_number = clusters_arr->number;

      //const int i_dimBlock = SeedCellPropertiesBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(cluster_number, i_dimBlock);

#if CUDA_CAN_USE_TAIL_LAUNCH
      seedCellPropertiesKernel <<< i_dimGrid, i_dimBlock, 0, cudaStreamTailLaunch>>>(clusters_arr, temporaries, geometry);
#else
      seedCellPropertiesKernel <<< i_dimGrid, i_dimBlock>>>(clusters_arr, temporaries, geometry);
#endif
    }
}


void BasicClusterInfoCalculator::updateSeedCellProperties(CaloRecGPU::EventDataHolder & holder,
                                                          CaloRecGPU::Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temps,
                                                          const ConstantDataHolder & instance_data,
                                                          const IGPUKernelSizeOptimizer & optimizer,
                                                          const bool synchronize,
                                                          CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration launch_config = optimizer.get_launch_configuration("BasicClusterInfoCalculator", 0);

  if (optimizer.use_minimal_kernel_sizes() && optimizer.can_use_dynamic_parallelism())
    {

      seedCellPropertiesDeferKernel <<< 1, 1, 0, stream_to_use>>>(holder.m_clusters_dev, temps, instance_data.m_geometry_dev, launch_config.block_x);
    }
  else
    {
      seedCellPropertiesKernel <<< launch_config.grid_x, launch_config.block_x, 0, stream_to_use>>>(holder.m_clusters_dev, temps, instance_data.m_geometry_dev);

    }

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}


/**********************************************************************************/

__global__ static
void calculateClusterInfoKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                const Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temporaries)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const ClusterTag tag = cell_state_arr->clusterTag[cell];
      if (tag.is_part_of_cluster())
        //By this point they all have the terminals anyway, so...
        {
          if (tag.is_shared_between_clusters())
            {
              const int primary_cluster = tag.cluster_index();
              const int secondary_cluster = tag.secondary_cluster_index();

              const float secondary_weight = __int_as_float(tag.secondary_cluster_weight());
              const float weight = 1.0f - secondary_weight;

              const float energy = cell_info_arr->energy[cell];
              const float abs_energy = fabsf(energy);
              const float phi_raw = geometry->phi[cell];

              atomicAdd(&(clusters_arr->clusterEnergy[primary_cluster]), energy * weight);
              atomicAdd(&(clusters_arr->clusterEt[primary_cluster]), abs_energy * weight);
              atomicAdd(&(clusters_arr->clusterEta[primary_cluster]), abs_energy * geometry->eta[cell] * weight);

              const float primary_phi_0 = temporaries->seedCellPhi[primary_cluster];
              const float primary_phi_real = Helpers::regularize_angle(phi_raw, primary_phi_0);
              atomicAdd(&(clusters_arr->clusterPhi[primary_cluster]), primary_phi_real * abs_energy * weight);

              atomicAdd(&(clusters_arr->clusterEnergy[secondary_cluster]), energy * secondary_weight);
              atomicAdd(&(clusters_arr->clusterEt[secondary_cluster]), abs_energy * secondary_weight);
              atomicAdd(&(clusters_arr->clusterEta[secondary_cluster]), abs_energy * geometry->eta[cell] * secondary_weight);

              const float secondary_phi_0 = temporaries->seedCellPhi[secondary_cluster];
              const float secondary_phi_real = Helpers::regularize_angle(phi_raw, secondary_phi_0);
              atomicAdd(&(clusters_arr->clusterPhi[secondary_cluster]), secondary_phi_real * abs_energy * secondary_weight);
            }
          else
            {
              const int cluster_index = tag.cluster_index();
              const float energy = cell_info_arr->energy[cell];
              const float abs_energy = fabsf(energy);
              const float phi_raw = geometry->phi[cell];

              atomicAdd(&(clusters_arr->clusterEnergy[cluster_index]), energy);
              atomicAdd(&(clusters_arr->clusterEt[cluster_index]), abs_energy);
              atomicAdd(&(clusters_arr->clusterEta[cluster_index]), abs_energy * geometry->eta[cell]);

              const float phi_0 = temporaries->seedCellPhi[cluster_index];
              const float phi_real = Helpers::regularize_angle(phi_raw, phi_0);
              atomicAdd(&(clusters_arr->clusterPhi[cluster_index]), phi_real * abs_energy);
            }
        }
    }
}


__global__ static
void finalizeClusterInfoKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                               const bool cut_in_absolute_ET, const float ET_threshold   )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int cluster_number = clusters_arr->number;
  for (int cluster = index; cluster < cluster_number; cluster += grid_size)
    {
      const float abs_energy = clusters_arr->clusterEt[cluster];

      if (abs_energy > 0)
        {
          const float tempeta = clusters_arr->clusterEta[cluster] / abs_energy;

          clusters_arr->clusterEta[cluster] = tempeta;

          const float temp_ET = clusters_arr->clusterEnergy[cluster] / coshf(abs(tempeta));

          clusters_arr->clusterEt[cluster] = temp_ET;

          clusters_arr->clusterPhi[cluster] = Helpers::regularize_angle(clusters_arr->clusterPhi[cluster] / abs_energy, 0.f);

          if ( !(temp_ET > ET_threshold || (cut_in_absolute_ET && fabsf(temp_ET) > ET_threshold) ) )
            {
              clusters_arr->seedCellID[cluster] = -1;
            }
        }
      else
        {
          clusters_arr->seedCellID[cluster] = -1;
          //This is just a way to signal that this is an invalid cluster.
        }
    }
}

__global__ static
void finalizeClustersDeferKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                 const bool cut_in_absolute_ET, const float ET_threshold,
                                 const int i_dimBlock)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    {
      const int cluster_number = clusters_arr->number;

      //const int i_dimBlock = FinalizeClusterInfoBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(cluster_number, i_dimBlock);
      const dim3 dimBlock(i_dimBlock, 1, 1);
      const dim3 dimGrid(i_dimGrid, 1, 1);
#if CUDA_CAN_USE_TAIL_LAUNCH
      finalizeClusterInfoKernel <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(clusters_arr, cut_in_absolute_ET, ET_threshold);
#else
      finalizeClusterInfoKernel <<< dimGrid, dimBlock>>>(clusters_arr, cut_in_absolute_ET, ET_threshold);
#endif
    }
}

__global__ static
void clearInvalidCells(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                       const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  for (int cell = index; cell < NCaloCells; cell += grid_size)
    {
      const ClusterTag tag = cell_state_arr->clusterTag[cell];
      if (tag.is_part_of_cluster())
        //By this point they all have the terminals anyway, so...
        {
          if (tag.is_shared_between_clusters())
            {
              const int first_cluster = tag.cluster_index();
              const int second_cluster = tag.secondary_cluster_index();

              const int first_seed = clusters_arr->seedCellID[first_cluster];
              const int second_seed = clusters_arr->seedCellID[second_cluster];

              if (first_seed < 0 && second_seed < 0)
                {
                  cell_state_arr->clusterTag[cell] = ClusterTag:: make_invalid_tag();
                }
              else if (first_seed < 0)
                {
                  cell_state_arr->clusterTag[cell] = ClusterTag::make_tag(second_cluster);
                }
              else if (second_seed < 0)
                {
                  cell_state_arr->clusterTag[cell] = ClusterTag::make_tag(first_cluster);
                }
              else /*if (first_seed >= 0 && second_seed >= 0)*/
                {
                  //Do nothing: the tag's already OK.
                }
            }
          else
            {
              if (clusters_arr->seedCellID[tag.cluster_index()] < 0)
                {
                  cell_state_arr->clusterTag[cell] = ClusterTag:: make_invalid_tag();
                }
            }
        }
    }
}

void BasicClusterInfoCalculator::calculateClusterProperties(CaloRecGPU::EventDataHolder & holder,
                                                            CaloRecGPU::Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temps,
                                                            const ConstantDataHolder & instance_data,
                                                            const IGPUKernelSizeOptimizer & optimizer,
                                                            const bool synchronize,
                                                            const bool cut_in_absolute_ET, const float ET_threshold,
                                                            CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration cfg_calculate = optimizer.get_launch_configuration("BasicClusterInfoCalculator", 1);
  const CUDAKernelLaunchConfiguration cfg_finalize  = optimizer.get_launch_configuration("BasicClusterInfoCalculator", 2);
  const CUDAKernelLaunchConfiguration cfg_clear     = optimizer.get_launch_configuration("BasicClusterInfoCalculator", 3);

  calculateClusterInfoKernel <<< cfg_calculate.grid_x, cfg_calculate.block_x, 0, stream_to_use>>>(holder.m_clusters_dev, holder.m_cell_state_dev,
                                                                                                  holder.m_cell_info_dev, instance_data.m_geometry_dev, temps);

  if (optimizer.use_minimal_kernel_sizes() && optimizer.can_use_dynamic_parallelism())
    {
      finalizeClustersDeferKernel <<< 1, 1, 0, stream_to_use>>>(holder.m_clusters_dev, cut_in_absolute_ET, ET_threshold, cfg_finalize.block_x);
    }
  else
    {
      finalizeClusterInfoKernel <<< cfg_finalize.grid_x, cfg_finalize.block_x, 0, stream_to_use>>>(holder.m_clusters_dev, cut_in_absolute_ET, ET_threshold);
    }
  clearInvalidCells <<< cfg_clear.block_x, cfg_clear.block_x, 0, stream_to_use>>>(holder.m_cell_state_dev, holder.m_clusters_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/*******************************************************************************************************************************/

void BasicClusterInfoCalculator::register_kernels(IGPUKernelSizeOptimizer & optimizer)
{
  void * kernels[] = { (void *) seedCellPropertiesKernel,
                       (void *) calculateClusterInfoKernel,
                       (void *) finalizeClusterInfoKernel,
                       (void *) clearInvalidCells
                     };

  int blocksizes[] = { SeedCellPropertiesBlockSize,
                       CalculateClusterInfoBlockSize,
                       FinalizeClusterInfoBlockSize,
                       ClearInvalidCellsBlockSize
                     };

  int  gridsizes[] = { Helpers::int_ceil_div(NMaxClusters, SeedCellPropertiesBlockSize),
                       Helpers::int_ceil_div(NCaloCells, CalculateClusterInfoBlockSize),
                       Helpers::int_ceil_div(NMaxClusters, FinalizeClusterInfoBlockSize),
                       Helpers::int_ceil_div(NCaloCells, ClearInvalidCellsBlockSize)
                     };

  int   maxsizes[] = { NMaxClusters,
                       NCaloCells,
                       NMaxClusters,
                       NCaloCells
                     };

  optimizer.register_kernels("BasicClusterInfoCalculator", 4, kernels, blocksizes, gridsizes, maxsizes);
}