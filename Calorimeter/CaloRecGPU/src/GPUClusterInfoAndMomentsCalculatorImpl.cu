//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

//NOTE: at several points of this implementation file,
//      some commented out appears here and there
//      to invalidate some clusters (seedCellIndex[clusters] = -1)
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
                                  TypeList<CenterX, CenterY, CenterZ, ClusterPhi, FirstPhi, FirstEta>,
                                  TypeList<ClusterEnergyEtaAndEt, FirstEngDens, SecondEngDens>,
                                  TypeList<EngFracMax, MaxAndSecondMaxCells>
                                  >;

    using SecondPassInitialization = TypeList <
                                     TypeList<>,
                                     TypeList<NBadCells>,
                                     TypeList<NBadCellsCorr>,
                                     TypeList<BadCellsCorrE>,
                                     TypeList<BadLArQFrac>,
                                     TypeList<>,
                                     TypeList<AvgTileQ>,
                                     TypeList<>,
                                     TypeList<>,
                                     TypeList<>,
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
                                     TypeList<>,
                                     TypeList<AverageTileQNormalization>,
                                     TypeList<EngBadCells>,
                                     TypeList<>,
                                     TypeList<>,
                                     TypeList<>,
                                     TypeList<>,
                                     TypeList<Significance>,
                                     TypeList<PTD, NumPositiveEnergyCells>,
                                     TypeList<Matrix00, Matrix11, Matrix22>,
                                     TypeList<AvgLArQ, AverageLArQNormalization>,
                                     TypeList<MaxSignificanceAndSampling>
                                     >;

    //MaxSignificanceAndSampling and Max and Second Max Cells must be in the same slot!
    //Matrix{00, 11, 22}, center{X,Y,Z} and {FirstEta, FirstPhi, <ClusterPhi>} must be in the same respective slots!
    //PTD and Mass must be in the same slot!
    //AvgLArQ and AvgLarQNorm must be in the same slot as FirstEngDens and SecondEngDens!
    //NumPositiveEnergyCells and Mass must be in the same slot!
    

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
                                    TypeList<LateralNormalization>,
                                    TypeList<SecondR, LongitudinalNormalization>,
                                    TypeList<SecondLambda, Lateral>,
                                    TypeList<Longitudinal, NExtraCellSampling>
                                    >;
  //SecondLambda must be in the same slot as AvgLArQ

    using ThirdPassCells = TypeList <
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
  TypeList<EnergyPerSample>,
  TypeList<SecondR>,
  TypeList<SecondLambda>,
  TypeList<NExtraCellSampling>,
  TypeList<Lateral>,
  TypeList<LateralNormalization>,
  TypeList<Longitudinal>,
  TypeList<LongitudinalNormalization>,
  TypeList<MaxEnergyAndCellPerSample>
#else
  TypeList <
  EnergyPerSample, SecondR, SecondLambda, NExtraCellSampling,
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
  };

  __device__ CellPassIndexing get_cell_pass_indexing()
  {
    const unsigned int cell_index = blockIdx.x * blockDim.x + threadIdx.x;
    const unsigned int grid_size = gridDim.x * blockDim.x;

    return CellPassIndexing{static_cast<int>(cell_index),
                            static_cast<int>(grid_size),
                            static_cast<int>(blockIdx.y)};
  }

}

/******************************************************************************
 * "Zeroth" Pass: Isolation + other initialization                            *
 ******************************************************************************/

constexpr static int IsolationMemsetChunk = 1;

__global__ static
void isolationClusterPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int cluster_number = clusters_arr->number;

  const int grid_size = gridDim.x * blockDim.x;
  const int start_index   = (blockIdx.x * blockDim.x + threadIdx.x) * IsolationMemsetChunk;

  for (int index = start_index; index + IsolationMemsetChunk <= cluster_number; index += grid_size)
    {
      for (int j = 0; j < IsolationMemsetChunk; ++j)
        {
          const int cluster_index = index + j;
          clusters_arr->moments.energyPerSample                  [blockIdx.y][cluster_index] = 0.f;
          CMCTemporaries::energyPerSampleAux       (clusters_arr, blockIdx.y, cluster_index) = 0.f;
          CMCTemporaries::numberNonEmptySamplings  (clusters_arr, blockIdx.y, cluster_index) = 0;
          CMCTemporaries::numberEmptySamplings     (clusters_arr, blockIdx.y, cluster_index) = 0;
          CMCTemporaries::maxMomentsEnergyPerSample(clusters_arr, blockIdx.y, cluster_index) = 0;
          if (blockIdx.y == 0)
            {
              clusters_arr->moments.engPos[cluster_index] = 0.f;
            }
          if (blockIdx.y == 1)
            {
              CMCTemporaries::engPosAux(clusters_arr, cluster_index) = 0.f;
            }
        }
    }
}

__global__ static
void isolationCellPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                             const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                             const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                             const bool use_abs_energy, const bool assume_complete_cells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  const int num_cells = cell_info_arr->number;

#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
  if (blockIdx.y == 0)
#endif

    {

      for (int cell = index; cell < num_cells; cell += grid_size)
        {
          const int this_hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);

          if (this_hash_ID < 0)
            {
              continue;
            }

          const int sampling   = geometry->sampling(this_hash_ID);
          const ClusterTag tag = clusters_arr->get_extra_cell_info(cell);

          const int this_cluster = (tag.is_part_of_cluster() ? tag.cluster_index() : -1);

          int neighbours[NMaxAll2DNeighbours];

          const int num_total_neighs = geometry->get_neighbours(LArNeighbours::neighbourOption::all2D, this_hash_ID, neighbours);

          int num_useful_neighs = 0;

          for (int neigh = 0; neigh < num_total_neighs; ++neigh)
            {
              const int neigh_hash_ID = neighbours[neigh];

              const int neigh_index = cell_info_arr->get_cell_with_hash_ID(neigh_hash_ID, assume_complete_cells);

              const ClusterTag neigh_tag = (neigh_index >= 0 ? clusters_arr->get_extra_cell_info(neigh_index) : 0);

              const int neigh_cluster = (neigh_index >= 0 && neigh_tag.is_part_of_cluster() ? neigh_tag.cluster_index() : -1);

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
                      if (this_cluster >= 0)
                        {
                          atomicAdd(&(CMCTemporaries::numberNonEmptySamplings(clusters_arr, sampling, neigh_cluster)), 1);
                        }
                      else
                        {
                          atomicAdd(&(CMCTemporaries::numberEmptySamplings(clusters_arr, sampling, neigh_cluster)), 1);
                        }
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
      for (int cell = index; cell < num_cells; cell += grid_size)
        {
          const ClusterTag tag = clusters_arr->get_extra_cell_info(cell);
#endif

          if (tag.is_part_of_cluster())
            {

#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
              const int this_hash_ID = cell_info_arr->get_hash_ID(cell, assume_complete_cells);

              if (this_hash_ID < 0)
                {
                  continue;
                }

              const int sampling   = geometry->sampling(this_hash_ID);
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
                  const int this_primary_cluster = tag.cluster_index();
                  
                  Helpers::device_kahan_babushka_neumaier(&(clusters_arr->moments.energyPerSample[sampling][this_primary_cluster]),
                                                          CMCTemporaries::energyPerSampleAux_ptr(clusters_arr, sampling, this_primary_cluster),
                                                          energy * primary_weight);

                  atomicMax(&(CMCTemporaries::maxMomentsEnergyPerSample(clusters_arr, sampling, tag.cluster_index())),
                            __float_as_uint(moments_energy * primary_weight));

                  Helpers::device_kahan_babushka_neumaier(&(clusters_arr->moments.engPos[this_primary_cluster]),
                                                          CMCTemporaries::engPosAux_ptr(clusters_arr, this_primary_cluster),
                                                          moments_energy * primary_weight);
                }

#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
              else if (blockIdx.y == 2 && tag.is_shared_between_clusters())
#else
              if (tag.is_shared_between_clusters())
#endif

                {
                  const int this_secondary_cluster = tag.secondary_cluster_index();
                  
                  Helpers::device_kahan_babushka_neumaier(&(clusters_arr->moments.energyPerSample[sampling][this_secondary_cluster]),
                                                          CMCTemporaries::energyPerSampleAux_ptr(clusters_arr, sampling, this_secondary_cluster),
                                                          energy * secondary_weight);

                  atomicMax(&(CMCTemporaries::maxMomentsEnergyPerSample(clusters_arr, sampling, this_secondary_cluster)),
                            __float_as_uint(moments_energy * secondary_weight));

                  Helpers::device_kahan_babushka_neumaier(&(clusters_arr->moments.engPos[this_secondary_cluster]),
                                                          &CMCTemporaries::engPosAux(clusters_arr, this_secondary_cluster),
                                                          moments_energy * secondary_weight);
                }
            }
        }
    }
}

__global__ static
void zerothClusterPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                             const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                             const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                             const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                             const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                             const bool skip_invalid,
                             const bool assume_complete_cells)
{
  const int cluster_number = clusters_arr->number;

  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int moment  = threadIdx.x % WarpSize;
  const int grid_size = gridDim.x * blockDim.x;

  Parameters p {assume_complete_cells, moment, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cluster = index / WarpSize; cluster < cluster_number; cluster += grid_size / WarpSize)
    {
      if (skip_invalid && clusters_arr->seedCellIndex[cluster] < 0)
        {
          continue;
        }

      if (moment < NumSamplings)
        {
          const int sampling = moment;
          const float sampling_energy = clusters_arr->moments.energyPerSample[sampling][cluster] +
                                        CMCTemporaries::energyPerSampleAux(clusters_arr, sampling, cluster);
          const unsigned int max_energy_pattern = CMCTemporaries::maxMomentsEnergyPerSample(clusters_arr, sampling, cluster);
          const float sampling_max_energy = __uint_as_float(max_energy_pattern);
          const int sampling_empty = CMCTemporaries::numberEmptySamplings(clusters_arr, sampling, cluster);
          const int sampling_non_empty = CMCTemporaries::numberNonEmptySamplings(clusters_arr, sampling, cluster);

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

              Helpers::partial_kahan_babushka_neumaier_sum(other_isol + other_isol_corr,      isolation,      isolation_corr);
              Helpers::partial_kahan_babushka_neumaier_sum(other_norm + other_norm_corr, isolation_norm, isolation_norm_corr);
              Helpers::partial_kahan_babushka_neumaier_sum(other_enfc + other_enfc_corr,  eng_frac_core,  eng_frac_core_corr);
            }

          switch (moment)
            {
              case 0:
                {
                  const float real_iso  = isolation      + isolation_corr;
                  const float real_norm = isolation_norm + isolation_norm_corr;
                  clusters_arr->moments.isolation[cluster] = (real_norm != 0.f ? real_iso / real_norm : 0.f);
                }
                break;
              case NumSamplings - 1:
                clusters_arr->moments.engFracCore[cluster] = eng_frac_core + eng_frac_core_corr;
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
void firstCellPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                         const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                         const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                         const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                         const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                         const bool assume_complete_cells)
{
  const auto indexing = get_cell_pass_indexing();

  Parameters p {assume_complete_cells, indexing.moment, clusters_arr, cell_info_arr, geometry, noise_arr, opts};
  
  for (int cell = indexing.cell; cell < clusters_arr->number_cells; cell += indexing.delta_cell)
    {
      do_cell_pass(ToCalculate::FirstPassCells{},
                  clusters_arr->cells.indices[cell],
                  clusters_arr->clusterIndices[cell],
                  clusters_arr->cellWeights[cell],
                  p);
    }
}

__global__ static
void firstClusterPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                            const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                            const bool skip_invalid,
                            const bool assume_complete_cells)
{
  const int cluster_number = clusters_arr->number;

  const auto indexing = get_cluster_pass_indexing();

  Parameters p {assume_complete_cells, indexing.moment, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cluster = indexing.cluster; cluster < cluster_number; cluster += indexing.delta_cluster)
    {
      if (skip_invalid && clusters_arr->seedCellIndex[cluster] < 0)
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
void secondCellPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                          const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                          const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                          const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                          const bool assume_complete_cells)
{
  const auto indexing = get_cell_pass_indexing();

  Parameters p {assume_complete_cells, indexing.moment, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cell = indexing.cell; cell < clusters_arr->number_cells; cell += indexing.delta_cell)
    {
      do_cell_pass(ToCalculate::SecondPassCells{},
                  clusters_arr->cells.indices[cell],
                  clusters_arr->clusterIndices[cell],
                  clusters_arr->cellWeights[cell],
                  p);
    }
}

__global__ static
void showerAxisPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          const float max_axis_angle, const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;
  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cluster = index; cluster < cluster_number; cluster += grid_size)
    {
      if (skip_invalid && clusters_arr->seedCellIndex[cluster] < 0)
        {
          continue;
        }

      const float center_x   = clusters_arr->moments.centerX[cluster];
      const float center_y   = clusters_arr->moments.centerY[cluster];
      const float center_z   = clusters_arr->moments.centerZ[cluster];
      const float center_mag_inv_base = rnorm3df(center_x, center_y, center_z);
      const float center_mag_inv = (isnan(center_mag_inv_base) || isinf(center_mag_inv_base) ? 1.f : center_mag_inv_base);
      clusters_arr->moments.centerMag[cluster] = 1.0f / center_mag_inv;
      float axis_x = center_x * center_mag_inv;
      float axis_y = center_y * center_mag_inv;
      float axis_z = center_z * center_mag_inv;
      float delta_phi = 0, delta_theta = 0, delta_alpha = 0;

      if (CMCTemporaries::numPositiveEnergyCells(clusters_arr, cluster) > 2)
        {
          const float norm = 1.f / (CMCTemporaries::sumSquareEnergies(clusters_arr, cluster) +
                                    CMCTemporaries::sumSquareEnergiesAux(clusters_arr, cluster));

          RealSymmetricMatrixSolverIterative solver { (CMCTemporaries::matrix00(clusters_arr, cluster) +
                                                       CMCTemporaries::matrix00Aux(clusters_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix11(clusters_arr, cluster) +
                                                       CMCTemporaries::matrix11Aux(clusters_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix22(clusters_arr, cluster) +
                                                       CMCTemporaries::matrix22Aux(clusters_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix10(clusters_arr, cluster) +
                                                       CMCTemporaries::matrix10Aux(clusters_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix21(clusters_arr, cluster) +
                                                       CMCTemporaries::matrix21Aux(clusters_arr, cluster)) * norm,
                                                      (CMCTemporaries::matrix20(clusters_arr, cluster) +
                                                       CMCTemporaries::matrix20Aux(clusters_arr, cluster)) * norm  };

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

                  const float d1 = norm3df( Helpers::product_sum_cornea_harrison_tang(prev_norm, vecs[i][0], -this_norm, axis_x),
                                            Helpers::product_sum_cornea_harrison_tang(prev_norm, vecs[i][1], -this_norm, axis_y),
                                            Helpers::product_sum_cornea_harrison_tang(prev_norm, vecs[i][2], -this_norm, axis_z) );

                  const float d2 = norm3df( Helpers::product_sum_cornea_harrison_tang(prev_norm, vecs[i][0], this_norm, axis_x),
                                            Helpers::product_sum_cornea_harrison_tang(prev_norm, vecs[i][1], this_norm, axis_y),
                                            Helpers::product_sum_cornea_harrison_tang(prev_norm, vecs[i][2], this_norm, axis_z) );

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
                  //clusters_arr->seedCellIndex[cluster] = -1;
                }
            }
          else
            {
              //clusters_arr->seedCellIndex[cluster] = -1;
            }
        }
      CMCTemporaries::showerAxisX(clusters_arr, cluster) = axis_x;
      CMCTemporaries::showerAxisY(clusters_arr, cluster) = axis_y;
      CMCTemporaries::showerAxisZ(clusters_arr, cluster) = axis_z;
      clusters_arr->moments.deltaPhi[cluster]   = delta_phi;
      clusters_arr->moments.deltaTheta[cluster] = delta_theta;
      clusters_arr->moments.deltaAlpha[cluster] = delta_alpha;
    }
}

__global__ static
void secondClusterPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                             const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                             const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                             const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                             const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                             const bool skip_invalid,
                             const bool assume_complete_cells)
{
  const int cluster_number = clusters_arr->number;

  const auto indexing = get_cluster_pass_indexing();

  Parameters p {assume_complete_cells, indexing.moment, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cluster = indexing.cluster; cluster < cluster_number; cluster += indexing.delta_cluster)
    {
      if (skip_invalid && clusters_arr->seedCellIndex[cluster] < 0)
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
void thirdCellPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                         const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                         const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                         const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                         const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                         const bool assume_complete_cells)
{
  const auto indexing = get_cell_pass_indexing();

  Parameters p {assume_complete_cells, indexing.moment, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cell = indexing.cell; cell < clusters_arr->number_cells; cell += indexing.delta_cell)
    {
      do_cell_pass(ToCalculate::ThirdPassCells{},
                  clusters_arr->cells.indices[cell],
                  clusters_arr->clusterIndices[cell],
                  clusters_arr->cellWeights[cell],
                  p);
    }
}

__global__ static
void thirdClusterPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const CaloRecGPU::Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                            const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                            const bool skip_invalid,
                            const bool assume_complete_cells)
{
  const int cluster_number = clusters_arr->number;

  const auto indexing = get_cluster_pass_indexing();

  Parameters p {assume_complete_cells, indexing.moment, clusters_arr, cell_info_arr, geometry, noise_arr, opts};

  for (int cluster = indexing.cluster; cluster < cluster_number; cluster += indexing.delta_cluster)
    {
      if (skip_invalid && clusters_arr->seedCellIndex[cluster] < 0)
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
void finalClusterPassKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const bool skip_invalid,
                            const bool assume_complete_cells)
{
  const int cluster_number = clusters_arr->number;

  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int thread_index  = threadIdx.x % WarpSize;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cluster = index / WarpSize; cluster < cluster_number; cluster += grid_size / WarpSize)
    {      
      if (skip_invalid && clusters_arr->seedCellIndex[cluster] < 0)
        {
          continue;
        }
      
      const float sum_energies = clusters_arr->moments.engPos[cluster];
      if (thread_index < NumSamplings)
        {
          const int sampling = thread_index;
          const int max_cell = CMCTemporaries::maxECellPerSample(clusters_arr, sampling, cluster);
          if (max_cell >= 0 && max_cell < NCaloCells)
            {
              const ClusterTag tag = clusters_arr->get_extra_cell_info(max_cell);
              
              const float sec_weight = __uint_as_float(tag.secondary_cluster_weight());

              clusters_arr->moments.maxEPerSample[sampling][cluster]   = cell_info_arr->energy[max_cell] * (tag.cluster_index() == cluster ? 1.0f - sec_weight : sec_weight);
              //The cell can belong to either the first or second cluster.

              const int max_cell_hash_ID = cell_info_arr->get_hash_ID(max_cell, assume_complete_cells);

              clusters_arr->moments.maxPhiPerSample[sampling][cluster] = geometry->phi[max_cell_hash_ID];
              clusters_arr->moments.maxEtaPerSample[sampling][cluster] = geometry->eta[max_cell_hash_ID];
            }
          else
            {
              clusters_arr->moments.maxEPerSample[sampling][cluster]   = 0.f;
              clusters_arr->moments.maxPhiPerSample[sampling][cluster] = 0.f;
              clusters_arr->moments.maxEtaPerSample[sampling][cluster] = 0.f;
            }
        }
      else if (thread_index == NumSamplings && sum_energies < 0.f)
        {
          /*
          clusters_arr->seedCellIndex[cluster] = -1;
          // */

          //Maybe we can use more threads in parallel?
          //Doesn't seem much likely given that the sampling stuff
          //takes quuuite a while with all the memory accesses...
          switch (thread_index - NumSamplings)
            {
              case 0:
                clusters_arr->moments.firstPhi     [cluster] = 0;
                clusters_arr->moments.firstEta     [cluster] = 0;
                clusters_arr->moments.secondR      [cluster] = 0;
                clusters_arr->moments.secondLambda [cluster] = 0;
                clusters_arr->moments.deltaPhi     [cluster] = 0;
                clusters_arr->moments.deltaTheta   [cluster] = 0;
                clusters_arr->moments.deltaAlpha   [cluster] = 0;
                clusters_arr->moments.centerX      [cluster] = 0;
                clusters_arr->moments.centerY      [cluster] = 0;
                clusters_arr->moments.centerZ      [cluster] = 0;
                break;
              case 1:
                clusters_arr->moments.centerMag     [cluster] = 0;
                clusters_arr->moments.centerLambda  [cluster] = 0;
                clusters_arr->moments.lateral       [cluster] = 0;
                clusters_arr->moments.longitudinal  [cluster] = 0;
                clusters_arr->moments.engFracEM     [cluster] = 0;
                clusters_arr->moments.engFracMax    [cluster] = 0;
                clusters_arr->moments.engFracCore   [cluster] = 0;
                clusters_arr->moments.firstEngDens  [cluster] = 0;
                clusters_arr->moments.secondEngDens [cluster] = 0;
                clusters_arr->moments.isolation     [cluster] = 0;
                break;
              case 2:
                clusters_arr->moments.engBadCells      [cluster] = 0;
                clusters_arr->moments.nBadCells        [cluster] = 0;
                clusters_arr->moments.nBadCellsCorr    [cluster] = 0;
                clusters_arr->moments.badCellsCorrE    [cluster] = 0;
                clusters_arr->moments.badLArQFrac      [cluster] = 0;
                clusters_arr->moments.significance     [cluster] = 0;
                clusters_arr->moments.cellSignificance [cluster] = 0;
                clusters_arr->moments.cellSigSampling  [cluster] = 0;
                clusters_arr->moments.avgLArQ          [cluster] = 0;
                clusters_arr->moments.avgTileQ         [cluster] = 0;
                break;
              case 3:
                clusters_arr->moments.engBadHVCells       [cluster] = 0;
                clusters_arr->moments.nBadHVCells         [cluster] = 0;
                clusters_arr->moments.PTD                 [cluster] = 0;
                clusters_arr->moments.mass                [cluster] = 0;
                clusters_arr->moments.EMProbability       [cluster] = 0;
                clusters_arr->moments.hadWeight           [cluster] = 0;
                clusters_arr->moments.OOCweight           [cluster] = 0;
                clusters_arr->moments.DMweight            [cluster] = 0;
                clusters_arr->moments.tileConfidenceLevel [cluster] = 0;
                break;
              default:
                break;
            }
        }
      else if (thread_index == NumSamplings + 1 && sum_energies >= 0.f)
        {
          const float center_x        = clusters_arr->moments.centerX[cluster];
          const float center_y        = clusters_arr->moments.centerY[cluster];
          const float center_z        = clusters_arr->moments.centerZ[cluster];
          const float axis_x          = CMCTemporaries::showerAxisX(clusters_arr, cluster);
          const float axis_y          = CMCTemporaries::showerAxisY(clusters_arr, cluster);
          const float axis_z          = CMCTemporaries::showerAxisZ(clusters_arr, cluster);

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

              const float axis_r = Helpers::product_sum_cornea_harrison_tang(axis_x, axis_x, axis_y, axis_y);

              if (axis_r > 0)
                {
                  const float rev_axis_r = 1.0f / axis_r;

                  const float axis_and_center_r = Helpers::product_sum_cornea_harrison_tang(axis_x, center_x, axis_y, center_y);

                  const float center_r_p1 = center_x * center_x;
                  const float center_r_c1 = fmaf(center_x, center_x, -center_r_p1);
                  const float center_r_p2 = center_y * center_y;
                  const float center_r_c2 = fmaf(center_y, center_y, -center_r_p2);
                  const float center_r_p3 = r_calo * r_calo;
                  const float center_r_c3 = fmaf(r_calo, r_calo, -center_r_p3);
                  const float center_r = Helpers::sum_kahan_babushka_neumaier(center_r_p1, center_r_p2, -center_r_p3,
                                                                              center_r_c1, center_r_c2, -center_r_c3 );

                  const float det = Helpers::product_sum_cornea_harrison_tang(axis_and_center_r * axis_and_center_r,
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
                                                                           CaloSampling::HEC0
                                                                         };

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
          clusters_arr->moments.centerLambda[cluster] = lambda_c;
        }
    }

  if (index == 0)
    {
      clusters_arr->state = ClusterInformationState::WithMoments;
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

void ClusterMomentsCalculator::calculateClusterPropertiesAndMoments(CaloRecGPU::EventDataHolder & holder,
                                                                    const ConstantDataHolder & instance_data,
                                                                    const CMCOptionsHolder & options,
                                                                    const IGPUKernelSizeOptimizer & optimizer,
                                                                    size_t (&times)[num_time_measurements],
                                                                    const bool synchronize,
                                                                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const MomentsKernelConfigurations configs = get_configurations(optimizer);

  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  auto grid_for_cells = [&](const auto & process, const unsigned int grid)
  {
    return dim3{Helpers::int_ceil_div(grid, process.number), process.number, 1};
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
  (holder.m_clusters_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  const auto t1 = clock_type::now();

  isolationCellPassKernel
#if CALORECGPU_MOMENTS_SEPARATE_CELLS_CALCULATION
  <<< dim3 {Helpers::int_ceil_div(configs.c_isol_cell.grid, 3), 3, 1}, configs.c_isol_cell.block, 0, stream_to_use >>>
#else
  <<< configs.c_isol_cell.grid, configs.c_isol_cell.block, 0, stream_to_use >>>
#endif
  (holder.m_clusters_dev,
   holder.m_cell_info_dev,
   instance_data.m_geometry_dev,
   options.m_options->use_abs_energy,
   holder.m_cell_info->complete);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  const auto t2 = clock_type::now();

  zerothClusterPassKernel
  <<< configs.c_0_clu.grid, configs.c_0_clu.block, 0, stream_to_use >>>
  (holder.m_clusters_dev,
   holder.m_cell_info_dev,
   instance_data.m_geometry_dev,
   instance_data.m_cell_noise_dev,
   options.m_options_dev,
   options.m_options->skip_invalid_clusters,
   holder.m_cell_info->complete);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  const auto t3 = clock_type::now();

  firstCellPassKernel
  <<< grid_for_cells(FirstPassCells{}, configs.c_1_cell.grid), configs.c_1_cell.block, 0, stream_to_use >>>
  (holder.m_clusters_dev,
   holder.m_cell_info_dev,
   instance_data.m_geometry_dev,
   instance_data.m_cell_noise_dev,
   options.m_options_dev,
   holder.m_cell_info->complete);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  const auto t4 = clock_type::now();

  firstClusterPassKernel
  <<< grid_for_clusters(FirstPassFinalization{}, SecondPassInitialization{}, configs.c_1_clu.grid), configs.c_1_clu.block, 0, stream_to_use >>>
  (holder.m_clusters_dev,
   holder.m_cell_info_dev,
   instance_data.m_geometry_dev,
   instance_data.m_cell_noise_dev,
   options.m_options_dev,
   options.m_options->skip_invalid_clusters,
   holder.m_cell_info->complete);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  const auto t5 = clock_type::now();

  secondCellPassKernel
  <<< grid_for_cells(SecondPassCells{}, configs.c_2_cell.grid), configs.c_2_cell.block, 0, stream_to_use >>>
  (holder.m_clusters_dev,
   holder.m_cell_info_dev,
   instance_data.m_geometry_dev,
   instance_data.m_cell_noise_dev,
   options.m_options_dev,
   holder.m_cell_info->complete);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  const auto t6 = clock_type::now();

  showerAxisPassKernel
  <<< configs.c_axis.grid, configs.c_axis.block, 0, stream_to_use >>>
  (holder.m_clusters_dev,
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
  (holder.m_clusters_dev,
   holder.m_cell_info_dev,
   instance_data.m_geometry_dev,
   instance_data.m_cell_noise_dev,
   options.m_options_dev,
   options.m_options->skip_invalid_clusters,
   holder.m_cell_info->complete);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  const auto t8 = clock_type::now();

  thirdCellPassKernel
  <<< grid_for_cells(ThirdPassCells{}, configs.c_3_cell.grid), configs.c_3_cell.block, 0, stream_to_use >>>
  (holder.m_clusters_dev,
   holder.m_cell_info_dev,
   instance_data.m_geometry_dev,
   instance_data.m_cell_noise_dev,
   options.m_options_dev,
   holder.m_cell_info->complete);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  const auto t9 = clock_type::now();

  thirdClusterPassKernel
  <<< grid_for_clusters(ThirdPassFinalization{}, FinalPassInitialization{}, configs.c_3_clu.grid), configs.c_3_clu.block, 0, stream_to_use >>>
  (holder.m_clusters_dev,
   holder.m_cell_info_dev,
   instance_data.m_geometry_dev,
   instance_data.m_cell_noise_dev,
   options.m_options_dev,
   options.m_options->skip_invalid_clusters,
   holder.m_cell_info->complete);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  const auto tA = clock_type::now();

  finalClusterPassKernel
  <<< configs.c_final_clu.grid, configs.c_final_clu.block, 0, stream_to_use >>>
  (holder.m_clusters_dev,
   holder.m_cell_info_dev,
   instance_data.m_geometry_dev,
   options.m_options->skip_invalid_clusters,
   holder.m_cell_info->complete);
  
  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }

  if (holder.m_clusters.valid())
    {
      holder.m_clusters->state = ClusterInformationState::WithMoments;
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
