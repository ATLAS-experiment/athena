//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

//NOTE: at several points of this implementation file,
//      some commented out appears here and there
//      to invalidate some clusters (seedCellID[clusters] = -1)
//      and clean them up. This is useful for debugging
//      the moments, by excluding clusters that may take
//      different choices than the CPU when there is a cutoff.

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "GPUClusterInfoAndMomentsCalculatorImpl.h"
#include "FPHelpers.h"
#include "GPUClusterInfoAndMomentsCalculatorImplHelper.h"

#include <cmath>
#include <cstdint>

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"
//Not ideal, but we have to measure inside the function
//for simplicity's sake...


#ifndef CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
  #define CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION 0
  //From our testing so far, having everything in the same block
  //is actually faster, but this can change depending on the GPU model.
  //Something to be optimized specifically later...
#endif

using namespace CaloRecGPU;
using namespace ClusterMomentsCalculator;

void ClusterMomentsCalculator::CMCOptionsHolder::sendToGPU(const bool clear_CPU)
{
  m_options_dev = m_options;
  if (clear_CPU)
    {
      m_options.clear();
    }
}

static_assert(NumSamplings <= 28, "We wrote the code under the assumption of 28 samplings at most.");

constexpr static int WarpSize = 32;

namespace
{
  namespace ToCalculate
  {
    using FirstPassInitialization = TypeList <
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling>,
                                    TypeList<EtaPerSample, PhiPerSample, AbsoluteEnergyPerSample, NCellSampling, EngPosAndEngFracCore>,
                                    //EngFracCore must be at index NumSamplings - 1
                                    TypeList<CenterX, CenterY, CenterZ, FirstEngDens, SecondEngDens>,
                                    TypeList<MX, EnergyDensityNormalization, ClusterEnergyEtaAndEt>,
                                    TypeList<MY, SumAbsEnergyNonMoments, FirstEta, FirstAndSecondMaxEnergyAndCell>,
                                    TypeList<MZ, ClusterPhi, SeedCellPhi, FirstPhi, EngFracEM>
                                    >;
    //This one is spread across a warp, courtesy of the zeroth pass.

    using FirstPassCells = TypeList <
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
  TypeList<NCellSampling, FirstAndSecondMaxEnergyAndCell>,
  TypeList<CenterX, MX>,
  TypeList<CenterY, MY>,
  TypeList<CenterZ, MZ>,
  TypeList<EngFracEM, FirstEngDens, SecondEngDens, EnergyDensityNormalization>,
  TypeList<AbsoluteEnergyPerSample, SumAbsEnergyNonMoments>,
  TypeList<ClusterEnergyEtaAndEt, EtaPerSample, FirstEta>,
  TypeList<ClusterPhi, PhiPerSample, FirstPhi>
#else
  TypeList <
  NCellSampling, FirstAndSecondMaxEnergyAndCell,
  CenterX, CenterY, CenterZ, MX, MY, MZ,
  EngFracEM, FirstEngDens
  >,
  TypeList <
  SecondEngDens, EnergyDensityNormalization,
  AbsoluteEnergyPerSample, SumAbsEnergyNonMoments,
  ClusterEnergyEtaAndEt, EtaPerSample, FirstEta,
  ClusterPhi, PhiPerSample, FirstPhi
  >
#endif
                           >;

    using FirstPassFinalization = TypeList <
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EtaPerSample, PhiPerSample>,
                                  TypeList<EngFracEM, Mass>,
                                  TypeList<CenterX, CenterY, CenterZ, FirstEngDens, SecondEngDens>,
                                  TypeList<ClusterEnergyEtaAndEt, ClusterPhi, FirstPhi, FirstEta>,
                                  TypeList<EngFracMax, MaxAndSecondMaxCells>
                                  >;

    using SecondPassInitialization = TypeList <
                                     TypeList<>,
                                     TypeList<NBadCells>,
                                     TypeList<NBadCellsCorr>,
                                     TypeList<BadCellsCorrE>,
                                     TypeList<BadLArQFrac>,
                                     TypeList<AvgLArQ>,
                                     TypeList<AvgTileQ>,
                                     TypeList<>,
                                     TypeList<>,
                                     TypeList<NumPositiveEnergyCells>,
                                     TypeList<SumSquareEnergies>,
                                     TypeList<>,
                                     TypeList<Matrix10>,
                                     TypeList<Matrix20>,
                                     TypeList<Matrix21>,
                                     TypeList<>,
                                     TypeList<>,
                                     TypeList<TimeAndSecondTime>,
                                     TypeList<>,
                                     TypeList<TimeNormalization>,
                                     TypeList<AverageLArQNormalization>,
                                     TypeList<AverageTileQNormalization>,
                                     TypeList<EngBadCells>,
                                     TypeList<>,
                                     TypeList<>,
                                     TypeList<>,
                                     TypeList<>,
                                     TypeList<Significance>,
                                     TypeList<PTD>,
                                     TypeList<Matrix00, Matrix11, Matrix22>,
                                     TypeList<>,
                                     TypeList<MaxSignificanceAndSampling>
                                     >;

    //MaxSignificanceAndSampling and Max and Second Max Cells must be in the same slot!
    //Matrix{00, 11, 22} and center{X,Y,Z} must be in the same respective slots!
    //PTD and Mass must be in the same slot!

    using SecondPassCells = TypeList <
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
  TypeList<EngBadCells, NBadCells>,
  TypeList<BadCellsCorrE, NBadCellsCorr>,
  TypeList<BadLArQFrac>,
  TypeList<AvgLArQ, AverageLArQNormalization>,
  TypeList<AvgTileQ, AverageTileQNormalization>,
  TypeList<PTD, NumPositiveEnergyCells>,
  TypeList<Matrix00>,
  TypeList<Matrix10>,
  TypeList<Matrix20>,
  TypeList<Matrix11>,
  TypeList<Matrix21>,
  TypeList<Matrix22>,
  TypeList<SumSquareEnergies>,
  TypeList<TimeAndSecondTime, TimeNormalization>,
  TypeList<Significance>,
  TypeList<MaxSignificanceAndSampling>
#else
  TypeList <
  EngBadCells, NBadCells, BadCellsCorrE, NBadCellsCorr,
  BadLArQFrac, AvgLArQ, AverageLArQNormalization,
  AvgTileQ, AverageTileQNormalization,
  PTD, NumPositiveEnergyCells,
  Matrix00,
  Matrix10, Matrix11,
  Matrix20, Matrix21, Matrix22,
  SumSquareEnergies, TimeAndSecondTime, TimeNormalization,
  Significance, MaxSignificanceAndSampling
  >
#endif
                            >;

    /* Shower axis pass */

    using SecondPassFinalization = TypeList <
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList</* for the per sampling init */>,
                                   TypeList<EngBadCells, BadCellsCorrE, BadLArQFrac, Significance>,
                                   TypeList<CellSignificance, CellSigSampling>,
                                   TypeList<AvgLArQ, AvgTileQ>,
                                   TypeList<PTD, TimeAndSecondTime>
                                   >;

    using ThirdPassInitialization = TypeList <
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<EnergyPerSample, MaxEnergyAndCellPerSample>,
                                    TypeList<LateralNormalization, NumCells>,
                                    TypeList<SecondR, LongitudinalNormalization>,
                                    TypeList<SecondLambda, Lateral>,
                                    TypeList<Longitudinal, NExtraCellSampling>
                                    >;

    using ThirdPassCells = TypeList <
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
  TypeList<EnergyPerSample>,
  TypeList<SecondR>,
  TypeList<SecondLambda>,
  TypeList<NumCells>,
  TypeList<NExtraCellSampling>,
  TypeList<Lateral>,
  TypeList<LateralNormalization>,
  TypeList<Longitudinal>,
  TypeList<LongitudinalNormalization>,
  TypeList<MaxEnergyAndCellPerSample>
#else
  TypeList <
  EnergyPerSample, SecondR, SecondLambda, NumCells, NExtraCellSampling,
  Lateral, LateralNormalization, Longitudinal, LongitudinalNormalization, MaxEnergyAndCellPerSample
  >
#endif
                           >;

    using ThirdPassFinalization = TypeList <
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<EnergyPerSample>,
                                  TypeList<Lateral>,
                                  TypeList<Longitudinal>,
                                  TypeList<SecondR, SecondLambda>,
                                  >;

    using FinalPassInitialization = TypeList <
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>,
                                    TypeList<MaxECellPerSample>
                                    >;
    //MaxECellPerSample and EnergyPerSample (for a given sampling) must be in the same slot!

  }

  constexpr static int ClusterPassBlockSize = 1024;
  constexpr static int CellPassBlockSize = 1024;
  //Maximize throughput?
  //Needs measurements, perhaps...
  //Also we could split all the sub-kernels
  //to have different block sizes.


  template <unsigned int num_moments_1, unsigned int num_moments_2>
  constexpr __host__ __device__ unsigned int cluster_pass_grid_size()
  {
    constexpr unsigned int num_moments = (num_moments_1 > num_moments_2 ? num_moments_1 : num_moments_2);

    return Helpers::int_ceil_div(NMaxClusters, ClusterPassBlockSize) * num_moments;
  }

  struct ClusterPassIndexing
  {
    int cluster;
    int delta_cluster;
    int moment;
  };

  __device__ ClusterPassIndexing get_cluster_pass_indexing()
  {
    const unsigned int cluster_index = blockIdx.x * blockDim.x + threadIdx.x;
    const unsigned int grid_size = gridDim.x * blockDim.x;

    return ClusterPassIndexing{static_cast<int>(cluster_index),
                               static_cast<int>(grid_size),
                               static_cast<int>(blockIdx.y)};
  }

  template <unsigned int num_moments>
  constexpr __host__ __device__ unsigned int cell_pass_grid_size()
  {
    return Helpers::int_ceil_div(NCaloCells, CellPassBlockSize) * num_moments * 2;
  }

  struct CellPassIndexing
  {
    int  cell;
    int  delta_cell;
    int  moment;
    bool use_second_cell;
  };

  __device__ CellPassIndexing get_cell_pass_indexing()
  {
    const unsigned int cell_index = blockIdx.x * blockDim.x + threadIdx.x;
    const unsigned int grid_size = gridDim.x * blockDim.x;

    return CellPassIndexing{static_cast<int>(cell_index),
                            static_cast<int>(grid_size),
                            static_cast<int>(blockIdx.y),
                            static_cast<bool>(blockIdx.z)};
  }

}

/******************************************************************************
 * "Zeroth" Pass: Isolation + other initialization                            *
 ******************************************************************************/

constexpr static int IsolationMemsetChunk = 1;

__global__ static
void isolationClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                                const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int cluster_number = clusters_arr->number;

  const int grid_size = gridDim.x * blockDim.x;
  const int start_index   = (blockIdx.x * blockDim.x + threadIdx.x) * IsolationMemsetChunk;

  for (int index = start_index; index + IsolationMemsetChunk <= cluster_number; index += grid_size)
    {
      for (int j = 0; j < IsolationMemsetChunk; ++j)
        {
          const int cluster_index = index + j;
          moments_arr->energyPerSample                          [blockIdx.y][cluster_index] = 0.f;
          CMCTemporaries::energyPerSampleAux       (moments_arr, blockIdx.y, cluster_index) = 0.f;
          CMCTemporaries::numberNonEmptySamplings  (moments_arr, blockIdx.y, cluster_index) = 0;
          CMCTemporaries::numberEmptySamplings     (moments_arr, blockIdx.y, cluster_index) = 0;
          CMCTemporaries::maxMomentsEnergyPerSample(moments_arr, blockIdx.y, cluster_index) = 0;
          if (blockIdx.y == 0)
          {
            moments_arr->engPos[cluster_index] = 0.f;
          }
          if (blockIdx.y == 1)
          {
            CMCTemporaries::engPosAux(moments_arr, cluster_index) = 0.f;
          }
        }
    }
}

__global__ static
void isolationCellPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                             const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                             const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                             const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                             const bool use_abs_energy)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
  if (blockIdx.y == 0)
#endif

    {

      for (int cell = index; cell < NCaloCells; cell += grid_size)
        {
          const int sampling   = geometry->sampling(cell);
          const ClusterTag tag = cell_state_arr->clusterTag[cell];

          const int this_cluster = (tag.is_part_of_cluster() ? tag.cluster_index() : -1);

          int * array_to_add = (this_cluster >= 0 ?
                                CMCTemporaries::numberNonEmptySamplings(moments_arr, sampling) :
                                CMCTemporaries::numberEmptySamplings(moments_arr, sampling));

          int neighbours[NMaxAll2DNeighbours];

          const int num_total_neighs = geometry->get_neighbours(LArNeighbours::neighbourOption::all2D, cell, neighbours);

          int num_useful_neighs = 0;

          for (int neigh = 0; neigh < num_total_neighs; ++neigh)
            {
              const int neigh_ID = neighbours[neigh];

              const ClusterTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];

              const int neigh_cluster = (neigh_tag.is_part_of_cluster() ? neigh_tag.cluster_index() : -1);

              if (neigh_cluster >= 0 && neigh_cluster != this_cluster)
                {
                  bool should_add = true;
                  for (int i = 0; i < num_useful_neighs; ++i)
                    {
                      if (neighbours[i] == neigh_cluster)
                        {
                          should_add = false;
                          break;
                        }
                    }
                  if (should_add)
                    {
                      neighbours[num_useful_neighs] = neigh_cluster;
                      ++num_useful_neighs;
                      atomicAdd(array_to_add + neigh_cluster, 1);
                    }
                }
            }

          //Possibly a broadcast step here instead to harmonize across the warp
          //and reduce the number of atomicAdds to be done?
          
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
        }
    }
  else if (blockIdx.y == 1 || blockIdx.y == 2)
    {
      for (int cell = index; cell < NCaloCells; cell += grid_size)
        {
          const ClusterTag tag = cell_state_arr->clusterTag[cell];
#endif

          if (tag.is_part_of_cluster())
            {
              
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
              const int   sampling         = geometry->sampling(cell);
#endif

              const float energy           = cell_info_arr->energy[cell];
              const float abs_energy       = fabsf(energy);
              const float moments_energy   = ((use_abs_energy || energy > 0.f) ? abs_energy : 0.f);
              const float secondary_weight = __uint_as_float(tag.secondary_cluster_weight());
              const float primary_weight   = 1.f - secondary_weight;

#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
              if (blockIdx.y == 1)
#endif

                {
                  add_with_corr(moments_arr->energyPerSample[sampling],
                                CMCTemporaries::energyPerSampleAux(moments_arr, sampling),
                                tag.cluster_index(), energy * primary_weight);

                  atomicMax(&(CMCTemporaries::maxMomentsEnergyPerSample(moments_arr, sampling, tag.cluster_index())),
                            __float_as_uint(moments_energy * primary_weight));
                            
                  add_with_corr(moments_arr->engPos,
                                CMCTemporaries::engPosAux(moments_arr),
                                tag.cluster_index(), moments_energy * primary_weight);
                }
                
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
              else if (blockIdx.y == 2 && tag.is_shared_between_clusters())
#else
              if (tag.is_shared_between_clusters())
#endif

                {
                  add_with_corr(moments_arr->energyPerSample[sampling],
                                CMCTemporaries::energyPerSampleAux(moments_arr, sampling),
                                tag.secondary_cluster_index(), energy * secondary_weight);

                  atomicMax(&(CMCTemporaries::maxMomentsEnergyPerSample(moments_arr, sampling, tag.secondary_cluster_index())),
                            __float_as_uint(moments_energy * secondary_weight));
                            
                  add_with_corr(moments_arr->engPos,
                                CMCTemporaries::engPosAux(moments_arr),
                                tag.secondary_cluster_index(), moments_energy * secondary_weight);
                }
            }
        }
    }
}

__global__ static
void zerothClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                             Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                             const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                             const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                             const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                             const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                             const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;

  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int moment  = threadIdx.x % WarpSize;
  const int grid_size = gridDim.x * blockDim.x;

  Parameters p {moment, moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cluster = index / WarpSize; cluster < cluster_number; cluster += grid_size / WarpSize)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }

      if (moment < NumSamplings)
        {
          const int sampling = moment;
          const float sampling_energy = moments_arr->energyPerSample[sampling][cluster] + CMCTemporaries::energyPerSampleAux(moments_arr, sampling, cluster);
          const unsigned int max_energy_pattern = CMCTemporaries::maxMomentsEnergyPerSample(moments_arr, sampling, cluster);
          const float sampling_max_energy = __uint_as_float(max_energy_pattern);
          const int sampling_empty = CMCTemporaries::numberEmptySamplings(moments_arr, sampling, cluster);
          const int sampling_non_empty = CMCTemporaries::numberNonEmptySamplings(moments_arr, sampling, cluster);

          const int total = sampling_empty + sampling_non_empty;

          float isolation      = 0.f, isolation_norm      = 0.f, eng_frac_core      = sampling_max_energy;
          float isolation_corr = 0.f, isolation_norm_corr = 0.f, eng_frac_core_corr = 0.f;

          if (total > 0 && sampling_energy > 0.f)
            {
              isolation = (sampling_energy * sampling_empty) / total;
              isolation_norm = sampling_energy;
            }

          const unsigned int mask = 0x0FFFFFFFU;
          //28 samplings, so without the last 4 threads.

          for (int i = 1; i < WarpSize; i *= 2)
            {
              const int origin = sampling ^ i;

              const float other_isol      = __shfl_xor_sync(mask, isolation,           i) * (origin < NumSamplings);
              const float other_isol_corr = __shfl_xor_sync(mask, isolation_corr,      i) * (origin < NumSamplings);
              const float other_norm      = __shfl_xor_sync(mask, isolation_norm,      i) * (origin < NumSamplings);
              const float other_norm_corr = __shfl_xor_sync(mask, isolation_norm_corr, i) * (origin < NumSamplings);
              const float other_enfc      = __shfl_xor_sync(mask, eng_frac_core,       i) * (origin < NumSamplings);
              const float other_enfc_corr = __shfl_xor_sync(mask, eng_frac_core_corr,  i) * (origin < NumSamplings);

              partial_kahan_babushka_neumaier_sum(other_isol + other_isol_corr,      isolation,      isolation_corr);
              partial_kahan_babushka_neumaier_sum(other_norm + other_norm_corr, isolation_norm, isolation_norm_corr);
              partial_kahan_babushka_neumaier_sum(other_enfc + other_enfc_corr,  eng_frac_core,  eng_frac_core_corr);
            }



          switch (moment)
            {
              case 0:
                {
                  const float real_iso  = isolation      + isolation_corr;
                  const float real_norm = isolation_norm + isolation_norm_corr;
                  moments_arr->isolation[cluster] = (real_norm != 0.f ? real_iso / real_norm : 0.f);
                }
                break;
              case NumSamplings - 1:
                moments_arr->engFracCore[cluster] = eng_frac_core + eng_frac_core_corr;
                break;
              default:
                break;
            }
        }
      __syncwarp();

      do_cluster_pass(TypeList<> {}, ToCalculate::FirstPassInitialization{}, cluster, p);
    }
}

/******************************************************************************
 * First Pass                                                                 *
 ******************************************************************************/

__global__ static
void firstCellPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                         Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                         const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                         const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                         const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                         const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                         const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr)
{
  const auto indexing = get_cell_pass_indexing();

  Parameters p {indexing.moment, moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cell = indexing.cell; cell < NCaloCells; cell += indexing.delta_cell)
    {
      const ClusterTag tag = cell_state_arr->clusterTag[cell];
      const float secondary_weight = __uint_as_float(tag.secondary_cluster_weight());

      if (tag.is_part_of_cluster())
        {
          if (indexing.use_second_cell && tag.is_shared_between_clusters())
            {
              do_cell_pass(ToCalculate::FirstPassCells{}, cell, tag.secondary_cluster_index(), secondary_weight, p);
            }
          else if (!indexing.use_second_cell)
            {
              do_cell_pass(ToCalculate::FirstPassCells{}, cell, tag.cluster_index(), 1.0f - secondary_weight, p);
            }
        }
    }
}

__global__ static
void firstClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                            Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                            const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                            const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;

  const auto indexing = get_cluster_pass_indexing();

  Parameters p {indexing.moment, moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cluster = indexing.cluster; cluster < cluster_number; cluster += indexing.delta_cluster)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }
      do_cluster_pass(ToCalculate::FirstPassFinalization{}, ToCalculate::SecondPassInitialization{}, cluster, p);
    }
}

/******************************************************************************
 * Second pass.                                                               *
 ******************************************************************************/

__global__ static
void secondCellPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                          Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                          const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                          const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                          const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                          const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr)
{
  const auto indexing = get_cell_pass_indexing();

  Parameters p {indexing.moment, moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cell = indexing.cell; cell < NCaloCells; cell += indexing.delta_cell)
    {
      const ClusterTag tag = cell_state_arr->clusterTag[cell];
      const float secondary_weight = __uint_as_float(tag.secondary_cluster_weight());
      if (tag.is_part_of_cluster())
        {
          if (indexing.use_second_cell && tag.is_shared_between_clusters())
            {
              do_cell_pass(ToCalculate::SecondPassCells{}, cell, tag.secondary_cluster_index(), secondary_weight, p);
            }
          else if (!indexing.use_second_cell)
            {
              do_cell_pass(ToCalculate::SecondPassCells{}, cell, tag.cluster_index(), 1.0f - secondary_weight, p);
            }
        }
    }
}

__global__ static
void showerAxisPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                          Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          const float max_axis_angle, const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;
  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cluster = index; cluster < cluster_number; cluster += grid_size)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }

      const float center_x   = moments_arr->centerX[cluster];
      const float center_y   = moments_arr->centerY[cluster];
      const float center_z   = moments_arr->centerZ[cluster];
      const float center_mag_inv = rnorm3df(center_x, center_y, center_z);
      moments_arr->centerMag[cluster] = 1.0f / center_mag_inv;
      float axis_x = center_x * center_mag_inv;
      float axis_y = center_y * center_mag_inv;
      float axis_z = center_z * center_mag_inv;
      float delta_phi = 0, delta_theta = 0, delta_alpha = 0;

      if (CMCTemporaries::numPositiveEnergyCells(moments_arr, cluster) > 2)
        {
          const float norm = 1.f / (CMCTemporaries::sumSquareEnergies(moments_arr, cluster) +
                                    CMCTemporaries::sumSquareEnergiesAux(moments_arr, cluster));
                                    
          RealSymmetricMatrixSolverIterative solver { (CMCTemporaries::matrix00(moments_arr, cluster) +
                                                       CMCTemporaries::matrix00Aux(moments_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix11(moments_arr, cluster) +
                                                       CMCTemporaries::matrix11Aux(moments_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix22(moments_arr, cluster) +
                                                       CMCTemporaries::matrix22Aux(moments_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix10(moments_arr, cluster) +
                                                       CMCTemporaries::matrix10Aux(moments_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix21(moments_arr, cluster) +
                                                       CMCTemporaries::matrix21Aux(moments_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix20(moments_arr, cluster) +
                                                       CMCTemporaries::matrix20Aux(moments_arr, cluster)) * norm  };

          float lambdas[3], vecs[3][3];

          solver.get_solution(lambdas, vecs);

          constexpr float min_lambdas = 1.e-6f;
          
          if (fabsf(lambdas[0]) >= min_lambdas && fabsf(lambdas[1]) >= min_lambdas && fabsf(lambdas[2]) >= min_lambdas)
            {
              int chosen_vec = -1;
              float prev_angle = 9e99;
              
              const float prev_norm = norm3df(axis_x, axis_y, axis_z);
              
              for (int i = 0; i < 3; ++i)
                {
                  //Following from Kahan and https://math.stackexchange.com/a/1782769,
                  //let's try a better angular difference...
                  const float this_norm = norm3df(vecs[i][0], vecs[i][1], vecs[i][2]);
                  
                  const float d1 = norm3df( product_sum_cornea_harrison_tang(prev_norm, vecs[i][0], -this_norm, axis_x),
                                            product_sum_cornea_harrison_tang(prev_norm, vecs[i][1], -this_norm, axis_y),
                                            product_sum_cornea_harrison_tang(prev_norm, vecs[i][2], -this_norm, axis_z) );
                                            
                  const float d2 = norm3df( product_sum_cornea_harrison_tang(prev_norm, vecs[i][0], this_norm, axis_x),
                                            product_sum_cornea_harrison_tang(prev_norm, vecs[i][1], this_norm, axis_y),
                                            product_sum_cornea_harrison_tang(prev_norm, vecs[i][2], this_norm, axis_z) );
                  
                  float raw_angle = 2 * atan2f(d1, d2);
                  
                  bool reverse = raw_angle > (Helpers::Constants::pi<float> / 2);

                  if (reverse)
                    {
                      raw_angle = Helpers::Constants::pi<float> - raw_angle;
                    }

                  if (chosen_vec == -1 || prev_angle > raw_angle)
                    {
                      chosen_vec = i;
                      prev_angle = raw_angle;
                      if (reverse)
                        {
                          vecs[i][0] *= -1;
                          vecs[i][1] *= -1;
                          vecs[i][2] *= -1;
                        }
                    }
                }

              auto calc_phi = [](const float x, const float y, const float z)
              {
                return atan2f(y, x);
              };
              auto calc_theta = [](const float x, const float y, const float z)
              {
                return atan2f(1.0f, z * rhypotf(x, y));
              };

              delta_alpha = prev_angle;

              delta_phi = Helpers::angular_difference(calc_phi(axis_x, axis_y, axis_z), calc_phi(vecs[chosen_vec][0], vecs[chosen_vec][1], vecs[chosen_vec][2]));

              delta_theta = calc_theta(axis_x, axis_y, axis_z) - calc_theta(vecs[chosen_vec][0], vecs[chosen_vec][1], vecs[chosen_vec][2]);


              if (prev_angle < max_axis_angle)
                {
                  axis_x = vecs[chosen_vec][0];
                  axis_y = vecs[chosen_vec][1];
                  axis_z = vecs[chosen_vec][2];
                }
              else
                {
                  //clusters_arr->seedCellID[cluster] = -1;
                }
            }
          else
            {
              //clusters_arr->seedCellID[cluster] = -1;
            }
        }
      CMCTemporaries::showerAxisX(moments_arr, cluster) = axis_x;
      CMCTemporaries::showerAxisY(moments_arr, cluster) = axis_y;
      CMCTemporaries::showerAxisZ(moments_arr, cluster) = axis_z;
      moments_arr->deltaPhi[cluster]   = delta_phi;
      moments_arr->deltaTheta[cluster] = delta_theta;
      moments_arr->deltaAlpha[cluster] = delta_alpha;
    }
}

__global__ static
void secondClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                             Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                             const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                             const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                             const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                             const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                             const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;

  const auto indexing = get_cluster_pass_indexing();

  Parameters p {indexing.moment, moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cluster = indexing.cluster; cluster < cluster_number; cluster += indexing.delta_cluster)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }

      do_cluster_pass(ToCalculate::SecondPassFinalization{}, ToCalculate::ThirdPassInitialization{}, cluster, p);
    }
}

/******************************************************************************
 * Third pass.                                                                *
 ******************************************************************************/

__global__ static
void thirdCellPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                         Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                         const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                         const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                         const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                         const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                         const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr)
{
  const auto indexing = get_cell_pass_indexing();

  Parameters p {indexing.moment, moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cell = indexing.cell; cell < NCaloCells; cell += indexing.delta_cell)
    {
      const ClusterTag tag = cell_state_arr->clusterTag[cell];
      const float secondary_weight = __uint_as_float(tag.secondary_cluster_weight());
      if (tag.is_part_of_cluster())
        {
          if (indexing.use_second_cell && tag.is_shared_between_clusters())
            {
              do_cell_pass(ToCalculate::ThirdPassCells{}, cell, tag.secondary_cluster_index(), secondary_weight, p);
            }
          else if (!indexing.use_second_cell)
            {
              do_cell_pass(ToCalculate::ThirdPassCells{}, cell, tag.cluster_index(), 1.0f - secondary_weight, p);
            }
        }
    }
}

__global__ static
void thirdClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                            Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                            const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                            const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;

  const auto indexing = get_cluster_pass_indexing();

  Parameters p {indexing.moment, moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cluster = indexing.cluster; cluster < cluster_number; cluster += indexing.delta_cluster)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }

      do_cluster_pass(ToCalculate::ThirdPassFinalization{}, ToCalculate::FinalPassInitialization{}, cluster, p);
    }
}

/******************************************************************************
 * Final pass: max cell per sample things, center lambda & cleanup            *
 ******************************************************************************/

__global__ static
void finalClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                            Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                            const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;

  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int thread_index  = threadIdx.x % WarpSize;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cluster = index / WarpSize; cluster < cluster_number; cluster += grid_size / WarpSize)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }
      const float sum_energies = moments_arr->engPos[cluster];
      if (thread_index < NumSamplings)
        {
          const int sampling = thread_index;
          const int max_cell = CMCTemporaries::maxECellPerSample(moments_arr, sampling, cluster);
          if (max_cell >= 0 && max_cell < NCaloCells)
            {
              const ClusterTag tag = cell_state_arr->clusterTag[max_cell];
              const float sec_weight = __uint_as_float(tag.secondary_cluster_weight());

              moments_arr->maxEPerSample[sampling][cluster]   = cell_info_arr->energy[max_cell] * (tag.cluster_index() == cluster ? 1.0f - sec_weight : sec_weight);
              //The cell can belong to either the first or second cluster.

              moments_arr->maxPhiPerSample[sampling][cluster] = geometry->phi[max_cell];
              moments_arr->maxEtaPerSample[sampling][cluster] = geometry->eta[max_cell];
            }
        }
      else if (thread_index == NumSamplings && sum_energies <= 0.f)
        {
          /*
          clusters_arr->seedCellID[cluster] = -1;
          // */

          //Maybe we can use more threads in parallel?
          //Doesn't seem much likely given that the sampling stuff
          //takes quuuite a while with all the memory accesses...
          switch (thread_index - NumSamplings)
            {
              case 0:
                moments_arr->firstPhi     [cluster] = 0;
                moments_arr->firstEta     [cluster] = 0;
                moments_arr->secondR      [cluster] = 0;
                moments_arr->secondLambda [cluster] = 0;
                moments_arr->deltaPhi     [cluster] = 0;
                moments_arr->deltaTheta   [cluster] = 0;
                moments_arr->deltaAlpha   [cluster] = 0;
                moments_arr->centerX      [cluster] = 0;
                moments_arr->centerY      [cluster] = 0;
                moments_arr->centerZ      [cluster] = 0;
                break;
              case 1:
                moments_arr->centerMag     [cluster] = 0;
                moments_arr->centerLambda  [cluster] = 0;
                moments_arr->lateral       [cluster] = 0;
                moments_arr->longitudinal  [cluster] = 0;
                moments_arr->engFracEM     [cluster] = 0;
                moments_arr->engFracMax    [cluster] = 0;
                moments_arr->engFracCore   [cluster] = 0;
                moments_arr->firstEngDens  [cluster] = 0;
                moments_arr->secondEngDens [cluster] = 0;
                moments_arr->isolation     [cluster] = 0;
                break;
              case 2:
                moments_arr->engBadCells      [cluster] = 0;
                moments_arr->nBadCells        [cluster] = 0;
                moments_arr->nBadCellsCorr    [cluster] = 0;
                moments_arr->badCellsCorrE    [cluster] = 0;
                moments_arr->badLArQFrac      [cluster] = 0;
                moments_arr->significance     [cluster] = 0;
                moments_arr->cellSignificance [cluster] = 0;
                moments_arr->cellSigSampling  [cluster] = 0;
                moments_arr->avgLArQ          [cluster] = 0;
                moments_arr->avgTileQ         [cluster] = 0;
                break;
              case 3:
                moments_arr->engBadHVCells       [cluster] = 0;
                moments_arr->nBadHVCells         [cluster] = 0;
                moments_arr->PTD                 [cluster] = 0;
                moments_arr->mass                [cluster] = 0;
                moments_arr->EMProbability       [cluster] = 0;
                moments_arr->hadWeight           [cluster] = 0;
                moments_arr->OOCweight           [cluster] = 0;
                moments_arr->DMweight            [cluster] = 0;
                moments_arr->tileConfidenceLevel [cluster] = 0;
                break;
              default:
                break;
            }
        }
      else if (thread_index == NumSamplings + 1 && sum_energies > 0.f)
        {
          const float center_x        = moments_arr->centerX[cluster];
          const float center_y        = moments_arr->centerY[cluster];
          const float center_z        = moments_arr->centerZ[cluster];
          const float axis_x          = CMCTemporaries::showerAxisX(moments_arr, cluster);
          const float axis_y          = CMCTemporaries::showerAxisY(moments_arr, cluster);
          const float axis_z          = CMCTemporaries::showerAxisZ(moments_arr, cluster);

          const float center_phi = atan2f(center_y, center_x);

          const float center_eta = Helpers::eta_from_coordinates(center_x, center_y, center_z);

          float lambda_c = 0.f;
          
          const int first_attempt_cell = geometry->get_closest_cell(CaloSampling::EMB1, center_eta, center_phi);
          
          if (first_attempt_cell >= 0)
            {
#if CALORECGPU_ETA_PHI_MAP_DEBUG
              printf("GREP FOR THIS: GPU %d %d %d %f %f\n", cluster, first_attempt_cell, CaloSampling::EMB1, center_eta, center_phi);
#endif
              const float r_calo = geometry->r[first_attempt_cell] - geometry->dr[first_attempt_cell] /
                                                               (geometry->is_tile(first_attempt_cell) ? 2.f : 1.f);

              const float axis_r = product_sum_cornea_harrison_tang(axis_x, axis_x, axis_y, axis_y);

              if (axis_r > 0)
                {
                  const float rev_axis_r = 1.0f / axis_r;
                  
                  const float axis_and_center_r = product_sum_cornea_harrison_tang(axis_x, center_x, axis_y, center_y);
                  
                  const float center_r_p1 = center_x * center_x;
                  const float center_r_c1 = fmaf(center_x, center_x, -center_r_p1);
                  const float center_r_p2 = center_y * center_y;
                  const float center_r_c2 = fmaf(center_y, center_y, -center_r_p2);
                  const float center_r_p3 = r_calo * r_calo;
                  const float center_r_c3 = fmaf(r_calo, r_calo, -center_r_p3);
                  const float center_r = sum_kahan_babushka_neumaier(center_r_p1, center_r_p2, -center_r_p3,
                                                                     center_r_c1, center_r_c2, -center_r_c3 );
                                                                     
                  const float det = product_sum_cornea_harrison_tang(axis_and_center_r * axis_and_center_r,
                                                                     rev_axis_r * rev_axis_r,
                                                                     -center_r,
                                                                     rev_axis_r);
                  
                  if (det > 0)
                    {
                      const float rootdet = sqrtf(det);
                      const float branch_1 =  fmaf(-axis_and_center_r, rev_axis_r, rootdet);
                      const float branch_2 = -fmaf( axis_and_center_r, rev_axis_r, rootdet);
                      lambda_c = min(fabsf(branch_1), fabsf(branch_2));
                    }
                }
            }
          else
            {
              constexpr int num_samplings_to_check = 4;
              constexpr int samplings_to_check[num_samplings_to_check] = { CaloSampling::EME1,
                                                                           CaloSampling::EME2,
                                                                           CaloSampling::FCAL0,
                                                                           CaloSampling::HEC0  };
              
              int this_cell = -1;
              
#if CALORECGPU_ETA_PHI_MAP_DEBUG
              int chosen_sampling = -1;
#endif
              
              for (int i = 0; i < num_samplings_to_check && this_cell < 0; ++i)
                {
                  this_cell = geometry->get_closest_cell(samplings_to_check[i], center_eta, center_phi);
                  
#if CALORECGPU_ETA_PHI_MAP_DEBUG
                  if (this_cell >= 0)
                  {
                    chosen_sampling = samplings_to_check[i];
                  }
#endif
                }
              
              if (this_cell >= 0)
                {
                  const float this_z = geometry->z[this_cell];
                  const float this_calc = this_z + (this_z >= 0.f ? -geometry->dz[this_cell] : geometry->dz[this_cell]) /
                                          (geometry->is_tile(this_cell) ? 2.f : 1.f);
                  
                  if (this_calc != 0.f && axis_z != 0.f)
                    {
                      lambda_c = fabsf( (this_calc - center_z) / axis_z );
                    }
                }
                
#if CALORECGPU_ETA_PHI_MAP_DEBUG
              printf("GREP FOR THIS: GPU %d %d %d %f %f\n", cluster, this_cell, chosen_sampling, center_eta, center_phi);
#endif    
            }
          moments_arr->centerLambda[cluster] = lambda_c;
        }
    }
}

/******************************************************************************
 * Actual kernel calling code.                                                *
 ******************************************************************************/


namespace
{
  struct MomentsKernelConfigurations
  {
    struct Config
    {
      unsigned int grid = 0, block = 0;

      Config() = default;
      Config(const Config &) = default;
      Config(Config &&) = default;
      Config & operator=(const Config &) = default;
      Config & operator=(Config &&) = default;

      __host__ __device__ Config(const CUDAKernelLaunchConfiguration & c):
        grid(static_cast<unsigned int>(c.grid_x)), block(static_cast<unsigned int>(c.block_x))
      {
      }

      __host__ __device__ Config & operator=(const CUDAKernelLaunchConfiguration & c)
      {
        grid  = static_cast<unsigned int>(c.grid_x);
        block = static_cast<unsigned int>(c.block_x);
        return (*this);
      }
    };

    Config c_isol_clu, c_isol_cell, c_0_clu, c_1_cell, c_1_clu, c_2_cell, c_axis, c_2_clu, c_3_cell, c_3_clu, c_final_clu;
  };

  MomentsKernelConfigurations get_configurations(const IGPUKernelSizeOptimizer & optimizer)
  {
    MomentsKernelConfigurations ret;

    ret.c_isol_clu  = optimizer.get_launch_configuration("ClusterMomentsCalculator",  0);
    ret.c_isol_cell = optimizer.get_launch_configuration("ClusterMomentsCalculator",  1);
    ret.c_0_clu     = optimizer.get_launch_configuration("ClusterMomentsCalculator",  2);
    ret.c_1_cell    = optimizer.get_launch_configuration("ClusterMomentsCalculator",  3);
    ret.c_1_clu     = optimizer.get_launch_configuration("ClusterMomentsCalculator",  4);
    ret.c_2_cell    = optimizer.get_launch_configuration("ClusterMomentsCalculator",  5);
    ret.c_axis      = optimizer.get_launch_configuration("ClusterMomentsCalculator",  6);
    ret.c_2_clu     = optimizer.get_launch_configuration("ClusterMomentsCalculator",  7);
    ret.c_3_cell    = optimizer.get_launch_configuration("ClusterMomentsCalculator",  8);
    ret.c_3_clu     = optimizer.get_launch_configuration("ClusterMomentsCalculator",  9);
    ret.c_final_clu = optimizer.get_launch_configuration("ClusterMomentsCalculator", 10);

    return ret;
  }
}


__global__ static
void calculateClusterPropertiesAndMomentsDeferKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                     Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                                                     Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                                     const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                                     const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                                     const Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                                                     const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                                                     const MomentsKernelConfigurations configs)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    {
      const int cluster_number = clusters_arr->number;

      auto updated_grid = [&](const MomentsKernelConfigurations::Config & c)
      {
        const unsigned int new_total_size = Helpers::int_ceil_div(c.grid * c.block, NMaxClusters) * cluster_number;
        return Helpers::int_ceil_div(new_total_size, c.block);
      };

      auto grid_for_cells = [&](const auto & process, const MomentsKernelConfigurations::Config & c)
      {
        constexpr unsigned int mult = process.number * 2;
        return dim3{Helpers::int_ceil_div(c.grid, mult), process.number, 2};
      };

      auto grid_for_clusters = [&](const auto & finalize, const auto & init, const MomentsKernelConfigurations::Config & c)
      {
        constexpr unsigned int number = (finalize.number > init.number ? finalize.number : init.number);
        return dim3{Helpers::int_ceil_div(updated_grid(c), number), number, 1};
      };

      using namespace ToCalculate;

      isolationClusterPassKernel
      <<< dim3{Helpers::int_ceil_div(updated_grid(configs.c_isol_clu), NumSamplings), NumSamplings, 1}, configs.c_isol_clu.block >>>
      (moments_arr, clusters_arr);

      isolationCellPassKernel
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
      <<< dim3{Helpers::int_ceil_div(configs.c_isol_cell.grid, 3), 3, 1}, configs.c_isol_cell.block >>>
#else
      <<< configs.c_isol_cell.grid, configs.c_isol_cell.block >>>
#endif
      (moments_arr, cell_state_arr, cell_info_arr, geometry, opts->use_abs_energy);

      zerothClusterPassKernel
      <<< updated_grid(configs.c_0_clu), configs.c_0_clu.block >>>
      (moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts, opts->skip_invalid_clusters);


      firstCellPassKernel
      <<< grid_for_cells(FirstPassCells{}, configs.c_1_cell), configs.c_1_cell.block >>>
      (moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts, cell_state_arr);

      firstClusterPassKernel
      <<< grid_for_clusters(FirstPassFinalization{}, SecondPassInitialization{}, configs.c_1_clu), configs.c_1_clu.block >>>
      (moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts, opts->skip_invalid_clusters);


      secondCellPassKernel
      <<< grid_for_cells(SecondPassCells{}, configs.c_2_cell), configs.c_2_cell.block >>>
      (moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts, cell_state_arr);

      showerAxisPassKernel
      <<< updated_grid(configs.c_axis), configs.c_axis.block >>>
      (moments_arr, clusters_arr, opts->max_axis_angle, opts->skip_invalid_clusters);

      secondClusterPassKernel
      <<< grid_for_clusters(SecondPassFinalization{}, ThirdPassInitialization{}, configs.c_2_clu), configs.c_2_clu.block >>>
      (moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts, opts->skip_invalid_clusters);


      thirdCellPassKernel
      <<< grid_for_cells(ThirdPassCells{}, configs.c_3_cell), configs.c_3_cell.block >>>
      (moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts, cell_state_arr);

      thirdClusterPassKernel
      <<< grid_for_clusters(ThirdPassFinalization{}, FinalPassInitialization{}, configs.c_3_clu), configs.c_3_clu.block >>>
      (moments_arr, clusters_arr, cell_info_arr, geometry, noise_arr, opts, opts->skip_invalid_clusters);


      finalClusterPassKernel
      <<< updated_grid(configs.c_final_clu), configs.c_final_clu.block>>>
      (moments_arr, clusters_arr, cell_state_arr, cell_info_arr, geometry, opts->skip_invalid_clusters);

      //We could have split this up and not rely so much on dynamic parallelism.
      //However, if not using CUDA 12 (which we probably won't be for a while),
      //we'd have to dyn-par our way through the number of clusters at every cluster-related kernel.
      //With tail calls, it's a bit simpler, but we'd have a mess of #ifdef and so on
      //until we could drop support for CUDA less than 12.
      //So I am currently taking the shortcut of just calling everything from here
      //and only calculating block sizes once...
    }
}

void ClusterMomentsCalculator::calculateClusterPropertiesAndMoments(CaloRecGPU::EventDataHolder & holder,
                                                                    const ConstantDataHolder & instance_data,
                                                                    const CMCOptionsHolder & options,
                                                                    const IGPUKernelSizeOptimizer & optimizer,
                                                                    size_t (&times)[num_time_measurements],
                                                                    const bool synchronize,
                                                                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream,
                                                                    const bool defer_instead_of_oversize)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const MomentsKernelConfigurations configs = get_configurations(optimizer);

  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  if (optimizer.use_minimal_kernel_sizes() && optimizer.can_use_dynamic_parallelism())
    {
      const auto t0 = clock_type::now();
      
      calculateClusterPropertiesAndMomentsDeferKernel <<< 1, 1, 0, stream_to_use >>> (holder.m_clusters_dev,
                                                                                      holder.m_moments_dev,
                                                                                      holder.m_cell_state_dev,
                                                                                      holder.m_cell_info_dev,
                                                                                      instance_data.m_geometry_dev,
                                                                                      instance_data.m_cell_noise_dev,
                                                                                      options.m_options_dev,
                                                                                      configs);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t1 = clock_type::now();

      for (unsigned int i = 0; i < num_time_measurements - 1; ++i)
        {
          times[i] = 0;
        }

      times[num_time_measurements - 1] = time_cast(t0, t1);
    }
  else
    {
      auto grid_for_cells = [&](const auto & process, const unsigned int grid)
      {
        unsigned int mult = process.number * 2;

        return dim3{Helpers::int_ceil_div(grid, mult), process.number, 2};
      };

      auto grid_for_clusters = [&](const auto & finalize, const auto & init, const unsigned int grid)
      {
        unsigned int number = (finalize.number > init.number ? finalize.number : init.number);

        return dim3{Helpers::int_ceil_div(grid, number), number, 1};
      };

      using namespace ToCalculate;


      const auto t0 = clock_type::now();

      isolationClusterPassKernel
      <<< dim3{Helpers::int_ceil_div(configs.c_isol_clu.grid, NumSamplings), NumSamplings, 1}, configs.c_isol_clu.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t1 = clock_type::now();

      isolationCellPassKernel
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
      <<< dim3{Helpers::int_ceil_div(configs.c_isol_cell.grid, 3), 3, 1}, configs.c_isol_cell.block, 0, stream_to_use >>>
#else
      <<< configs.c_isol_cell.grid, configs.c_isol_cell.block, 0, stream_to_use >>>
#endif
      (holder.m_moments_dev,
       holder.m_cell_state_dev,
       holder.m_cell_info_dev,
       instance_data.m_geometry_dev,
       options.m_options->use_abs_energy);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t2 = clock_type::now();

      zerothClusterPassKernel
      <<< configs.c_0_clu.grid, configs.c_0_clu.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev,
       holder.m_cell_info_dev,
       instance_data.m_geometry_dev,
       instance_data.m_cell_noise_dev,
       options.m_options_dev,
       options.m_options->skip_invalid_clusters);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t3 = clock_type::now();

      firstCellPassKernel
      <<< grid_for_cells(FirstPassCells{}, configs.c_1_cell.grid), configs.c_1_cell.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev,
       holder.m_cell_info_dev,
       instance_data.m_geometry_dev,
       instance_data.m_cell_noise_dev,
       options.m_options_dev,
       holder.m_cell_state_dev);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t4 = clock_type::now();

      firstClusterPassKernel
      <<< grid_for_clusters(FirstPassFinalization{}, SecondPassInitialization{}, configs.c_1_clu.grid), configs.c_1_clu.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev,
       holder.m_cell_info_dev,
       instance_data.m_geometry_dev,
       instance_data.m_cell_noise_dev,
       options.m_options_dev,
       options.m_options->skip_invalid_clusters);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t5 = clock_type::now();

      secondCellPassKernel
      <<< grid_for_cells(SecondPassCells{}, configs.c_2_cell.grid), configs.c_2_cell.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev,
       holder.m_cell_info_dev,
       instance_data.m_geometry_dev,
       instance_data.m_cell_noise_dev,
       options.m_options_dev,
       holder.m_cell_state_dev);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t6 = clock_type::now();

      showerAxisPassKernel
      <<< configs.c_axis.grid, configs.c_axis.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev,
       options.m_options->max_axis_angle,
       options.m_options->skip_invalid_clusters);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t7 = clock_type::now();

      secondClusterPassKernel
      <<< grid_for_clusters(SecondPassFinalization{}, ThirdPassInitialization{}, configs.c_2_clu.grid), configs.c_2_clu.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev,
       holder.m_cell_info_dev,
       instance_data.m_geometry_dev,
       instance_data.m_cell_noise_dev,
       options.m_options_dev,
       options.m_options->skip_invalid_clusters);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t8 = clock_type::now();

      thirdCellPassKernel
      <<< grid_for_cells(ThirdPassCells{}, configs.c_3_cell.grid), configs.c_3_cell.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev,
       holder.m_cell_info_dev,
       instance_data.m_geometry_dev,
       instance_data.m_cell_noise_dev,
       options.m_options_dev,
       holder.m_cell_state_dev);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto t9 = clock_type::now();

      thirdClusterPassKernel
      <<< grid_for_clusters(ThirdPassFinalization{}, FinalPassInitialization{}, configs.c_3_clu.grid), configs.c_3_clu.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev,
       holder.m_cell_info_dev,
       instance_data.m_geometry_dev,
       instance_data.m_cell_noise_dev,
       options.m_options_dev,
       options.m_options->skip_invalid_clusters);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto tA = clock_type::now();

      finalClusterPassKernel
      <<< configs.c_final_clu.grid, configs.c_final_clu.block, 0, stream_to_use >>>
      (holder.m_moments_dev,
       holder.m_clusters_dev,
       holder.m_cell_state_dev,
       holder.m_cell_info_dev,
       instance_data.m_geometry_dev,
       options.m_options->skip_invalid_clusters);

      if (synchronize)
        {
          CUDA_ERRCHECK(cudaPeekAtLastError());
          CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
        }

      const auto tB = clock_type::now();
      
      times[ 0] = time_cast(t0, t1);
      times[ 1] = time_cast(t1, t2);
      times[ 2] = time_cast(t2, t3);
      times[ 3] = time_cast(t3, t4);
      times[ 4] = time_cast(t4, t5);
      times[ 5] = time_cast(t5, t6);
      times[ 6] = time_cast(t6, t7);
      times[ 7] = time_cast(t7, t8);
      times[ 8] = time_cast(t8, t9);
      times[ 9] = time_cast(t9, tA);
      times[10] = time_cast(tA, tB);
    }
}

/*******************************************************************************************************************************/

void ClusterMomentsCalculator::register_kernels(IGPUKernelSizeOptimizer & optimizer)
{
  auto local_max = [](const auto & x, const auto & y)
  {
    return (x > y ? x : y);
  };

  void * kernels[] = { (void *) isolationClusterPassKernel,
                       (void *) isolationCellPassKernel,
                       (void *) zerothClusterPassKernel,
                       (void *) firstCellPassKernel,
                       (void *) firstClusterPassKernel,
                       (void *) secondCellPassKernel,
                       (void *) showerAxisPassKernel,
                       (void *) secondClusterPassKernel,
                       (void *) thirdCellPassKernel,
                       (void *) thirdClusterPassKernel,
                       (void *) finalClusterPassKernel
                     };

  int blocksizes[] = { ClusterPassBlockSize,                                                                            //isolation clusters
                       CellPassBlockSize,                                                                               //isolation cells
                       ClusterPassBlockSize,                                                                            //   zeroth clusters
                       CellPassBlockSize,                                                                               //    first cells
                       ClusterPassBlockSize,                                                                            //    first clusters
                       CellPassBlockSize,                                                                               //   second cells
                       ClusterPassBlockSize,                                                                            //   shower_axis
                       ClusterPassBlockSize,                                                                            //   second clusters
                       CellPassBlockSize,                                                                               //    third cells
                       ClusterPassBlockSize,                                                                            //    third clusters
                       ClusterPassBlockSize,                                                                            //    final clusters
                     };

  using namespace ToCalculate;

  int  gridsizes[] = { Helpers::int_ceil_div(NMaxClusters * NumSamplings, IsolationMemsetChunk * ClusterPassBlockSize), //isolation clusters
                       Helpers::int_ceil_div(NCaloCells * 3, CellPassBlockSize),                                        //isolation cells
                       Helpers::int_ceil_div(NMaxClusters, Helpers::int_floor_div(ClusterPassBlockSize, WarpSize)),     //   zeroth clusters
                       cell_pass_grid_size<FirstPassCells::number>(),                                                   //    first cells
                       cluster_pass_grid_size<FirstPassFinalization::number, SecondPassInitialization::number>(),       //    first clusters
                       cell_pass_grid_size<SecondPassCells::number>(),                                                  //   second cells
                       Helpers::int_ceil_div(NMaxClusters, ClusterPassBlockSize),                                       //   shower_axis
                       cluster_pass_grid_size<SecondPassFinalization::number, ThirdPassInitialization::number>(),       //   second clusters
                       cell_pass_grid_size<ThirdPassCells::number>(),                                                   //    third cells
                       cluster_pass_grid_size<ThirdPassFinalization::number, FinalPassInitialization::number>(),        //    third clusters
                       Helpers::int_ceil_div(NMaxClusters, Helpers::int_floor_div(ClusterPassBlockSize, WarpSize)),     //    final clusters
                     };

  int   maxsizes[] = { Helpers::int_ceil_div(NMaxClusters * NumSamplings, IsolationMemsetChunk),                        //isolation clusters
                       NCaloCells   * 3,                                                                                //isolation cells
                       NMaxClusters * WarpSize,                                                                         //   zeroth clusters
                       NCaloCells  *  FirstPassCells::number * 2,                                                       //    first cells
                       NMaxClusters * local_max(FirstPassFinalization::number, SecondPassInitialization::number),       //    first clusters
                       NCaloCells  *  SecondPassCells::number * 2,                                                      //   second cells
                       NMaxClusters,                                                                                    //   shower_axis
                       NMaxClusters * local_max(SecondPassFinalization::number, ThirdPassInitialization::number),       //   second clusters
                       NCaloCells  *  ThirdPassCells::number * 2,                                                       //    third cells
                       NMaxClusters * local_max(ThirdPassFinalization::number, FinalPassInitialization::number),        //    third clusters
                       NMaxClusters * WarpSize                                                                          //    final clusters
                     };

  optimizer.register_kernels("ClusterMomentsCalculator", 11, kernels, blocksizes, gridsizes, maxsizes);
}
