//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "BasicGPUClusterInfoCalculatorImpl.h"

#ifndef CALORECGPU_USE_INDIVIDUAL_TEMPORARY_ARRAYS

#define CALORECGPU_TEMP_STRUCT_TO_USE InfoTemps

#endif

#include "TemporaryHelpers.h"

#include <cmath>
#include <stdio.h>

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

using namespace CaloRecGPU;
using namespace BasicClusterInfoCalculator;

namespace
{
  namespace Temporaries
  {
    struct InfoTemps;
    
    CALORECGPU_TEMPARR_1(seed_cell_phi, etaCaloFrame, float);

    CALORECGPU_TEMPARR_1(abs_energy, phiCaloFrame, float);

    CALORECGPU_TEMPARR_1(abs_energy_aux, eta1CaloFrame, float);

    CALORECGPU_TEMPARR_1(energy_aux, phi1CaloFrame, float);

    CALORECGPU_TEMPARR_1(eta_aux, eta2CaloFrame, float);

    CALORECGPU_TEMPARR_1(phi_aux, phi2CaloFrame, float);
    
    struct InfoTemps
    {
      float seed_cell_phi  [NMaxClusters];
      float abs_energy     [NMaxClusters];
      float abs_energy_aux [NMaxClusters]; 
      float energy_aux     [NMaxClusters];
      float eta_aux        [NMaxClusters];
      float phi_aux        [NMaxClusters];
    };
  }
}



/**********************************************************************************/
constexpr static int SeedCellPropertiesBlockSize = 512;

constexpr static int CalculateClusterInfoBlockSize = 512;
constexpr static int FinalizeClusterInfoBlockSize = 256;
constexpr static int ClearInvalidCellsBlockSize = 512;

/**********************************************************************************/


__global__ static
void seedCellPropertiesKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                              const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                              const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                              const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int cluster_number = clusters_arr->number;
  for (int cluster = index; cluster < cluster_number; cluster += grid_size)
    {
      clusters_arr->clusterEnergy[cluster] = 0.f;
      Temporaries::energy_aux(clusters_arr, cluster) = 0.f;

      clusters_arr->clusterEta[cluster] = 0.f;
      Temporaries::eta_aux(clusters_arr, cluster) = 0.f;

      clusters_arr->clusterPhi[cluster] = 0.f;
      Temporaries::phi_aux(clusters_arr, cluster) = 0.f;

      Temporaries::abs_energy(clusters_arr, cluster) = 0.f;
      Temporaries::abs_energy_aux(clusters_arr, cluster) = 0.f;

      const int seed_cell = clusters_arr->seedCellIndex[cluster];
      if (seed_cell >= 0)
        {
          Temporaries::seed_cell_phi(clusters_arr, cluster) = geometry->phi[cell_info_arr->get_hash_ID(seed_cell, assume_complete_cells)];
        }
      else
        {
          Temporaries::seed_cell_phi(clusters_arr, cluster) = 0.f;
        }
    }
}

void BasicClusterInfoCalculator::updateSeedCellProperties(CaloRecGPU::EventDataHolder & holder,
                                                          const ConstantDataHolder & instance_data,
                                                          const IGPUKernelSizeOptimizer & optimizer,
                                                          const bool synchronize,
                                                          CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration launch_config = optimizer.get_launch_configuration("BasicClusterInfoCalculator", 0);

  seedCellPropertiesKernel <<< launch_config.grid_x, launch_config.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                                holder.m_cell_info_dev,
                                                                                                instance_data.m_geometry_dev,
                                                                                                holder.m_cell_info->complete);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}


/**********************************************************************************/

__global__ static
void calculateClusterInfoKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int n_cells = clusters_arr->number_cells;

  auto add_cell_contribution = [&](const int cluster_index, const int cell_index, const float weight, const bool load_info = true,
                                   int cell_hash_ID = -1, float energy = 0, float abs_energy = 0, float phi_raw = 0, float eta = 0)
  {
    if (load_info)
      {
        cell_hash_ID = cell_info_arr->get_hash_ID(cell_index, assume_complete_cells);
        energy = cell_info_arr->energy[cell_index];
        abs_energy = fabsf(energy);
        phi_raw = geometry->phi[cell_hash_ID];
        eta = geometry->eta[cell_hash_ID];
      }
    
    Helpers::device_kahan_babushka_neumaier(&(clusters_arr->clusterEnergy[cluster_index]),
                                            Temporaries::energy_aux_ptr(clusters_arr, cluster_index),
                                            energy * weight);

    Helpers::device_kahan_babushka_neumaier(Temporaries::abs_energy_ptr(clusters_arr, cluster_index),
                                            Temporaries::abs_energy_aux_ptr(clusters_arr, cluster_index),
                                            abs_energy * weight);

    Helpers::device_kahan_babushka_neumaier(&(clusters_arr->clusterEta[cluster_index]),
                                            Temporaries::eta_aux_ptr(clusters_arr, cluster_index),
                                            eta * abs_energy * weight);

    const float phi_0 = Temporaries::seed_cell_phi(clusters_arr, cluster_index);
    const float phi_real = Helpers::regularize_angle(phi_raw, phi_0);

    Helpers::device_kahan_babushka_neumaier(&(clusters_arr->clusterPhi[cluster_index]),
                                            Temporaries::phi_aux_ptr(clusters_arr, cluster_index),
                                            phi_real * abs_energy * weight);
  };


  if (clusters_arr->has_cells_per_cluster())
    {
      for (int cell = index; cell < n_cells; cell += grid_size)
        {
          add_cell_contribution(clusters_arr->clusterIndices[cell],
                                clusters_arr->cells.indices[cell],
                                clusters_arr->cellWeights[cell]);
        }
    }
  else
    {
      for (int cell = index; cell < n_cells; cell += grid_size)
        {
          const ClusterTag tag = clusters_arr->cells.tags[cell];

          if (tag.is_part_of_cluster())
            {
              if (tag.is_shared_between_clusters())
                {
                  const float secondary_weight = __int_as_float(tag.secondary_cluster_weight());
                  const float weight = 1.0f - secondary_weight;

                  const int cell_hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);
                  const float energy = cell_info_arr->energy[cell];
                  const float abs_energy = fabsf(energy);
                  const float phi_raw = geometry->phi[cell_hash_ID];
                  const float eta = geometry->eta[cell_hash_ID];

                  add_cell_contribution(tag.cluster_index(), cell, weight,
                                        false, cell_hash_ID, energy, abs_energy, phi_raw, eta);

                  add_cell_contribution(tag.secondary_cluster_index(), cell, secondary_weight,
                                        false, cell_hash_ID, energy, abs_energy, phi_raw, eta);
                }
              else
                {
                  add_cell_contribution(tag.cluster_index(), cell, 1.0f);
                }
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
      const float abs_energy = Temporaries::abs_energy(clusters_arr, cluster) +
                               Temporaries::abs_energy_aux(clusters_arr, cluster);

      if (abs_energy > 0)
        {
          const float rev_abs_energy = 1.f / abs_energy;

          const float pre_energy = clusters_arr->clusterEnergy[cluster];
          const float energy_correction = Temporaries::energy_aux(clusters_arr, cluster);

          const float pre_eta = clusters_arr->clusterEta[cluster];
          const float eta_correction = Temporaries::eta_aux(clusters_arr, cluster);

          //More precise ET computation:

          const float exp_1 = expf(pre_eta * rev_abs_energy);
          const float exp_2 = expf(eta_correction * rev_abs_energy);

          const float exp_mult = exp_1 * exp_2;

          const float numerator   = 2.f * Helpers::product_sum_cornea_harrison_tang(pre_energy, exp_mult,
                                                                                    energy_correction, exp_mult);
          const float inv_denominator = 1.f / fmaf(exp_mult, exp_mult, 1.f);

          const float cluster_ET = numerator * inv_denominator;

          //const float cluster_ET = energy / coshf(abs(eta));

          if ( !(cluster_ET > ET_threshold || (cut_in_absolute_ET && fabsf(cluster_ET) > ET_threshold) ) )
            {
              clusters_arr->seedCellIndex[cluster] = -1;
              clusters_arr->has_deleted_clusters = true; //Concurrent writes are alright
            }
          else
            {
              const float energy = pre_energy + energy_correction;
              const float eta = (pre_eta + eta_correction) * rev_abs_energy;

              clusters_arr->clusterEnergy[cluster] = energy;

              clusters_arr->clusterEt[cluster] = cluster_ET;

              clusters_arr->clusterEta[cluster] = eta;

              const float phi = (clusters_arr->clusterPhi[cluster] +
                                 Temporaries::eta_aux(clusters_arr, cluster)) * rev_abs_energy;

              clusters_arr->clusterPhi[cluster] = Helpers::regularize_angle(phi, 0.f);
            }
        }
      else
        {
          clusters_arr->seedCellIndex[cluster] = -1;
          clusters_arr->has_deleted_clusters = true; //Concurrent writes are alright
        }
    }

  if (index == 0)
    {
      clusters_arr->state = (clusters_arr->has_cells_per_cluster() ? ClusterInformationState::WithBasicInfo : ClusterInformationState::TagsWithBasicInfo);
    }
}

__global__ static
void clearInvalidCells(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  if (!clusters_arr->has_cells_per_cluster() && clusters_arr->has_deleted_clusters)
    {
      const int n_cells = clusters_arr->number_cells;
      for (int cell = index; cell < n_cells; cell += grid_size)
        {
          const ClusterTag tag = clusters_arr->cells.tags[cell];
          if (tag.is_part_of_cluster())
            {
              if (tag.is_shared_between_clusters())
                {
                  const int first_cluster = tag.cluster_index();
                  const int second_cluster = tag.secondary_cluster_index();

                  const int first_seed = clusters_arr->seedCellIndex[first_cluster];
                  const int second_seed = clusters_arr->seedCellIndex[second_cluster];

                  if (first_seed < 0 && second_seed < 0)
                    {
                      clusters_arr->cells.tags[cell] = ClusterTag:: make_invalid_tag();
                    }
                  else if (first_seed < 0)
                    {
                      clusters_arr->cells.tags[cell] = ClusterTag::make_tag(second_cluster);
                    }
                  else if (second_seed < 0)
                    {
                      clusters_arr->cells.tags[cell] = ClusterTag::make_tag(first_cluster);
                    }
                  else /*if (first_seed >= 0 && second_seed >= 0)*/
                    {
                      //Do nothing: the tag's already OK.
                    }
                }
              else
                {
                  if (clusters_arr->seedCellIndex[tag.cluster_index()] < 0)
                    {
                      clusters_arr->cells.tags[cell] = ClusterTag::make_invalid_tag();
                    }
                }
            }
        }
    }
  //We don't clean up the list of cells per cluster
  //(too much to adjust here, better to have a specialized
  // algorithm and/or rerun cluster sorting & compactification),
  //and we also have nothing to clean up if all the clusters are non-empty.
}

void BasicClusterInfoCalculator::calculateClusterProperties(CaloRecGPU::EventDataHolder & holder,
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

  calculateClusterInfoKernel <<< cfg_calculate.grid_x, cfg_calculate.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                                  holder.m_cell_info_dev,
                                                                                                  instance_data.m_geometry_dev,
                                                                                                  holder.m_cell_info->complete);

  finalizeClusterInfoKernel <<< cfg_finalize.grid_x, cfg_finalize.block_x, 0, stream_to_use>>>(holder.m_clusters_dev, cut_in_absolute_ET, ET_threshold);

  if (!holder.m_clusters.valid() || !holder.m_clusters->has_cells_per_cluster())
    //Our tools keep the state of the clusters up to date if they are available on the CPU,
    //and we can skip this altogether if we know we don't have anything to adjust.
    {
      clearInvalidCells <<< cfg_clear.block_x, cfg_clear.block_x, 0, stream_to_use>>>(holder.m_clusters_dev);
    }

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  if (holder.m_clusters.valid())
    {
      holder.m_clusters->state = (holder.m_clusters->has_cells_per_cluster() ?
                                  ClusterInformationState::WithBasicInfo :
                                  ClusterInformationState::TagsWithBasicInfo    );
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
