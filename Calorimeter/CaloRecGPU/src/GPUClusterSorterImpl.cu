//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_SORTER_DEBUG_CELL_ASSIGNMENT

  #define CALORECGPU_SORTER_DEBUG_CELL_ASSIGNMENT 0

#endif

#ifndef CALORECGPU_SORTER_FORCE_FULL_ONESWEEP_SORTING

  #define CALORECGPU_SORTER_FORCE_FULL_ONESWEEP_SORTING 0

#endif

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "GPUClusterSorterImpl.h"

#ifndef CALORECGPU_USE_INDIVIDUAL_TEMPORARY_ARRAYS

#define CALORECGPU_TEMP_STRUCT_TO_USE SorterTemps

#endif

#include "TemporaryHelpers.h"

#include <cstring>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>
#include <stdio.h>

#include "FPHelpers.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

using namespace CaloRecGPU;
using namespace GPUClusterSorting;

namespace
{
  namespace Temporaries
  {
    struct SorterTemps;
    
    CALORECGPU_TEMPVAR(clusters_to_exclude, nExtraCellSampling, 0, int);
    CALORECGPU_TEMPVAR(old_num_cells, nExtraCellSampling, 2, int);

    CALORECGPU_TEMPARR_1(old_seed_cells, vertexFraction, int);

    CALORECGPU_TEMPARR_1(num_cells_per_cluster, nVertexFraction, int);

    CALORECGPU_TEMPARR_1(intermediate_cell_counters, etaCaloFrame, int);

    CALORECGPU_TEMPARR_1(abs_energy, phiCaloFrame, float);

    CALORECGPU_TEMPARR_1(abs_energy_aux, eta1CaloFrame, float);

    CALORECGPU_TEMPARR_1(energy_aux, phi1CaloFrame, float);
    
    CALORECGPU_TEMPARR_1(eta_aux, eta2CaloFrame, float);

    CALORECGPU_TEMPARR_1(histogram_buffer, deltaPhi, uint32_t);

    //We only use the first 4 entries of the following temporaries,
    //as we only have 4 iterations, but it's convenient to index the whole array.

    CALORECGPU_TEMP2DARR_1(onesweep_counters, energyPerSample, uint32_t);

    CALORECGPU_TEMP2DARR_1(sorting_keys, maxEPerSample, uint32_t);

    //For the sorting algorithm, since it is in ascending order,
    //we will sort the bit-flipped versions, which,
    //given floating points in total ordering, still preserves
    //the right order in the end.

    CALORECGPU_TEMP2DARR_1(sorting_values, etaPerSample, uint16_t);
    static_assert(NMaxClusters - 1 <= std::numeric_limits<uint16_t>::max());

    CALORECGPU_TEMPARR_1(prefix_sum_counters, deltaAlpha, uint32_t);

    CALORECGPU_TEMPARR_1(old_to_new_cluster_map, centerX, int);

    struct SorterTemps
    {
      int clusters_to_exclude;
      int old_num_cells;
            
      int old_seed_cells             [NMaxClusters];
      int num_cells_per_cluster      [NMaxClusters];
      int intermediate_cell_counters [NMaxClusters];
      
      float abs_energy               [NMaxClusters];
      float abs_energy_aux           [NMaxClusters];
      float energy                   [NMaxClusters];
      float energy_aux               [NMaxClusters];
      float eta_aux                  [NMaxClusters];

      uint32_t histogram_buffer      [NMaxClusters];
      uint32_t onesweep_counters     [NumSamplings][NMaxClusters];
      uint32_t sorting_keys          [NumSamplings][NMaxClusters];
      uint16_t sorting_values        [NumSamplings][NMaxClusters];
      
      uint32_t prefix_sum_counters   [NMaxClusters];
      
      int old_to_new_cluster_map     [NMaxClusters];
    };
  }
}



/**********************************************************************************/
constexpr static int InitializeCountersBlockSize = 1024;
constexpr static int FirstCellIterationBlockSize = 512;
constexpr static int CalcAndPrepSortingBlockSize = 256;
constexpr static int FinalizeCellInfoBlockSize   = 512;

/**********************************************************************************/

__global__ static
void initializeCountersKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int cluster_number = clusters_arr->number;
  for (int cluster = index; cluster < cluster_number; cluster += grid_size)
    {
      Temporaries::num_cells_per_cluster(clusters_arr, cluster) = 0;

      clusters_arr->clusterEnergy[cluster] = 0.f;
      Temporaries::energy_aux(clusters_arr, cluster) = 0.f;

      clusters_arr->clusterEta[cluster] = 0.f;
      Temporaries::eta_aux(clusters_arr, cluster) = 0.f;

      Temporaries::abs_energy(clusters_arr, cluster) = 0.f;
      Temporaries::abs_energy_aux(clusters_arr, cluster) = 0.f;
    }

  if (index == 0)
    {
      Temporaries::clusters_to_exclude(clusters_arr) = 0;
      Temporaries::intermediate_cell_counters(clusters_arr, 0) = 0;
    }
}

__global__ static
void firstCellIterationKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                              const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                              const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                              const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int n_cells = clusters_arr->number_cells;

  auto add_cell_contribution = [&](const int cluster_index, const float weight,
                                   float energy, float abs_energy, float eta)
  {

    Helpers::device_kahan_babushka_neumaier(&(clusters_arr->clusterEnergy[cluster_index]),
                                            Temporaries::energy_aux_ptr(clusters_arr, cluster_index),
                                            energy * weight);

    Helpers::device_kahan_babushka_neumaier(Temporaries::abs_energy_ptr(clusters_arr, cluster_index),
                                            Temporaries::abs_energy_aux_ptr(clusters_arr, cluster_index),
                                            abs_energy * weight);

    Helpers::device_kahan_babushka_neumaier(&(clusters_arr->clusterEta[cluster_index]),
                                            Temporaries::eta_aux_ptr(clusters_arr, cluster_index),
                                            eta * abs_energy * weight);

    atomicAdd(&Temporaries::num_cells_per_cluster(clusters_arr, cluster_index), 1);
  };

  for (int cell = index; cell < n_cells; cell += grid_size)
    {
      const ClusterTag tag = clusters_arr->cells.tags[cell];

      if (tag.is_part_of_cluster())
        {
          const int cell_hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);
          const float energy = cell_info_arr->energy[cell];
          const float abs_energy = fabsf(energy);
          const float eta = geometry->eta[cell_hash_ID];

          if (tag.is_shared_between_clusters())
            {
              const float secondary_weight = __int_as_float(tag.secondary_cluster_weight());
              const float weight = 1.0f - secondary_weight;

              add_cell_contribution(tag.cluster_index(), weight, energy, abs_energy, eta); 

              add_cell_contribution(tag.secondary_cluster_index(), secondary_weight, energy, abs_energy, eta);
            }
          else
            {
              add_cell_contribution(tag.cluster_index(), 1.0f, energy, abs_energy, eta);
            }
        }

      clusters_arr->get_extra_cell_info(cell) = tag;

    }

  if (index == 0)
    {
      Temporaries::old_num_cells(clusters_arr) = n_cells;
    }
}

const int ExpectedNumberOfSortingIterations = 4;
//Each OneSweep iteration takes care of 8 bits.
//Each 32 bit floating point has, well, 4 * 8 bits.

const int ExpectedNumberOfBinsPerDigit = 256;
//8 bit digits means 2^8 = 256 bins.

const int ExpectedNumberOfItemsPerBlock = 4096;

__global__ static
void calculateInfoAndPrepareSort(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                 const bool cut_in_absolute_ET, const float ET_threshold   )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int cluster_number = clusters_arr->number;
  
#if CALORECGPU_SORTER_FORCE_FULL_ONESWEEP_SORTING
  const int max_cluster_number = max(cluster_number, 5000);
#else
  const int max_cluster_number = cluster_number;
#endif
  
  for (int cluster = index; cluster < max_cluster_number; cluster += grid_size)
    {
#if CALORECGPU_SORTER_FORCE_FULL_ONESWEEP_SORTING
      if (cluster >= cluster_number)
        {
          Temporaries::sorting_keys(clusters_arr, 0, cluster) = 0xFFFFFFFFU;
          Temporaries::sorting_values(clusters_arr, 0, cluster) = 0xFFFFU;
          
          Temporaries::histogram_buffer(clusters_arr, cluster) = 0;
          for (int i = 0; i < ExpectedNumberOfSortingIterations; ++i)
            {
              Temporaries::onesweep_counters(clusters_arr, i, cluster) = 0;
            }
          
          Temporaries::prefix_sum_counters(clusters_arr, cluster) = 0;
      
          continue;
        }
#endif
      
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
              Temporaries::old_seed_cells(clusters_arr, cluster) = -1;

              Temporaries::sorting_keys(clusters_arr, 0, cluster) = 0xFFFFFFFFU;
              
              atomicAdd(Temporaries::clusters_to_exclude_ptr(clusters_arr), 1);
            }
          else
            {
              Temporaries::old_seed_cells(clusters_arr, cluster) = clusters_arr->seedCellIndex[cluster];

              const uint32_t bit_pattern_ET = __float_as_uint(cluster_ET);
              const uint32_t total_ordering = FloatingPointHelpers::StandardFloat::template to_total_ordering<uint32_t>(bit_pattern_ET);

              Temporaries::sorting_keys(clusters_arr, 0, cluster) = ~total_ordering;
              //OneSweep orders in ascending order.
              //By bit-flipping everything, smaller
              //floats will be larger numbers -> appear later.
            }
        }
      else
        {
          Temporaries::old_seed_cells(clusters_arr, cluster) = -1;

          Temporaries::sorting_keys(clusters_arr, 0, cluster) = 0xFFFFFFFFU;

          atomicAdd(Temporaries::clusters_to_exclude_ptr(clusters_arr), 1);
        }
      

      Temporaries::sorting_values(clusters_arr, 0, cluster) = cluster;
      Temporaries::old_to_new_cluster_map(clusters_arr, cluster) = -1;
      
      
      if (max_cluster_number > ExpectedNumberOfItemsPerBlock)
        {
          Temporaries::histogram_buffer(clusters_arr, cluster) = 0;
          
          for (int i = 0; i < ExpectedNumberOfSortingIterations; ++i)
            {
              Temporaries::onesweep_counters(clusters_arr, i, cluster) = 0;
            }
        }

      Temporaries::prefix_sum_counters(clusters_arr, cluster) = 0;
    }

  if (index == 0)
    {
      clusters_arr->cellsPrefixSum[0] = 0;
    }
}

void GPUClusterSorting::initialPropertiesCalculation(CaloRecGPU::EventDataHolder & holder,
                                                     const ConstantDataHolder & instance_data,
                                                     const IGPUKernelSizeOptimizer & optimizer,
                                                     const bool synchronize,
                                                     const bool cut_in_absolute_ET, const float ET_threshold,
                                                     CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration cfg_init = optimizer.get_launch_configuration("GPUClusterSorting", 0);
  const CUDAKernelLaunchConfiguration cfg_cell = optimizer.get_launch_configuration("GPUClusterSorting", 1);
  const CUDAKernelLaunchConfiguration cfg_info = optimizer.get_launch_configuration("GPUClusterSorting", 2);

  initializeCountersKernel <<< cfg_init.grid_x, cfg_init.block_x, 0, stream_to_use>>>(holder.m_clusters_dev);

  firstCellIterationKernel <<< cfg_cell.grid_x, cfg_cell.block_x, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                      holder.m_cell_info_dev,
                                                                                      instance_data.m_geometry_dev,
                                                                                      holder.m_cell_info->complete);

  calculateInfoAndPrepareSort <<< cfg_info.grid_x, cfg_info.block_x, 0, stream_to_use>>>(holder.m_clusters_dev, cut_in_absolute_ET, ET_threshold);


  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/**********************************************************************************/

//Now: our version of OneSweep that leverages our temporaries
//and does not require synchronizing to get the number of clusters...
//Reference: https://gpuopen.com/learn/boosting_gpu_radix_sort/
//(cleaner codebase than CUB...)

namespace
{

  using KeyType = uint32_t;
  using ValueType = uint16_t;

  static_assert(std::is_same_v<KeyType,   std::decay_t<decltype(Temporaries::sorting_keys  (std::declval<ClusterInfoArr *>(), 0, 0))>>);
  static_assert(std::is_same_v<ValueType, std::decay_t<decltype(Temporaries::sorting_values(std::declval<ClusterInfoArr *>(), 0, 0))>>);

  constexpr int KeyBitSize = sizeof(KeyType) * CHAR_BIT;

  constexpr int OneSweepBitsPerBin = 8;
  constexpr int OneSweepNumBinsPerDigit = 1 << OneSweepBitsPerBin; //256
  constexpr int OneSweepNumDigits = Helpers::int_ceil_div(KeyBitSize, OneSweepBitsPerBin); //4

  static_assert(OneSweepNumDigits * OneSweepNumBinsPerDigit <= NMaxClusters);
  //So that the histogram_buffer can work...
  //(We could treat it as a larger array, we have space, but still.)

  static_assert(OneSweepNumDigits == ExpectedNumberOfSortingIterations);
  static_assert(OneSweepNumBinsPerDigit == ExpectedNumberOfBinsPerDigit);

  constexpr int HistogramNumThreadsPerBlock = 256;
  constexpr int HistogramItemsPerThread = 8;
  constexpr int HistogramItemsPerBlock = HistogramItemsPerThread * HistogramNumThreadsPerBlock; //2048

  constexpr int WarpSize = 32;

  constexpr int ReorderNumWarpsPerBlock = 8;
  constexpr int ReorderNumThreadsPerBlock = ReorderNumWarpsPerBlock * WarpSize; //256
  constexpr int ReorderItemsPerThread = 16;
  constexpr int ReorderItemsPerWarp = ReorderItemsPerThread * WarpSize; //512
  constexpr int ReorderItemsPerBlock = ReorderItemsPerWarp * ReorderNumWarpsPerBlock; //4096

  static_assert(ReorderItemsPerBlock == ExpectedNumberOfItemsPerBlock);

  __device__ uint32_t onesweep_scan(const int this_thread_index, uint32_t * shared_counters, const uint32_t prefix = 0)
  {
    static_assert(HistogramNumThreadsPerBlock >= OneSweepNumBinsPerDigit && 
                  ReorderNumThreadsPerBlock >= OneSweepNumBinsPerDigit       );
    
    constexpr int num_warps_total = Helpers::int_ceil_div(OneSweepNumBinsPerDigit, WarpSize);
    
    static_assert(num_warps_total <= WarpSize);
    
    __shared__ uint32_t shared_offsets[num_warps_total];
    
    constexpr unsigned int full_mask = 0xFFFFFFFFU;
    
    const int this_warp_index  = this_thread_index / WarpSize;
    const int intra_warp_index = this_thread_index % WarpSize;
    
    if (this_warp_index >= num_warps_total)
      {
        return 0;
        //Doesn't matter that we return 0 here,
        //as any possible use of the prefix for
        //chaining these scans will only use
        //it in the relevant warps...
      }

    uint32_t accum = (this_thread_index < OneSweepNumBinsPerDigit ? shared_counters[this_thread_index] : 0);
    
    for (int i = 1; i < WarpSize; i *= 2)
      {
        const uint32_t other = __shfl_up_sync(full_mask, accum, i) * (intra_warp_index >= i);
      
        accum += other;
      }
  
    if (intra_warp_index == WarpSize - 1)
      {
        shared_offsets[this_warp_index] = accum;
      }
  
    __syncthreads();
    
    if (this_warp_index == 0)
      {
        uint32_t global_accum = (intra_warp_index < num_warps_total ? shared_offsets[intra_warp_index] : 0);
        
        for (int i = 1; i < num_warps_total; i *= 2)
          {
            const uint32_t other = __shfl_up_sync(full_mask, global_accum, i) * (intra_warp_index >= i);
            
            global_accum += other;
          }
        
        if (intra_warp_index < num_warps_total)
          {
            shared_offsets[intra_warp_index] = global_accum;
          }
      }
  
    __syncthreads();
    
    if (this_thread_index < OneSweepNumBinsPerDigit)
      {
        shared_counters[this_thread_index] = (this_warp_index > 0 ? shared_offsets[this_warp_index - 1] : 0) +
                                             accum + prefix - shared_counters[this_thread_index];
      }
    
    return shared_offsets[num_warps_total - 1];
  }
  
  __device__ void onesweep_histogram(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int num_clusters)
  {
    __shared__ uint32_t shared_counters[OneSweepNumDigits][OneSweepNumBinsPerDigit];

    const int this_thread_index = threadIdx.x;
    const int this_block_index = blockIdx.x;

    for (int i = this_thread_index; i < OneSweepNumDigits * OneSweepNumBinsPerDigit; i += HistogramNumThreadsPerBlock)
      {
        shared_counters[i / OneSweepNumBinsPerDigit][i % OneSweepNumBinsPerDigit] = 0;
      }

    __syncthreads();
    
    for (int i = 0; i < HistogramItemsPerThread; ++i)
      {
        const int this_index = this_block_index  * HistogramItemsPerBlock  +
                               this_thread_index * HistogramItemsPerThread +
                               i;

        if (this_index < num_clusters)
          {
            const KeyType item = Temporaries::sorting_keys(clusters_arr, 0, this_index);
            for (int j = 0; j < OneSweepNumDigits; ++j)
              {
                constexpr KeyType bit_mask = (1 << OneSweepBitsPerBin) - 1; //0xFF
                const KeyType bits = (item >> (j * OneSweepBitsPerBin)) & bit_mask;

                atomicInc(&shared_counters[j][bits], 0xFFFFFFFFU);
                
                //Can we do something smart here to minimize atomic operations
                //by doing something per-warp?
              }
          }
      }

    __syncthreads();

    for (int i = 0; i < OneSweepNumDigits; ++i)
      {
        onesweep_scan(this_thread_index, &shared_counters[i][0]);
        __syncthreads();
      }

    for (int i = this_thread_index; i < OneSweepNumDigits * OneSweepNumBinsPerDigit; i += HistogramNumThreadsPerBlock)
      {
        const int this_digit = i / OneSweepNumBinsPerDigit;
        const int this_bin   = i % OneSweepNumBinsPerDigit;
        
        atomicAdd(&Temporaries::histogram_buffer(clusters_arr, this_digit * OneSweepNumBinsPerDigit + this_bin),
                  shared_counters[this_digit][this_bin]);
      }
  }
  
  __device__ void onesweep_sort(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int num_clusters, const int iteration)
  {
    __shared__ uint32_t shared_sum[OneSweepNumBinsPerDigit];

    constexpr int FirstPhaseSumSize = OneSweepNumBinsPerDigit * ReorderNumWarpsPerBlock;

    struct FirstPhase
    {
      uint32_t hist[OneSweepNumBinsPerDigit];
      uint32_t sum[FirstPhaseSumSize];
    };
    struct SecondPhase
    {
      KeyType elements[ReorderItemsPerBlock];
    };
    struct ThirdPhase
    {
      ValueType elements[ReorderItemsPerBlock];
      uint8_t buckets[ReorderItemsPerBlock];
    };

    union SharedMemory
    {
      FirstPhase  p1;
      SecondPhase p2;
      ThirdPhase  p3;
    };

    __shared__ SharedMemory shared_store;

    KeyType local_keys[ReorderItemsPerThread];
    uint32_t warp_offsets[ReorderItemsPerThread];

    const int this_block_index  = blockIdx.x;
    const int this_thread_index = threadIdx.x;
    const int this_warp_index   = this_thread_index / WarpSize;
    const int intra_warp_index  = this_thread_index % WarpSize;

    auto get_key_bits = [&](const KeyType key) -> KeyType
    {
      constexpr KeyType bit_mask = (1 << OneSweepBitsPerBin) - 1; //0xFF
      return ((key >> (iteration * OneSweepBitsPerBin)) & bit_mask);
    };

    for (int i = this_thread_index; i < FirstPhaseSumSize; i += ReorderNumThreadsPerBlock)
      {
        shared_store.p1.sum[i] = 0;
      }

    for (int i = 0, j = 0; i < ReorderItemsPerWarp; i += WarpSize, ++j)
      {
        const int this_index = this_block_index * ReorderItemsPerBlock + this_warp_index * ReorderItemsPerWarp + i + intra_warp_index;
        if (this_index < num_clusters)
          {
            local_keys[j] = Temporaries::sorting_keys(clusters_arr, iteration, this_index);
          }
        else
          {
            local_keys[j] = 0xFFFFFFFFU;
          }
      }
    
    __syncthreads();

    constexpr uint32_t full_mask = 0xFFFFFFFFU;

    for (int i = 0, j = 0; i < ReorderItemsPerWarp; i += WarpSize, ++j)
      {
        const int this_index = this_block_index * ReorderItemsPerBlock + this_warp_index * ReorderItemsPerWarp + i + intra_warp_index;
        const KeyType key_bits = get_key_bits(local_keys[j]);

        uint32_t thread_mask = __ballot_sync(full_mask, this_index < num_clusters);

        for (int k = 0; k < OneSweepBitsPerBin; ++k)
          {
            const uint32_t bit = (key_bits >> k) & 1U;

            const uint32_t diff = (full_mask * bit) ^ __ballot_sync(full_mask, bit != 0);

            thread_mask &= ~diff;
          }

        const uint32_t lower_mask = (1U << intra_warp_index) - 1;

        const uint32_t digit_count = shared_store.p1.sum[key_bits * ReorderNumWarpsPerBlock + this_warp_index];
        warp_offsets[j] = digit_count + __popc(thread_mask & lower_mask);

        __syncwarp(full_mask);

        const uint32_t leader = __ffs(thread_mask) - 1;

        if (intra_warp_index == leader)
          {
            shared_store.p1.sum[key_bits * ReorderNumWarpsPerBlock + this_warp_index] = digit_count + __popc(thread_mask);
          }

        __syncwarp(full_mask);
      }

    __syncthreads();
    
    for (int i = this_thread_index; i < OneSweepNumBinsPerDigit; i += ReorderNumThreadsPerBlock)
      {
        uint32_t sum = 0;

        for (int j = 0; j < ReorderNumWarpsPerBlock; ++j)
          {
            sum += shared_store.p1.sum[i * ReorderNumWarpsPerBlock + j];
          }

        shared_store.p1.hist[i] = sum;
        
        const int this_index = OneSweepNumBinsPerDigit * this_block_index + i;

        constexpr uint32_t local_ready_mask = 0x40000000U;
        constexpr uint32_t sum_ready_mask = 0x80000000U;
        constexpr uint32_t both_masks = (local_ready_mask | sum_ready_mask);

        Temporaries::onesweep_counters(clusters_arr, iteration, this_index) = sum | local_ready_mask;

        const uint32_t hist_entry = Temporaries::histogram_buffer(clusters_arr, iteration * OneSweepNumBinsPerDigit + i);

        uint32_t accum = 0;

        for (int other_block = this_block_index; other_block > 0; --other_block)
          {
            const int other_index = OneSweepNumBinsPerDigit * (other_block - 1) + i;

            volatile uint32_t *ptr = static_cast<volatile uint32_t *>(&Temporaries::onesweep_counters(clusters_arr, iteration, other_index));
            
            uint32_t block_result = 0;
            do
              {
                block_result = *ptr;
              }
            while ((block_result & both_masks) == 0);

            accum += (block_result & (~both_masks));

            if (block_result & sum_ready_mask)
              {
                break;
              }
          }

        Temporaries::onesweep_counters(clusters_arr, iteration, this_index) = (sum + accum) | sum_ready_mask;

        shared_sum[i] = hist_entry + accum;
      }

    __syncthreads();
    
    onesweep_scan(this_thread_index, &shared_store.p1.hist[0]);
    
    __syncthreads();
    
    for (int i = this_thread_index; i < OneSweepNumBinsPerDigit; i += ReorderNumThreadsPerBlock)
      {
        uint32_t sum = shared_store.p1.hist[i];

        shared_sum[i] -= sum;

        for (int w = 0; w < ReorderNumWarpsPerBlock; ++w)
          {
            const int this_index = i * ReorderNumWarpsPerBlock + w;
            const uint32_t prev = shared_store.p1.sum[this_index];
            shared_store.p1.sum[this_index] = sum;
            sum += prev;
          }
      }

    __syncthreads();
    
    for (int i = 0; i < ReorderItemsPerThread; ++i)
      {
        const KeyType key_bits = get_key_bits(local_keys[i]);
        warp_offsets[i] += shared_store.p1.sum[key_bits * ReorderNumWarpsPerBlock + this_warp_index];
      }

    __syncthreads();
    
    const bool is_last_iter = (iteration == OneSweepNumDigits - 1);

    if (!is_last_iter)
      //We don't need to update the keys at the last iteration.
      {
        for (int i = intra_warp_index, j = 0; i < ReorderItemsPerWarp; i += WarpSize, ++j)
          {
            const int this_index = this_block_index * ReorderItemsPerBlock + this_warp_index * ReorderItemsPerWarp + i;
            const KeyType key_bits = get_key_bits(local_keys[j]);

            if (this_index < num_clusters)
              {
                shared_store.p2.elements[warp_offsets[j]] = local_keys[j];
              }
          }

        __syncthreads();
    
        for (int i = this_thread_index; i < ReorderItemsPerBlock; i += ReorderNumThreadsPerBlock)
          {
            const int this_index = this_block_index * ReorderItemsPerBlock + i;

            if (this_index < num_clusters)
              {
                const KeyType this_key = shared_store.p2.elements[i];
                const KeyType key_bits = get_key_bits(this_key);

                const int dest_index = shared_sum[key_bits] + i;

                Temporaries::sorting_keys(clusters_arr, iteration + 1, dest_index) = this_key;
              }
          }

        __syncthreads();
      }
    
    for (int i = intra_warp_index, j = 0; i < ReorderItemsPerWarp; i += WarpSize, ++j)
      {
        const int this_index = this_block_index * ReorderItemsPerBlock + this_warp_index * ReorderItemsPerWarp + i;

        const KeyType key_bits = get_key_bits(local_keys[j]);

        if (this_index < num_clusters)
          {
            shared_store.p3.elements[warp_offsets[j]] = Temporaries::sorting_values(clusters_arr, iteration, this_index);
            shared_store.p3.buckets[warp_offsets[j]] = key_bits;
          }
      }

    __syncthreads();
    
    int real_num_clusters = num_clusters;

    if (is_last_iter)
      {
        real_num_clusters -= Temporaries::clusters_to_exclude(clusters_arr);
      }

    for (int i = this_thread_index; i < ReorderItemsPerBlock; i += ReorderNumThreadsPerBlock)
      {
        const int this_index = this_block_index * ReorderItemsPerBlock + i;

        if (this_index < num_clusters)
          {
            const ValueType this_value = shared_store.p3.elements[i];
            const int this_bucket = shared_store.p3.buckets[i];

            const int dest_index = shared_sum[this_bucket] + i;

            if (dest_index < real_num_clusters)
              {
                Temporaries::sorting_values(clusters_arr, iteration + 1, dest_index) = this_value;

                if (is_last_iter)
                  {
                    Temporaries::old_to_new_cluster_map(clusters_arr, this_value) = dest_index;
                  }
              }
          }
      }
  }

  //And now the local sort, which uses a single block.
  //This is most likely the case for the majority of events
  //we can realistically get, as N < 2000...

  constexpr int BitsPerLocalSort = 4;
  [[maybe_unused]] constexpr int NumTotalLocalSorts = Helpers::int_ceil_div(KeyBitSize, BitsPerLocalSort); //8
  constexpr int LocalSortNumBinsPerDigit = 1 << BitsPerLocalSort; //16

  constexpr int LocalSortingNumWarps            = 4;
  constexpr int LocalSortingNumThreads          = LocalSortingNumWarps * WarpSize; //128
  constexpr int LocalSortingItemsPerThread      = 32;
  constexpr int LocalSortingMaxNumber           = LocalSortingNumThreads * LocalSortingItemsPerThread; //4096
  constexpr int LocalSortingHistEntryMinNumBits = Helpers::int_ceil_log_2(LocalSortingMaxNumber - 1); //12
  constexpr int LocalSortingHistEntryNumBits    = 16;

  static_assert(LocalSortingHistEntryMinNumBits <= LocalSortingHistEntryNumBits);

  struct PackedLocalHistogram
  {
    using carrier_type = uint64_t;

    static constexpr unsigned int s_bits_per_carrier    = sizeof(carrier_type) * CHAR_BIT; //64
    static constexpr unsigned int s_entries_per_carrier = Helpers::int_floor_div(s_bits_per_carrier, LocalSortingHistEntryNumBits); //4
    
    static constexpr auto s_number_carriers = Helpers::int_ceil_div(LocalSortNumBinsPerDigit, s_entries_per_carrier); //4

    carrier_type carriers[s_number_carriers][LocalSortingNumThreads + 1];

   private:

    static constexpr carrier_type s_mask       = (static_cast<carrier_type>(1) << LocalSortingHistEntryNumBits) - 1;
    static constexpr unsigned int s_divider    = s_entries_per_carrier;
    static constexpr unsigned int s_multiplier = LocalSortingHistEntryNumBits;

   public:

    constexpr uint16_t get_value(const unsigned int thread, const unsigned int bin) const
    {
      const unsigned int index = bin / s_divider;
      const unsigned int shift = (bin % s_divider) * s_multiplier;
      
      return (carriers[index][thread] >> shift) & s_mask;
    }
    
    _Pragma("nv_diag_suppress 177")
    constexpr void set_value(const unsigned int thread, const unsigned int bin, const uint16_t value)
    {
      const unsigned int index = bin / s_divider;
      const unsigned int shift = (bin % s_divider) * s_multiplier;

      const carrier_type new_value = ((static_cast<carrier_type>(value) & s_mask) << shift);

      carriers[index][thread] = (carriers[index][thread] & ~(s_mask << shift)) | new_value;
    }
    _Pragma("nv_diag_default 177")
    //To avoid the warnings about the function not having been referenced.
    //Useful for debugging the algorithm.

    //Safe to be called simultaneously from multiple threads
    //if indeed the thread index is provided.
    constexpr void add_value(const unsigned int thread, const unsigned int bin, const uint16_t value)
    {
      const unsigned int index = bin / s_divider;
      const unsigned int shift = (bin % s_divider) * s_multiplier;

      const carrier_type new_value = ((static_cast<carrier_type>(value) & s_mask) << shift);
      
      carriers[index][thread] += new_value;
    }
  };

  __device__ void local_sort_bin_prefix_sum(const int this_thread_index, const int idx, PackedLocalHistogram & shared_hist, PackedLocalHistogram::carrier_type * shared_array)
  {  
    static_assert(LocalSortingNumWarps <= WarpSize);
  
    constexpr unsigned int full_mask = 0xFFFFFFFFU;
    
    const int this_warp_index  = this_thread_index / WarpSize;
    const int intra_warp_index = this_thread_index % WarpSize;
  
    PackedLocalHistogram::carrier_type accum = shared_hist.carriers[idx][this_thread_index];
    
    for (int i = 1; i < WarpSize; i *= 2)
      {
        const PackedLocalHistogram::carrier_type other = __shfl_up_sync(full_mask, accum, i) * (intra_warp_index >= i);
      
        accum += other;
      }
  
    if (intra_warp_index == WarpSize - 1)
      {
        shared_array[this_warp_index] = accum;
      }
  
    __syncthreads();
  
    if (this_warp_index == 0)
      {
        PackedLocalHistogram::carrier_type global_accum = (intra_warp_index < LocalSortingNumWarps ? shared_array[intra_warp_index] : 0);
    
        for (int i = 1; i < LocalSortingNumWarps; i *= 2)
          {
            const PackedLocalHistogram::carrier_type other = __shfl_up_sync(full_mask, global_accum, i) * (intra_warp_index >= i);
      
            global_accum += other;
          }
    
        if (intra_warp_index < LocalSortingNumWarps)
          {
            shared_array[intra_warp_index] = global_accum;
          }
      }
  
    __syncthreads();
  
    shared_hist.carriers[idx][this_thread_index] = (this_warp_index > 0 ? shared_array[this_warp_index - 1] : 0) + accum - shared_hist.carriers[idx][this_thread_index];
  }

  __device__ void local_sort_final_prefix_sum(const int this_thread_index, PackedLocalHistogram & shared_hist)
  { 
    static_assert(LocalSortNumBinsPerDigit <= WarpSize);
  
    constexpr unsigned int full_mask = 0xFFFFFFFFU;
    
    const int this_warp_index  = this_thread_index / WarpSize;
    const int intra_warp_index = this_thread_index % WarpSize;
  
    if (this_warp_index == 0)
      {
        unsigned int accum = (this_thread_index < LocalSortNumBinsPerDigit ? shared_hist.get_value(LocalSortingNumThreads, intra_warp_index) : 0);
    
        for (int i = 1; i < LocalSortNumBinsPerDigit; i *= 2)
          {
            const unsigned int other = __shfl_up_sync(full_mask, accum, i) * (intra_warp_index >= i);
        
            accum += other;
          }
    
        if (intra_warp_index < LocalSortNumBinsPerDigit)
          {
            constexpr unsigned int partial_mask = (1U << LocalSortNumBinsPerDigit) - 1;

            PackedLocalHistogram::carrier_type final_value = accum - shared_hist.get_value(LocalSortingNumThreads, intra_warp_index);

            for (int i = 1; i < PackedLocalHistogram::s_entries_per_carrier; i *= 2)
              {
                const PackedLocalHistogram::carrier_type other = __shfl_up_sync(partial_mask, final_value, i) * (intra_warp_index >= i);

                final_value = (final_value << (LocalSortingHistEntryNumBits * i)) | other;
              }

            const int this_carrier_index  = intra_warp_index / PackedLocalHistogram::s_entries_per_carrier;
            const int intra_carrier_index = intra_warp_index % PackedLocalHistogram::s_entries_per_carrier;
            
            if (intra_carrier_index == (PackedLocalHistogram::s_entries_per_carrier - 1))
              {
                shared_hist.carriers[this_carrier_index][LocalSortingNumThreads] = final_value;
                //A single 64 bit write to a 64 bit variable to prevent overwrites...
              }
          }
      }
  }
  
  __device__ void local_sort_inner(const int this_offset,
                                   const int this_thread_index,
                                   KeyType * local_keys,
                                   KeyType * shared_keys,
                                   ValueType * local_values,
                                   ValueType * shared_values,
                                   const int start_bit,
                                   const int real_items_per_thread)

  {
    __shared__ PackedLocalHistogram shared_hist;
    __shared__ PackedLocalHistogram::carrier_type shared_array[LocalSortingNumWarps];

    for (int i = 0; i < PackedLocalHistogram::s_number_carriers; ++i)
      {
        shared_hist.carriers[i][this_thread_index] = static_cast<PackedLocalHistogram::carrier_type>(0);
      }
    
    for (int i = 0; i < real_items_per_thread; ++i)
      {
        const unsigned int key_bits = (local_keys[i] >> start_bit) & ((1U << BitsPerLocalSort) - 1);

        shared_hist.add_value(this_thread_index, key_bits, 1);
      }
    
    for (int i = 0; i < PackedLocalHistogram::s_number_carriers; ++i)
      {
        local_sort_bin_prefix_sum(this_thread_index, i, shared_hist, shared_array);
        
        if (this_thread_index == LocalSortingNumThreads - 1)
          {
            shared_hist.carriers[i][LocalSortingNumThreads] = shared_array[LocalSortingNumWarps - 1];
          }
        
        __syncthreads();
      }

    local_sort_final_prefix_sum(this_thread_index, shared_hist);

    __syncthreads();
    
    for (int i = 0; i < real_items_per_thread; ++i)
      {
        const unsigned int key_bits = (local_keys[i] >> start_bit) & ((1U << BitsPerLocalSort) - 1);

        const uint16_t offset = shared_hist.get_value(LocalSortingNumThreads, key_bits);

        const uint16_t rank = shared_hist.get_value(this_thread_index, key_bits);

        shared_hist.add_value(this_thread_index, key_bits, 1);
        
        shared_keys  [offset + rank] = local_keys  [i];
        shared_values[offset + rank] = local_values[i];
      }
    
    __syncthreads();
    
    for (int i = 0; i < real_items_per_thread; ++i)
      {
        const int this_index = this_offset + i;
        local_keys  [i] = shared_keys  [this_index];
        local_values[i] = shared_values[this_index];
      }
    
  }

  __device__ void local_sort(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int cluster_number)
  {
    __shared__ KeyType   shared_keys  [LocalSortingMaxNumber];
    __shared__ ValueType shared_values[LocalSortingMaxNumber];

    KeyType   local_keys[LocalSortingItemsPerThread];
    ValueType local_values[LocalSortingItemsPerThread];

    const int real_items_per_thread = Helpers::int_ceil_div(cluster_number, LocalSortingNumThreads);
    //So we don't need to use multiple redundant items
    //that won't be properly sorted, even.

    const int this_thread_index = threadIdx.x;
    const int this_offset = this_thread_index * real_items_per_thread;

    for (int i = 0; i < real_items_per_thread; ++i)
      {
        const int this_index = this_offset + i;

        const KeyType   this_key   = (this_index < cluster_number ?
                                      Temporaries::sorting_keys(clusters_arr, 0, this_index) : 0xFFFFFFFFU);
        const ValueType this_value = (this_index < cluster_number ?
                                      Temporaries::sorting_values(clusters_arr, 0, this_index) : 0xFFFFU);

        local_keys  [i] = this_key;
        local_values[i] = this_value;

        shared_keys  [this_index] = this_key;
        shared_values[this_index] = this_value;
      }

    //Unroll?
    for (int start_bit = 0; start_bit < KeyBitSize; start_bit += BitsPerLocalSort)
      {
        local_sort_inner(this_offset, this_thread_index, local_keys, shared_keys, local_values, shared_values, start_bit, real_items_per_thread);
      }

    const int real_num_clusters =  cluster_number - Temporaries::clusters_to_exclude(clusters_arr);

    for (int i = 0; i < real_items_per_thread; ++i)
      {
        const int this_index = this_offset + i;

        if (this_index < real_num_clusters)
          {
            //Temporaries::sorting_keys(clusters_arr, ExpectedNumberOfSortingIterations, this_index) = local_keys[i];
            //We actually don't use the final keys for anything...

            const ValueType this_r = local_values[i];

            Temporaries::sorting_values(clusters_arr, ExpectedNumberOfSortingIterations, this_index) = this_r;

            Temporaries::old_to_new_cluster_map(clusters_arr, this_r) = this_index;
          }
      }
  }
}

constexpr int BuildHistogramBlockSize = (LocalSortingNumThreads > HistogramNumThreadsPerBlock ? LocalSortingNumThreads : HistogramNumThreadsPerBlock);
constexpr int BuildHistogramGridSize  = Helpers::int_ceil_div(NMaxClusters, HistogramItemsPerBlock);

__global__ static void buildHistogramKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
#if CALORECGPU_SORTER_FORCE_FULL_ONESWEEP_SORTING
  const int number_of_clusters = max(5000, clusters_arr->number);
#else
  const int number_of_clusters = clusters_arr->number;
#endif

  if (number_of_clusters <= LocalSortingMaxNumber)
    {
      if (threadIdx.x >= LocalSortingNumThreads)
        {
          return;
        }
      
      if (blockIdx.x == 0)
        {
          local_sort(clusters_arr, number_of_clusters);
        }
    }
  else if (blockIdx.x * HistogramItemsPerBlock < number_of_clusters)
    {
      if (threadIdx.x >= HistogramNumThreadsPerBlock)
        {
          return;
        }
      
      onesweep_histogram(clusters_arr, number_of_clusters);
    }
}

constexpr int SortClustersBlockSize = ReorderNumThreadsPerBlock;
constexpr int SortClustersGridSize  = Helpers::int_ceil_div(NMaxClusters, ReorderItemsPerBlock);

__global__ static void sortClustersKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int iteration)
{
#if CALORECGPU_SORTER_FORCE_FULL_ONESWEEP_SORTING
  const int number_of_clusters = max(5000, clusters_arr->number);
#else
  const int number_of_clusters = clusters_arr->number;
#endif
  
  if (number_of_clusters > LocalSortingMaxNumber && blockIdx.x * ReorderItemsPerBlock < number_of_clusters)
    {
      onesweep_sort(clusters_arr, number_of_clusters, iteration);
    }
}

//Possible TO-DO for future optimization:
//make block and grid size adaptable!

void GPUClusterSorting::sortClusters(CaloRecGPU::EventDataHolder & holder,
                                     const ConstantDataHolder & instance_data,
                                     const IGPUKernelSizeOptimizer & optimizer,
                                     const bool synchronize,
                                     CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  buildHistogramKernel <<< BuildHistogramGridSize, BuildHistogramBlockSize, 0, stream_to_use>>>(holder.m_clusters_dev);

  for (int i = 0; i < OneSweepNumDigits; ++i)
    {
      sortClustersKernel <<< SortClustersGridSize, SortClustersBlockSize, 0, stream_to_use>>>(holder.m_clusters_dev, i);
    }

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/**********************************************************************************/

constexpr int PrefixSumItemsPerThread  = 8;
constexpr int PrefixSumWarpsPerBlock = 16;
constexpr int PrefixSumThreadsPerBlock = PrefixSumWarpsPerBlock * WarpSize;
constexpr int PrefixSumItemsPerBlock   = PrefixSumItemsPerThread * PrefixSumThreadsPerBlock;
constexpr int PrefixSumGridSize = Helpers::int_ceil_div(NMaxClusters, PrefixSumItemsPerBlock);
//To be adjusted later...

__global__ static
void numCellsPrefixSumKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int this_block_index = blockIdx.x;
  const int this_thread_index = threadIdx.x;
  const int this_warp_index = this_thread_index / WarpSize;
  const int intra_warp_index = this_thread_index % WarpSize;

  __shared__ int shared_prefix;
  __shared__ int prefix_sum[PrefixSumWarpsPerBlock];

  const int cluster_number = clusters_arr->number;
  const int invalid_clusters = Temporaries::clusters_to_exclude(clusters_arr);
  const int real_num_clusters = cluster_number - invalid_clusters;

  int local_prefix[PrefixSumItemsPerThread];
  int local_seeds[PrefixSumItemsPerThread];

  int intra_thread_sum = 0;

  if (this_block_index == 0 && this_thread_index == 0 && real_num_clusters == 0)
    {
      clusters_arr->number_cells = 0;
    }

  if (this_block_index * PrefixSumItemsPerBlock >= real_num_clusters)
    {
      return;
      //Nothing to do...
    }

  const int real_items_per_thread = ( (this_block_index + 1) * PrefixSumItemsPerBlock < real_num_clusters ?
                                      PrefixSumItemsPerThread : Helpers::int_ceil_div(real_num_clusters % PrefixSumItemsPerBlock, PrefixSumThreadsPerBlock) );

  for (int i = 0; i < real_items_per_thread; ++i)
    {
      const int cluster_index = this_block_index * PrefixSumItemsPerBlock + this_thread_index * real_items_per_thread + i;

      if (cluster_index < real_num_clusters)
        {
          const int old_cluster_index = Temporaries::sorting_values(clusters_arr, ExpectedNumberOfSortingIterations, cluster_index);

          const int this_num_cells = Temporaries::num_cells_per_cluster(clusters_arr, old_cluster_index);

          intra_thread_sum += this_num_cells;
          local_prefix[i] = intra_thread_sum;

          local_seeds[i] = Temporaries::old_seed_cells(clusters_arr, old_cluster_index);

        }
      else
        {
          local_prefix[i] = 0;
          local_seeds[i] = -1;
        }
    }

  constexpr unsigned int full_mask = 0xFFFFFFFFU;

  int intra_warp_sum = intra_thread_sum;

  for (int i = 1; i < WarpSize; i *= 2)
    {
      const int other_sum = __shfl_up_sync(full_mask, intra_warp_sum, i) * (intra_warp_index >= i);

      intra_warp_sum += other_sum;
    }

  if (intra_warp_index == WarpSize - 1)
    {
      prefix_sum[this_warp_index] = intra_warp_sum;
    }

  __syncthreads();

  static_assert(PrefixSumWarpsPerBlock <= WarpSize);

  if (this_warp_index == 0)
    {
      int final_prefix_sum = (intra_warp_index < PrefixSumWarpsPerBlock ? prefix_sum[intra_warp_index] : 0);

      for (int i = 1; i < PrefixSumWarpsPerBlock; i *= 2)
        {
          const int other_sum = __shfl_up_sync(full_mask, final_prefix_sum, i) * (intra_warp_index >= i);

          final_prefix_sum += other_sum;
        }

      if (intra_warp_index < PrefixSumWarpsPerBlock)
        {
          prefix_sum[intra_warp_index] = final_prefix_sum;
        }

      if (intra_warp_index == PrefixSumWarpsPerBlock - 1)
        {
          constexpr uint32_t local_ready_mask = 0x40000000;
          constexpr uint32_t sum_ready_mask = 0x80000000;
          constexpr uint32_t both_masks = (local_ready_mask | sum_ready_mask);

          Temporaries::prefix_sum_counters(clusters_arr, this_block_index) = final_prefix_sum | local_ready_mask;

          uint32_t accum = 0;

          for (int other_block = this_block_index; other_block > 0; --other_block)
            {
              volatile uint32_t *ptr = static_cast<volatile uint32_t *>(&Temporaries::prefix_sum_counters(clusters_arr, other_block - 1));
                  
              uint32_t block_result = 0;
              do
                {
                  block_result = *ptr;
                }
              while ((block_result & both_masks) == 0);

              accum += (block_result & (~both_masks));

              if (block_result & sum_ready_mask)
                {
                  break;
                }
            }

          Temporaries::prefix_sum_counters(clusters_arr, this_block_index) = (final_prefix_sum + accum) | sum_ready_mask;

          shared_prefix = accum;
        }
    }

  __syncthreads();

  const int start_prefix = shared_prefix + (this_warp_index > 0 ? prefix_sum[this_warp_index - 1] : 0) + intra_warp_sum - intra_thread_sum;

  for (int i = 0; i < real_items_per_thread; ++i)
    {
      const int cluster_index = this_block_index * PrefixSumItemsPerBlock + this_thread_index * real_items_per_thread + i;

      if (cluster_index < real_num_clusters)
        {
          clusters_arr->seedCellIndex[cluster_index] = local_seeds[i];

          clusters_arr->cellsPrefixSum[cluster_index + 1] =  start_prefix + local_prefix[i];

          Temporaries::intermediate_cell_counters(clusters_arr, cluster_index) =  start_prefix + (i > 0 ?  local_prefix[i - 1] : 0) + 1;

          if (cluster_index + 1 == real_num_clusters)
            {
              clusters_arr->number_cells = start_prefix + local_prefix[i];
            }
        }
    }
}

__global__ static
void finalizeCellInformationKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int n_cells = Temporaries::old_num_cells(clusters_arr);

  auto add_cell_to_cluster = [&](const int cell, const int cluster, const int seed, const float weight)
  {
    if (cell == seed)
      {
        const int begin_cluster = clusters_arr->cellsPrefixSum[cluster];
        clusters_arr->cells.indices[begin_cluster] = cell;
        clusters_arr->cellWeights[begin_cluster] = weight;
        clusters_arr->clusterIndices[begin_cluster] = cluster;
      }
    else
      {
        const int position = atomicAdd(&Temporaries::intermediate_cell_counters(clusters_arr, cluster), 1);
        clusters_arr->cells.indices[position] = cell;
        clusters_arr->cellWeights[position] = weight;
        clusters_arr->clusterIndices[position] = cluster;
      }
  };

  for (int cell = index; cell < n_cells; cell += grid_size)
    {
      const ClusterTag tag = clusters_arr->get_extra_cell_info(cell);
      if (tag.is_part_of_cluster())
        {
          if (tag.is_shared_between_clusters())
            {
              const int old_first_cluster  = tag.cluster_index();
              const int old_second_cluster = tag.secondary_cluster_index();

              const int first_cluster  = Temporaries::old_to_new_cluster_map(clusters_arr, old_first_cluster);
              const int second_cluster = Temporaries::old_to_new_cluster_map(clusters_arr, old_second_cluster);

              const uint32_t weight_pattern = tag.secondary_cluster_weight();
              const float secondary_weight = __uint_as_float(weight_pattern);
              const float primary_weight = 1.0f - secondary_weight;

              const int first_seed  = Temporaries::old_seed_cells(clusters_arr, old_first_cluster);
              const int second_seed = Temporaries::old_seed_cells(clusters_arr, old_second_cluster);

              const bool first_valid = first_cluster >= 0 && first_seed >= 0;
              const bool second_valid = second_cluster >= 0 && second_seed >= 0;

              if (!first_valid && !second_valid)
                {
                  clusters_arr->get_extra_cell_info(cell) = ClusterTag:: make_invalid_tag();
                }
              else if (!first_valid)
                {
                  clusters_arr->get_extra_cell_info(cell) = ClusterTag::make_tag(second_cluster, __float_as_uint(primary_weight), 0xFFFFU);
                  add_cell_to_cluster(cell, second_cluster, second_seed, secondary_weight);
                }
              else if (!second_valid)
                {
                  clusters_arr->get_extra_cell_info(cell) = ClusterTag::make_tag(first_cluster, weight_pattern, 0xFFFFU);
                  add_cell_to_cluster(cell, first_cluster, first_seed, primary_weight);
                }
              else /*if (first_valid && second_valid)*/
                {
                  clusters_arr->get_extra_cell_info(cell) = ClusterTag::make_tag(first_cluster, weight_pattern, second_cluster);
                  add_cell_to_cluster(cell, first_cluster, first_seed, primary_weight);
                  add_cell_to_cluster(cell, second_cluster, second_seed, secondary_weight);
                  //Do nothing: the tag's already OK.
                }
            }
          else
            {
              const int old_cluster = tag.cluster_index();
              
              const int new_cluster = Temporaries::old_to_new_cluster_map(clusters_arr, old_cluster);

              const int new_seed    = Temporaries::old_seed_cells(clusters_arr, old_cluster);

              if (new_cluster >= 0 && new_seed >= 0)
                {
                  clusters_arr->get_extra_cell_info(cell) = ClusterTag::make_tag(new_cluster);
                  add_cell_to_cluster(cell, new_cluster, new_seed, 1.0f);
                }
              else
                {
                  clusters_arr->get_extra_cell_info(cell) = ClusterTag:: make_invalid_tag();
                }
            }
        }
    }

  if (index == 0)
    {
      clusters_arr->number -= Temporaries::clusters_to_exclude(clusters_arr);
      clusters_arr->state = ClusterInformationState::Full;
      clusters_arr->has_deleted_clusters = false;
    }
}

//Possible TO-DO for future optimization:
//make block and grid size adaptable!

void GPUClusterSorting::finalizeClusterAssignment(CaloRecGPU::EventDataHolder & holder,
                                                   const ConstantDataHolder & instance_data,
                                                   const IGPUKernelSizeOptimizer & optimizer,
                                                   const bool synchronize,
                                                   CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);
  
  numCellsPrefixSumKernel <<< PrefixSumGridSize, PrefixSumThreadsPerBlock, 0, stream_to_use>>>(holder.m_clusters_dev);

  const CUDAKernelLaunchConfiguration cfg_finalize = optimizer.get_launch_configuration("GPUClusterSorting", 3);
  
  finalizeCellInformationKernel <<< cfg_finalize.grid_x, cfg_finalize.block_x, 0, stream_to_use>>>(holder.m_clusters_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  if (holder.m_clusters.valid())
    {
      holder.m_clusters->state = ClusterInformationState::Full;
      holder.m_clusters->has_deleted_clusters = false;
    }
  
#if CALORECGPU_SORTER_DEBUG_CELL_ASSIGNMENT
  holder.m_clusters = holder.m_clusters_dev;

  for (int i = 0; i < holder.m_clusters->number; ++i)
    {
      std::sort(holder.m_clusters->cells.indices + holder.m_clusters->cellsPrefixSum[i], holder.m_clusters->cells.indices + holder.m_clusters->cellsPrefixSum[i + 1]);
    }

  for (int i = 0; i < holder.m_clusters->number_cells; ++i)
    {
      printf("%6d %6d %6d (%6d)\n", i, holder.m_clusters->cells.indices[i], holder.m_clusters->clusterIndices[i], holder.m_clusters->number);
    }
  std::cout << "---------------------------------------" << std::endl;
#endif
}

/*******************************************************************************************************************************/

void GPUClusterSorting::register_kernels(IGPUKernelSizeOptimizer & optimizer)
{
  void * kernels[] = { (void *) initializeCountersKernel,
                       (void *) firstCellIterationKernel,
                       (void *) calculateInfoAndPrepareSort,
                       (void *) finalizeCellInformationKernel
                     };

  int blocksizes[] = { InitializeCountersBlockSize,
                       FirstCellIterationBlockSize,
                       CalcAndPrepSortingBlockSize,
                       FinalizeCellInfoBlockSize
                     };

  int  gridsizes[] = { Helpers::int_ceil_div(NMaxClusters, InitializeCountersBlockSize),
                       Helpers::int_ceil_div(NCaloCells, FirstCellIterationBlockSize),
                       Helpers::int_ceil_div(NMaxClusters, CalcAndPrepSortingBlockSize),
                       Helpers::int_ceil_div(NCaloCells, FinalizeCellInfoBlockSize)
                     };

  int   maxsizes[] = { NMaxClusters,
                       NCaloCells,
                       NMaxClusters,
                       NCaloCells
                     };

  optimizer.register_kernels("GPUClusterSorting", 4, kernels, blocksizes, gridsizes, maxsizes);
}
