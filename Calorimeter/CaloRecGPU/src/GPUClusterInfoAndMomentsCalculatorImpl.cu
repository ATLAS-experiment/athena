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


#include "CaloGeoHelpers/CaloSampling.h"
//Just enums and stuff, CUDA compatible.

#include <cmath>

using namespace CaloRecGPU;
using namespace ClusterMomentsCalculator;

void ClusterMomentsCalculator::CMCOptionsHolder::allocate()
{
  m_options.allocate();
  m_options_dev.allocate();
}

void ClusterMomentsCalculator::CMCOptionsHolder::sendToGPU(const bool clear_CPU)
{
  m_options_dev = m_options;
  if (clear_CPU)
    {
      m_options.clear();
    }
}

constexpr static int ClusterPassBlockSize = 1024;
constexpr static int CellPassBlockSize = 1024;
//Maximize throughput?
//Needs measurements, perhaps...
//Also we could split all the sub-kernels
//to have different block sizes.

namespace
{

  namespace CMCHack
  {
    //A generic TempSpecifier has typedef called "type" to specify the type of the return array
    //(e. g. should be float for most things), a static constexpr unsigned int number
    //to count the number of the array (or, more accurately, the number of other 4-byte
    //arrays before this one), and a static constexpr bool reverse to mark temporaries
    //that we being storing from the end of the moments array...
    //In practice, for our use cases, we will specialize this
    //to be sure there is no overlap...
    template <class TempSpecifier>
    __host__ __device__ typename TempSpecifier::type * get_temporary_array(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      char * c_ptr = (char *) ((ClusterMomentsArr *) moments_arr);
      if (TempSpecifier::reverse)
        {
          return (typename TempSpecifier::type * ) ( c_ptr + (NMaxClusters * TempSpecifier::number * sizeof(float)) );
          //I know. sizeof(char) == 1. Clearer this way, still...
        }
      else
        {
          return (typename TempSpecifier::type * ) ( c_ptr + sizeof(ClusterMomentsArr) - (NMaxClusters * (TempSpecifier::number + 1) * sizeof(float)) );
          //I know. sizeof(char) == 1. Clearer this way, still...
        }
    }
  }

  struct SeedCellPhi
  {
    using type = float;
  };

  struct EnergyDensityNormalization
  {
    using type = float;
  };

  struct SumAbsEnergyNonMoments
  {
    using type = float;
  };

  struct MaxCellEnergyAndCell
  {
    using type = unsigned long long int;
  };

  struct SecondMaxCellEnergyAndCell
  {
    using type = unsigned long long int;
  };

  //For later:

  struct MaxCells
  {
    using type = int;
  };

  struct SecondMaxCells
  {
    using type = int;
  };

  //-----------------

  struct NumberEmptySamplings
  {
    using type = int[NMaxClusters];
  };

  struct NumberNonEmptySamplings
  {
    using type = int[NMaxClusters];
  };

  struct MX
  {
    using type = float;
  };
  struct MY
  {
    using type = float;
  };
  struct MZ
  {
    using type = float;
  };

  struct MaxMomentsEnergyPerSample
  {
    using type = unsigned int[NMaxClusters];
    //Stored as unsigned ints because there's no atomicMax for floats in CUDA...
  };

  struct MaxSignificanceAndSampling
  {
    using type = unsigned long long int;
  };

  struct Matrix00
  {
    using type = float;
  };
  struct Matrix10
  {
    using type = float;
  };
  struct Matrix20
  {
    using type = float;
  };
  struct Matrix11
  {
    using type = float;
  };
  struct Matrix21
  {
    using type = float;
  };
  struct Matrix22
  {
    using type = float;
  };
  struct SumSquareEnergies
  {
    using type = float;
  };
  struct TimeNormalization
  {
    using type = float;
  };
  struct AverageLArQNorm
  {
    using type = float;
  };
  struct AverageTileQNorm
  {
    using type = float;
  };

  struct ShowerAxisX
  {
    using type = float;
  };

  struct ShowerAxisY
  {
    using type = float;
  };

  struct ShowerAxisZ
  {
    using type = float;
  };

  struct NumPositiveEnergyCells
  {
    using type = int;
  };

  struct AbsoluteEnergyPerSample
  {
    using type = float[NMaxClusters];
  };

  struct LateralNormalization
  {
    using type = float;
  };
  struct LongitudinalNormalization
  {
    using type = float;
  };
  struct MaxEnergyAndCellPerSample
  {
    using type = unsigned long long int[NMaxClusters];
  };

  struct MaxECellPerSample
  {
    using type = int[NMaxClusters];
  };


  namespace CMCHack
//Special cases, to prevent overlap.
  {
    template <>
    __host__ __device__ typename SeedCellPhi::type * get_temporary_array<SeedCellPhi>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (SeedCellPhi::type *) moments_arr->engCalibFracRest;
    }

    template <>
    __host__ __device__ typename EnergyDensityNormalization::type * get_temporary_array<EnergyDensityNormalization>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (EnergyDensityNormalization::type *) moments_arr->etaCaloFrame;
    }

    template <>
    __host__ __device__ typename SumAbsEnergyNonMoments::type * get_temporary_array<SumAbsEnergyNonMoments>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (SumAbsEnergyNonMoments::type *) moments_arr->engCalibFracHad;
    }

    template <>
    __host__ __device__ typename MaxCellEnergyAndCell::type * get_temporary_array<MaxCellEnergyAndCell>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (MaxCellEnergyAndCell::type *) moments_arr->engCalibDeadUnclass;
    }

    template <>
    __host__ __device__ typename SecondMaxCellEnergyAndCell::type * get_temporary_array<SecondMaxCellEnergyAndCell>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (SecondMaxCellEnergyAndCell::type *) moments_arr->engCalibDeadFCAL;
    }

    template <>
    __host__ __device__ typename MaxCells::type * get_temporary_array<MaxCells>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (MaxCells::type *) moments_arr->engBadHVCells;
    }
    template <>
    __host__ __device__ typename SecondMaxCells::type * get_temporary_array<SecondMaxCells>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (SecondMaxCells::type *) moments_arr->nBadHVCells;
    }

    template <>
    __host__ __device__ typename NumberEmptySamplings::type * get_temporary_array<NumberEmptySamplings>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (NumberEmptySamplings::type *) moments_arr->maxEPerSample;
    }

    template <>
    __host__ __device__ typename NumberNonEmptySamplings::type * get_temporary_array<NumberNonEmptySamplings>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (NumberNonEmptySamplings::type *) moments_arr->maxPhiPerSample;
    }

    template <>
    __host__ __device__ typename MaxMomentsEnergyPerSample::type * get_temporary_array<MaxMomentsEnergyPerSample>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (MaxMomentsEnergyPerSample::type *) moments_arr->maxEtaPerSample;
    }

    template <>
    __host__ __device__ typename MaxSignificanceAndSampling::type * get_temporary_array<MaxSignificanceAndSampling>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (MaxSignificanceAndSampling::type *) moments_arr->eta1CaloFrame;
    }


    template <>
    __host__ __device__ typename MX::type * get_temporary_array<MX>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (MX::type *) moments_arr->engCalibOutL;
    }

    template <>
    __host__ __device__ typename MY::type * get_temporary_array<MY>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (MY::type *) moments_arr->engCalibOutM;
    }

    template <>
    __host__ __device__ typename MZ::type * get_temporary_array<MZ>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (MZ::type *) moments_arr->engCalibOutT;
    }

    template <>
    __host__ __device__ typename Matrix00::type * get_temporary_array<Matrix00>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (Matrix00::type *) moments_arr->engCalibDeadT;
    }
    template <>
    __host__ __device__ typename Matrix10::type * get_temporary_array<Matrix10>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (Matrix10::type *) moments_arr->engCalibEMB0;
    }
    template <>
    __host__ __device__ typename Matrix20::type * get_temporary_array<Matrix20>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (Matrix20::type *) moments_arr->engCalibEME0;
    }
    template <>
    __host__ __device__ typename Matrix11::type * get_temporary_array<Matrix11>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (Matrix11::type *) moments_arr->engCalibTileG3;
    }
    template <>
    __host__ __device__ typename Matrix21::type * get_temporary_array<Matrix21>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (Matrix21::type *) moments_arr->engCalibDeadTot;
    }
    template <>
    __host__ __device__ typename Matrix22::type * get_temporary_array<Matrix22>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (Matrix22::type *) moments_arr->engCalibDeadEMB0;
    }
    template <>
    __host__ __device__ typename SumSquareEnergies::type * get_temporary_array<SumSquareEnergies>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (SumSquareEnergies::type *) moments_arr->engCalibDeadTile0;
    }
    template <>
    __host__ __device__ typename TimeNormalization::type * get_temporary_array<TimeNormalization>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (TimeNormalization::type *) moments_arr->engCalibDeadTileG3;
    }
    template <>
    __host__ __device__ typename AverageLArQNorm::type * get_temporary_array<AverageLArQNorm>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (AverageLArQNorm::type *) moments_arr->engCalibDeadEME0;
    }
    template <>
    __host__ __device__ typename AverageTileQNorm::type * get_temporary_array<AverageTileQNorm>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (AverageTileQNorm::type *) moments_arr->engCalibDeadHEC0;
    }


    template <>
    __host__ __device__ ShowerAxisX::type * get_temporary_array<ShowerAxisX>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (ShowerAxisX::type *) moments_arr->EMProbability;
    }
    template <>
    __host__ __device__ ShowerAxisY::type * get_temporary_array<ShowerAxisY>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (ShowerAxisY::type *) moments_arr->hadWeight;
    }
    template <>
    __host__ __device__ ShowerAxisZ::type * get_temporary_array<ShowerAxisZ>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (ShowerAxisZ::type *) moments_arr->OOCweight;
    }

    template <>
    __host__ __device__ typename NumPositiveEnergyCells::type * get_temporary_array<NumPositiveEnergyCells>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (NumPositiveEnergyCells::type *) moments_arr->vertexFraction;
    }

    template <>
    __host__ __device__ typename AbsoluteEnergyPerSample::type * get_temporary_array<AbsoluteEnergyPerSample>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (AbsoluteEnergyPerSample::type *) moments_arr->vertexFraction;
    }


    template <>
    __host__ __device__ LateralNormalization::type * get_temporary_array<LateralNormalization>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return moments_arr->DMweight;
    }
    template <>
    __host__ __device__ LongitudinalNormalization::type * get_temporary_array<LongitudinalNormalization>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return moments_arr->tileConfidenceLevel;
    }
    template <>
    __host__ __device__ MaxEnergyAndCellPerSample::type * get_temporary_array<MaxEnergyAndCellPerSample>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (MaxEnergyAndCellPerSample::type *) moments_arr->maxPhiPerSample;
    }

    template <>
    __host__ __device__ typename MaxECellPerSample::type * get_temporary_array<MaxECellPerSample>(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr)
    {
      return (MaxECellPerSample::type *) moments_arr->vertexFraction;
    }
  }

}

//The per-sample things might very well benefit from being
//struct-of-arrays instead of pure array-of-structs
//since we do the samplings in parallel.
//However, it'd be less clean when it came
//to the temporary array thing...

constexpr int WarpSize = 32;
//Let's do this per warp...
//In sufficiently new hardware,
//independent thread scheduling
//might very well ensure all the threads
//that are updating the same moments
//get executed together,
//while the warp-wide broadcasting
//will possibly improve memory accesses
//(and, in general, doing the moments
// in parallel will be a net performance gain
// when compared to have all of them being updated in a single thread...))

static_assert(NumSamplings <= 28, "We wrote the code under the assumption of 28 samplings at most.");

/******************************************************************************
 * First Pass                                                                 *
 ******************************************************************************/

__global__ static
void zerothClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                             Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                             const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                             const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;

  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int moment  = threadIdx.x % WarpSize;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cluster = index / WarpSize; cluster < cluster_number; cluster += grid_size / WarpSize)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }

      if (moment < NumSamplings)
        {
          const int sampling = moment;
          moments_arr->energyPerSample[sampling][cluster] = 0.f;
          moments_arr->nCellSampling[sampling][cluster] = 0;
          CMCHack::get_temporary_array< NumberEmptySamplings      >(moments_arr)[sampling][cluster] = 0;
          CMCHack::get_temporary_array< NumberNonEmptySamplings   >(moments_arr)[sampling][cluster] = 0;
          CMCHack::get_temporary_array< MaxMomentsEnergyPerSample >(moments_arr)[sampling][cluster] = 0;
        }
      else
        {
          switch (moment - NumSamplings)
            //NumSamplings == 28
            {
              case 0:
                moments_arr->centerX[cluster] = 0.f;
                moments_arr->centerY[cluster] = 0.f;
                moments_arr->centerZ[cluster] = 0.f;
                moments_arr->firstEngDens[cluster] = 0.f;
                moments_arr->secondEngDens[cluster] = 0.f;
                break;
              case 1:
                moments_arr->engFracEM[cluster] = 0.f;
                moments_arr->engPos[cluster] = 0.f;
                clusters_arr->clusterEnergy[cluster] = 0.f;
                CMCHack::get_temporary_array<SumAbsEnergyNonMoments>(moments_arr)[cluster] = 0.f;
                CMCHack::get_temporary_array<EnergyDensityNormalization>(moments_arr)[cluster] = 0.f;
                break;
              case 2:
                CMCHack::get_temporary_array<MX>(moments_arr)[cluster] = 0.f;
                CMCHack::get_temporary_array<MY>(moments_arr)[cluster] = 0.f;
                CMCHack::get_temporary_array<MaxCellEnergyAndCell>(moments_arr)[cluster] = 0ULL;
                CMCHack::get_temporary_array<SecondMaxCellEnergyAndCell>(moments_arr)[cluster] = 0ULL;
                clusters_arr->clusterEta[cluster] = 0.f;
                break;
              case 3:
                CMCHack::get_temporary_array<MZ>(moments_arr)[cluster] = 0.f;
                clusters_arr->clusterPhi[cluster] = 0.f;
                {
                  const int seed_cell = clusters_arr->seedCellID[cluster];
                  if (seed_cell >= 0 && seed_cell < NCaloCells)
                    {
                      CMCHack::get_temporary_array<SeedCellPhi>(moments_arr)[cluster] = geometry->phi[seed_cell];
                    }
                }
                break;
              default:
                break;
            }
        }
    }
}


__global__ static
void firstCellPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                         Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                         const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                         const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                         const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                         const bool use_abs_energy)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int in_warp_index = threadIdx.x % WarpSize;

  for (int cell = index / WarpSize; cell < NCaloCells; cell += grid_size / WarpSize)
    {
      const ClusterTag tag        = cell_state_arr->clusterTag[cell];
      const int        sampling   = geometry->sampling(cell);

      if (tag.is_part_of_cluster())
        {
          const float energy         = cell_info_arr->energy[cell];
          const float abs_energy     = fabsf(energy);
          const float moments_energy = ((use_abs_energy || energy > 0.f) ? abs_energy : 0.f);
          const float x              = geometry->x[cell];
          const float y              = geometry->y[cell];
          const float z              = geometry->z[cell];
          const float eta            = geometry->eta[cell];
          const float phi            = geometry->phi[cell];
          const float volume         = geometry->volume[cell];

          auto accumulateForCluster = [&](const int cluster, const float weight, const int this_moment)
          {
            const float weighted_energy = moments_energy * weight;
            switch (this_moment)
              {
                case 0:
                  atomicAdd(&(moments_arr->energyPerSample[sampling][cluster]), energy * weight);
                  break;
                case 1:
                  atomicAdd(&(moments_arr->nCellSampling[sampling][cluster]), 1);
                  break;
                case 2:
                  atomicMax(&(CMCHack::get_temporary_array<MaxMomentsEnergyPerSample>(moments_arr)[sampling][cluster]), __float_as_uint(weighted_energy));
                  break;
                case 3:
                  atomicAdd(&(moments_arr->centerX[cluster]), x * weighted_energy);
                  break;
                case 4:
                  atomicAdd(&(moments_arr->centerY[cluster]), y * weighted_energy);
                  break;
                case 5:
                  atomicAdd(&(moments_arr->centerZ[cluster]), z * weighted_energy);
                  break;
                case 6:
                  if ( sampling == CaloSampling::EMB1   ||
                       sampling == CaloSampling::EMB2   ||
                       sampling == CaloSampling::EMB3   ||
                       sampling == CaloSampling::EME1   ||
                       sampling == CaloSampling::EME2   ||
                       sampling == CaloSampling::EME3   ||
                       sampling == CaloSampling::FCAL0     )
                    {
                      atomicAdd(&(moments_arr->engFracEM[cluster]), weighted_energy);
                    }
                  break;
                case 7:
                  if (volume > 0)
                    {
                      const float w_E_over_V = weighted_energy / volume;
                      atomicAdd(&(moments_arr->firstEngDens[cluster]), weighted_energy * w_E_over_V);
                      atomicAdd(&(moments_arr->secondEngDens[cluster]), weighted_energy * w_E_over_V * w_E_over_V);
                      atomicAdd(&(CMCHack::get_temporary_array<EnergyDensityNormalization>(moments_arr)[cluster]), weighted_energy);
                    }
                  break;
                case 8:
                case 9:
                case 10:
                  {
                    const float dir = x * x + y * y + z * z;
                    const float r_dir = (dir > 0.f ? rsqrtf(dir) : dir);
                    const float w_E_r_dir = weighted_energy * r_dir;
                    if (this_moment == 8)
                      {
                        const float mx = w_E_r_dir * x;
                        atomicAdd(&(CMCHack::get_temporary_array<MX>(moments_arr)[cluster]), mx);
                      }
                    else if (this_moment == 9)
                      {
                        const float my = w_E_r_dir * y;
                        atomicAdd(&(CMCHack::get_temporary_array<MY>(moments_arr)[cluster]), my);
                      }
                    else /*if (this_moment == 10)*/
                      {
                        const float mz = w_E_r_dir * z;
                        atomicAdd(&(CMCHack::get_temporary_array<MZ>(moments_arr)[cluster]), mz);
                      }
                  }
                  break;
                case 11:
                  atomicAdd(&(moments_arr->engPos[cluster]), weighted_energy);
                  break;
                case 12:
                  if (weighted_energy > 0)
                    {
                      unsigned long long int energy_and_cell = __float_as_uint(weighted_energy);
                      //Energy is positive, so no need to switch to total ordering...
                      energy_and_cell = (energy_and_cell << 32) | (cell + 1);

                      const unsigned long long int old_enc = atomicMax(&(CMCHack::get_temporary_array<MaxCellEnergyAndCell>(moments_arr)[cluster]), energy_and_cell);
                      atomicMax(&(CMCHack::get_temporary_array<SecondMaxCellEnergyAndCell>(moments_arr)[cluster]), min(old_enc, energy_and_cell));
                    }
                  break;
                case 13:
                  atomicAdd(&(clusters_arr->clusterEnergy[cluster]), energy * weight);
                  atomicAdd(&(CMCHack::get_temporary_array<SumAbsEnergyNonMoments>(moments_arr)[cluster]), abs_energy * weight);
                  break;
                case 14:
                  atomicAdd(&(clusters_arr->clusterEta[cluster]), abs_energy * weight * eta);
                  break;
                case 15:
                  {
                    const float phi_0 = CMCHack::get_temporary_array<SeedCellPhi>(moments_arr)[cluster];
                    const float phi_real = Helpers::regularize_angle(phi, phi_0);
                    atomicAdd(&(clusters_arr->clusterPhi[cluster]), phi_real * abs_energy * weight);
                  }
                  break;
                default:
                  break;
              }
          };

          if (tag.is_shared_between_clusters())
            {
              const float secondary_weight = __uint_as_float(tag.secondary_cluster_weight());
              if (in_warp_index >= WarpSize / 2)
                {
                  accumulateForCluster(tag.secondary_cluster_index(), secondary_weight, in_warp_index - WarpSize / 2);
                }
              else
                {
                  accumulateForCluster(tag.cluster_index(), 1.0f - secondary_weight, in_warp_index);
                }
            }
          else
            {
              accumulateForCluster(tag.cluster_index(), 1.0f, in_warp_index);
            }
        }

      //Optimization for our neighbour handling:
      //since we are using all2D,
      //which assures max neighbours < 32,
      //and which also encompasses
      //all of its smaller options,
      //we can have each thread knowing where to check
      //instead of building an actual neighbour list.
      //Also, this is symmetric,
      //that is, A being in the list of neighbours of B
      //implies B is in the list of neighbours of A.

      constexpr int neighbour_option_num = 4;

      const int num_relevant_neighbours = NeighOffset(geometry->neighbours.offsets[cell]).get_end_cell(neighbour_option_num);

      int cluster_to_check = 0;

      if (in_warp_index < num_relevant_neighbours)
        {
          const unsigned int mask = (1U << num_relevant_neighbours) - 1;

          const int neigh = geometry->neighbours.get_neighbour(cell, in_warp_index);
          const ClusterTag neigh_tag = cell_state_arr->clusterTag[neigh];

          if (neigh_tag.is_part_of_cluster())
            {
              cluster_to_check = neigh_tag.cluster_index() + 1;
              //We add 1 because 0 means no cluster to check here.
              //Also, since the cluster indices are 16 bit,
              //of course there's no issue here.
            }

          for (int thread_to_check = 0; thread_to_check < num_relevant_neighbours; ++thread_to_check)
            {
              const int other = __shfl_sync(mask, cluster_to_check, thread_to_check);
              cluster_to_check *= 1 - 2 * (thread_to_check < in_warp_index && abs(other) == cluster_to_check);
              //Potentially mark this cluster as already considered.
            }

          //Maybe there is a solution that uses sorting instead?
          //Best choice here would be a sorting network
          //(bitonic sorting or Batcher's odd-even),
          //but then we'd need to eliminate the equal clusters too...
          //All in all, probably something like C ln(n)(ln(n) + 1) + 2 operations
          //per thread, with C somewhere around 2 rather than 0.5.
          //And, with n = 32, does the added complexity
          //really justify this? I would need a good reason
          //before considering to implement that sort of thing,
          //and I strongly suspect the performance benefits
          //not to be that significant. Still...

          if (cluster_to_check > 0)
            //Valid and non-repeated.
            {
              const int neigh_cluster = cluster_to_check - 1;
              if (tag.is_part_of_cluster())
                {
                  if (tag.cluster_index() != neigh_cluster)
                    {
                      atomicAdd(&(CMCHack::get_temporary_array<NumberNonEmptySamplings>(moments_arr)[sampling][neigh_cluster]), 1);
                    }
                }
              else
                {
                  atomicAdd(&(CMCHack::get_temporary_array<NumberEmptySamplings>(moments_arr)[sampling][neigh_cluster]), 1);
                }
            }
        }
    }
}

//Finalize/normalize the first pass moments,
//zero out what is needed for the second pass...

__global__ static
void firstClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                            Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;

  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int moment  = threadIdx.x % WarpSize;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cluster = index / WarpSize; cluster < cluster_number; cluster += grid_size / WarpSize)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }
      const float sum_energies = moments_arr->engPos[cluster];
      if (moment < NumSamplings)
        {
          const int sampling = moment;
          const float sampling_energy = moments_arr->energyPerSample[sampling][cluster];
          const unsigned int max_energy_pattern = CMCHack::get_temporary_array<MaxMomentsEnergyPerSample>(moments_arr)[sampling][cluster];
          const float sampling_max_energy = __uint_as_float(max_energy_pattern);
          const int sampling_empty = CMCHack::get_temporary_array<NumberEmptySamplings>(moments_arr)[sampling][cluster];
          const int sampling_non_empty = CMCHack::get_temporary_array<NumberNonEmptySamplings>(moments_arr)[sampling][cluster];

          const float total = sampling_empty + sampling_non_empty;

          float isolation = 0.f, isolation_norm = 0.f, eng_frac_core = sampling_max_energy;

          if (total > 0.f && sampling_energy > 0.f)
            {
              isolation = (sampling_energy * sampling_empty) / total;
              isolation_norm = sampling_energy;
            }

          const unsigned int mask = 0x0FFFFFFFU;
          //28 samplings, so without the last 4 threads.

          for (int i = 1; i < WarpSize; i *= 2)
            {
              const int origin = sampling ^ i;
              const float other_isol = __shfl_xor_sync(mask, isolation, i) * (origin < NumSamplings);
              const float other_isol_norm = __shfl_xor_sync(mask, isolation_norm, i) * (origin < NumSamplings);
              const float other_efc = __shfl_xor_sync(mask, eng_frac_core, i) * (origin < NumSamplings);
              isolation += other_isol;
              isolation_norm += other_isol_norm;
              eng_frac_core += other_efc;
            }

          switch (moment)
            {
              case 0:
                moments_arr->isolation[cluster] = (isolation_norm != 0.f ? isolation / isolation_norm : 0.f);
                break;
              case NumSamplings - 1:
                moments_arr->engFracCore[cluster] = (sum_energies > 0.f) * eng_frac_core / sum_energies;
                break;
              default:
                break;
            }
        }
      else
        {
          switch (moment - NumSamplings)
            {
              case 0:
                {
                  const float rev_sum_energies = 1.0f / (sum_energies > 0.f ? sum_energies : 1.f);
                  moments_arr->centerX[cluster] *= rev_sum_energies;
                  moments_arr->centerY[cluster] *= rev_sum_energies;
                  moments_arr->centerZ[cluster] *= rev_sum_energies;
                  const float energy_density_norm = CMCHack::get_temporary_array<EnergyDensityNormalization>(moments_arr)[cluster];
                  const float rev_energy_density_norm = 1.f / (energy_density_norm > 0.f ? energy_density_norm : 1.f);
                  moments_arr->firstEngDens[cluster]  *= rev_energy_density_norm;
                  moments_arr->secondEngDens[cluster] *= rev_energy_density_norm;
                }
                break;
              case 1:
                {
                  moments_arr->engFracEM[cluster] /= (sum_energies > 0.f ? sum_energies : 1.f);

                  const float mx = CMCHack::get_temporary_array<MX>(moments_arr)[cluster];
                  const float my = CMCHack::get_temporary_array<MY>(moments_arr)[cluster];
                  const float mz = CMCHack::get_temporary_array<MZ>(moments_arr)[cluster];
                  const float sqrd_mass = sum_energies * sum_energies - mx * mx - my * my - mz * mz;

                  moments_arr->mass[cluster] = sqrtf(fabsf(sqrd_mass)) * ((sqrd_mass > 0.f) - (sqrd_mass < 0.f));
                }
                break;
              case 2:
                {
                  unsigned long long int max_E_and_cell = CMCHack::get_temporary_array<MaxCellEnergyAndCell>(moments_arr)[cluster];
                  const int max_cell = (max_E_and_cell & 0x7FFFFFFF) - 1;
                  const float max_E = __uint_as_float(max_E_and_cell >> 32);
                  moments_arr->engFracMax[cluster] = max_E / (sum_energies > 0.f ? sum_energies : 1.f);
                  CMCHack::get_temporary_array<MaxCells>(moments_arr)[cluster] = max_cell;

                  const unsigned long long int second_max_E_and_cell = CMCHack::get_temporary_array<SecondMaxCellEnergyAndCell>(moments_arr)[cluster];
                  const int second_max_cell = (second_max_E_and_cell & 0x7FFFFFFF) - 1;
                  CMCHack::get_temporary_array<SecondMaxCells>(moments_arr)[cluster] = second_max_cell;
                }
                break;
              case 3:
                {
                  const float abs_energy = CMCHack::get_temporary_array<SumAbsEnergyNonMoments>(moments_arr)[cluster];
                  if (abs_energy > 0)
                    {
                      const float tempeta = clusters_arr->clusterEta[cluster] / abs_energy;

                      clusters_arr->clusterEta[cluster] = tempeta;

                      const float temp_ET = clusters_arr->clusterEnergy[cluster] / coshf(abs(tempeta));

                      clusters_arr->clusterEt[cluster] = temp_ET;
                      clusters_arr->clusterPhi[cluster] = Helpers::regularize_angle(clusters_arr->clusterPhi[cluster] / abs_energy, 0.f);
                    }
                }
                break;
              default:
                break;
            }
        }

      switch (moment)
        {
          //Avoid 0 and NumSamplings - 1
          //since they set the sampling things...
          case 1:
            moments_arr->engBadCells[cluster] = 0.f;
            break;
          case 2:
            moments_arr->nBadCells[cluster] = 0;
            break;
          case 3:
            moments_arr->nBadCellsCorr[cluster] = 0;
            break;
          case 4:
            moments_arr->badCellsCorrE[cluster] = 0.f;
            break;
          case 5:
            moments_arr->badLArQFrac[cluster] = 0.f;
            break;
          case 6:
            moments_arr->avgLArQ[cluster] = 0.f;
            break;
          case 7:
            moments_arr->avgTileQ[cluster] = 0.f;
            break;
          case 8:
            moments_arr->PTD[cluster] = 0.f;
            break;
          case 9:
            moments_arr->numCells[cluster] = 0;
            break;
          case 10:
            CMCHack::get_temporary_array<NumPositiveEnergyCells>(moments_arr)[cluster] = 0;
            break;
          case 11:
            CMCHack::get_temporary_array<SumSquareEnergies>(moments_arr)[cluster] = 0.f;
            break;
          case 12:
            CMCHack::get_temporary_array<Matrix22>(moments_arr)[cluster] = 0.f;
            break;
          case 13:
            CMCHack::get_temporary_array<Matrix21>(moments_arr)[cluster] = 0.f;
            break;
          case 14:
            CMCHack::get_temporary_array<Matrix11>(moments_arr)[cluster] = 0.f;
            break;
          case 15:
            CMCHack::get_temporary_array<Matrix20>(moments_arr)[cluster] = 0.f;
            break;
          case 16:
            CMCHack::get_temporary_array<Matrix10>(moments_arr)[cluster] = 0.f;
            break;
          case 17:
            CMCHack::get_temporary_array<Matrix00>(moments_arr)[cluster] = 0.f;
            break;
          case 18:
            moments_arr->time[cluster] = 0.f;
            break;
          case 19:
            moments_arr->secondTime[cluster] = 0.f;
            break;
          case 20:
            CMCHack::get_temporary_array<TimeNormalization>(moments_arr)[cluster] = 0.f;
            break;
          case 21:
            CMCHack::get_temporary_array<AverageLArQNorm>(moments_arr)[cluster] = 0.f;
            break;
          case 22:
            CMCHack::get_temporary_array<AverageTileQNorm>(moments_arr)[cluster] = 0.f;
            break;
          case 23:
            CMCHack::get_temporary_array<MaxSignificanceAndSampling>(moments_arr)[cluster] = 0ULL;
            break;
          case 24:
            moments_arr->significance[cluster] = 0.f;
            break;
          default:
            break;
        }
    }
}

/******************************************************************************
 * Second pass.                                                               *
 ******************************************************************************/

__global__ static
void secondCellPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                          const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                          const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                          const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                          const Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                          const bool use_abs_energy, const bool use_two_gaussian_noise,
                          const float min_LAr_quality)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int in_warp_index = threadIdx.x % WarpSize;

  for (int cell = index / WarpSize; cell < NCaloCells; cell += grid_size / WarpSize)
    {
      const ClusterTag tag = cell_state_arr->clusterTag[cell];
      if (tag.is_part_of_cluster())
        {
          const float energy              = cell_info_arr->energy[cell];
          const float abs_energy          = fabsf(energy);
          const float moments_energy      = ((use_abs_energy || energy > 0.f) ? abs_energy : 0.f);
          const float time                = cell_info_arr->time[cell];
          const float x                   = geometry->x[cell];
          const float y                   = geometry->y[cell];
          const float z                   = geometry->z[cell];
          const bool  is_tile             = geometry->is_tile(cell);
          const int   sampling            = geometry->sampling(cell);
          const int   gain                = cell_info_arr->gain[cell];
          //No need to check for invalid cells as they won't be part of clusters...
          const float noise               = ( is_tile && use_two_gaussian_noise                        ?
                                              noise_arr->get_double_gaussian_noise(cell, gain, energy) :
                                              noise_arr->get_noise(cell, gain)                            );

          const QualityProvenance qp      = cell_info_arr->qualityProvenance[cell];
          const bool              is_bad  = cell_info_arr->is_bad(is_tile, qp, false);

          auto accumulateForCluster = [&](const int cluster, const float weight, const int this_moment)
          {
            const float weighted_energy = moments_energy * weight;
            const float weighted_energy_or_negative = (use_abs_energy ? fabsf(energy) : energy) * weight;
            const float square_w_E      = weighted_energy * weighted_energy;
            const float center_x        = moments_arr->centerX[cluster];
            const float center_y        = moments_arr->centerY[cluster];
            const float center_z        = moments_arr->centerZ[cluster];
            switch (this_moment)
              {
                case 0:
                  if (is_bad)
                    {
                      atomicAdd(&(moments_arr->engBadCells[cluster]), weighted_energy_or_negative);
                      atomicAdd(&(moments_arr->nBadCells[cluster]), 1);
                    }
                  break;
                case 1:
                  if (is_bad && energy != 0.f)
                    {
                      atomicAdd(&(moments_arr->badCellsCorrE[cluster]), weighted_energy_or_negative);
                      atomicAdd(&(moments_arr->nBadCellsCorr[cluster]), 1);
                    }
                  break;
                case 2:
                  if (!is_bad && !is_tile && ((qp.provenance() & 0x2800U) == 0x2000U) && qp.quality() > min_LAr_quality)
                    {
                      atomicAdd(&(moments_arr->badLArQFrac[cluster]), weighted_energy_or_negative);
                    }
                  break;
                case 3:
                  if (!is_bad && !is_tile && ((qp.provenance() & 0x2800U) == 0x2000U))
                    {
                      const float square_E_or_neg = weighted_energy_or_negative * weighted_energy_or_negative;
                      const float quality = qp.quality();
                      atomicAdd(&(moments_arr->avgLArQ[cluster]), square_E_or_neg * quality);
                      atomicAdd(&(CMCHack::get_temporary_array<AverageLArQNorm>(moments_arr)[cluster]), square_E_or_neg);
                    }
                  break;
                case 4:
                  if (!is_bad && is_tile && qp.tile_qual1() != 0xFFU && qp.tile_qual2() != 0xFFU)
                    {
                      const float square_E_or_neg = weighted_energy_or_negative * weighted_energy_or_negative;
                      const float max_quality = max((unsigned int) qp.tile_qual1(), (unsigned int) qp.tile_qual2());
                      atomicAdd(&(moments_arr->avgTileQ[cluster]), square_E_or_neg * max_quality);
                      atomicAdd(&(CMCHack::get_temporary_array<AverageTileQNorm>(moments_arr)[cluster]), square_E_or_neg);
                    }
                  break;
                case 5:
                  atomicAdd(&(moments_arr->PTD[cluster]), square_w_E);

                  //Comment on there:
                  //
                  //  +--------------- begin comment on there ---------------+
                  //  |                                                      |
                  //  | do not convert to pT since clusters are small and    |
                  //  | there is virtually no difference and cosh just costs |
                  //  | time ...                                             |
                  //  |                                                      |
                  //  +---------------- end comment on there ----------------+
                  //
                  //So maybe we could change this here?

                  atomicAdd(&(moments_arr->numCells[cluster]), 1);

                  if (weighted_energy_or_negative > 0)
                    {
                      atomicAdd(&(CMCHack::get_temporary_array<NumPositiveEnergyCells>(moments_arr)[cluster]), 1);
                    }

                  break;
                case 6:
                  atomicAdd(&(CMCHack::get_temporary_array<Matrix00>(moments_arr)[cluster]), square_w_E * (x - center_x) * (x - center_x));
                  break;
                case 7:
                  atomicAdd(&(CMCHack::get_temporary_array<Matrix10>(moments_arr)[cluster]), square_w_E * (x - center_x) * (y - center_y));
                  break;
                case 8:
                  atomicAdd(&(CMCHack::get_temporary_array<Matrix20>(moments_arr)[cluster]), square_w_E * (x - center_x) * (z - center_z));
                  break;
                case 9:
                  atomicAdd(&(CMCHack::get_temporary_array<Matrix11>(moments_arr)[cluster]), square_w_E * (y - center_y) * (y - center_y));
                  break;
                case 10:
                  atomicAdd(&(CMCHack::get_temporary_array<Matrix21>(moments_arr)[cluster]), square_w_E * (y - center_y) * (z - center_z));
                  break;
                case 11:
                  atomicAdd(&(CMCHack::get_temporary_array<Matrix22>(moments_arr)[cluster]), square_w_E * (z - center_z) * (z - center_z));
                  break;
                case 12:
                  atomicAdd(&(CMCHack::get_temporary_array<SumSquareEnergies>(moments_arr)[cluster]), square_w_E);
                  break;
                //Note: exclusion of PreSamplerB/E.
                case 13:
                  if ( ((is_tile && (qp.provenance() & 0x8080U)) || (!is_tile && (qp.provenance() & 0x2000U))) &&
                       sampling != CaloSampling::PreSamplerB && sampling != CaloSampling::PreSamplerE          )
                    {
                      const float normE = weight * energy;
                      const float squared_norm = normE * normE;
                      atomicAdd(&(moments_arr->time[cluster]), time * squared_norm);
                      atomicAdd(&(moments_arr->secondTime[cluster]), time * time * squared_norm);
                      atomicAdd(&(CMCHack::get_temporary_array<TimeNormalization>(moments_arr)[cluster]), squared_norm);
                    }
                  break;
                case 14:
                  atomicAdd(&(moments_arr->significance[cluster]), noise * noise);
                  break;
                case 15:
                  {
                    const float max_sig = noise > 0.f ? weighted_energy_or_negative / noise : 0.f;
                    unsigned long long int max_S_and_S = __float_as_uint(fabsf(max_sig));
                    max_S_and_S = max_S_and_S << 32 | (((unsigned long long int) sampling << 1)) | (max_sig > 0);
                    atomicMax(&(CMCHack::get_temporary_array<MaxSignificanceAndSampling>(moments_arr)[cluster]), max_S_and_S);
                  }
                  break;
                default:
                  break;
              }
          };

          if (tag.is_shared_between_clusters())
            {
              const float secondary_weight = __uint_as_float(tag.secondary_cluster_weight());
              if (in_warp_index >= WarpSize / 2)
                {
                  accumulateForCluster(tag.secondary_cluster_index(), secondary_weight, in_warp_index - WarpSize / 2);
                }
              else
                {
                  accumulateForCluster(tag.cluster_index(), 1.0f - secondary_weight, in_warp_index);
                }
            }
          else
            {
              accumulateForCluster(tag.cluster_index(), 1.0f, in_warp_index);
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

      if (CMCHack::get_temporary_array<NumPositiveEnergyCells>(moments_arr)[cluster] > 2)
        {
          const float norm = 1.f / CMCHack::get_temporary_array<SumSquareEnergies>(moments_arr)[cluster];
          RealSymmetricMatrixSolver solver { CMCHack::get_temporary_array<Matrix00>(moments_arr)[cluster] * norm,
                                             CMCHack::get_temporary_array<Matrix11>(moments_arr)[cluster] * norm,
                                             CMCHack::get_temporary_array<Matrix22>(moments_arr)[cluster] * norm,
                                             CMCHack::get_temporary_array<Matrix10>(moments_arr)[cluster] * norm,
                                             CMCHack::get_temporary_array<Matrix21>(moments_arr)[cluster] * norm,
                                             CMCHack::get_temporary_array<Matrix20>(moments_arr)[cluster] * norm  };

          float lambdas[3], vecs[3][3];

          solver.get_solution(lambdas, vecs, true);
          

          constexpr float min_lambdas = 1.e-6f;

          if (fabsf(lambdas[0]) >= min_lambdas && fabsf(lambdas[1]) >= min_lambdas && fabsf(lambdas[2]) >= min_lambdas)
            {
              int chosen_vec = -1;
              float prev_angle = 9e99;
              for (int i = 0; i < 3; ++i)
                {
                  const float prod = (vecs[i][0] * axis_x + vecs[i][1] * axis_y + vecs[i][2] * axis_z);
                  float raw_angle = acosf(prod > 1.f ? 1.f : (prod < -1.f ? -1.f : prod));

                  bool reverse = raw_angle > Helpers::Constants::pi<float> / 2;

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
            }
        }
      CMCHack::get_temporary_array<ShowerAxisX>(moments_arr)[cluster] = axis_x;
      CMCHack::get_temporary_array<ShowerAxisY>(moments_arr)[cluster] = axis_y;
      CMCHack::get_temporary_array<ShowerAxisZ>(moments_arr)[cluster] = axis_z;
      moments_arr->deltaPhi[cluster] = delta_phi;
      moments_arr->deltaTheta[cluster] = delta_theta;
      moments_arr->deltaAlpha[cluster] = delta_alpha;
    }
}

__global__ static
void secondClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                             Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                             const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;

  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int moment  = threadIdx.x % WarpSize;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cluster = index / WarpSize; cluster < cluster_number; cluster += grid_size / WarpSize)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }

      const float sum_energies = moments_arr->engPos[cluster];
      const float cluster_energy = clusters_arr->clusterEnergy[cluster];

      if (moment < NumSamplings)
        {
          const int sampling = moment;
          CMCHack::get_temporary_array<MaxEnergyAndCellPerSample>(moments_arr)[sampling][cluster] = 0;
          moments_arr->etaPerSample[sampling][cluster] = 0.f;
          moments_arr->phiPerSample[sampling][cluster] = 0.f;
          CMCHack::get_temporary_array<AbsoluteEnergyPerSample>(moments_arr)[sampling][cluster] = 0.f;
        }
      else
        {
          switch (moment - NumSamplings)
            {
              case 0:
                {
                  if (moments_arr->numCells[cluster] <= 0)
                    {
                      clusters_arr->seedCellID[cluster] = -1;
                    }
                  moments_arr->badLArQFrac[cluster] /= (cluster_energy != 0.f ? cluster_energy : 1.f);
                  const float prev_v = moments_arr->significance[cluster];
                  moments_arr->significance[cluster] = (prev_v > 0.f ? cluster_energy * rsqrtf(prev_v) : 0.f);
                  moments_arr->firstPhi[cluster] = 0.f;
                  CMCHack::get_temporary_array<LateralNormalization>(moments_arr)[cluster] = 0.f;
                }
                break;
              case 1:
                {
                  const unsigned long long int max_sig_and_samp = CMCHack::get_temporary_array<MaxSignificanceAndSampling>(moments_arr)[cluster];
                  const float max_sig = __uint_as_float(max_sig_and_samp >> 32);
                  const int max_samp = (max_sig_and_samp & 0xFFFFFFFEU) >> 1;
                  moments_arr->cellSignificance[cluster] = max_sig * (max_sig_and_samp & 1 ? 1.f : -1.f);
                  moments_arr->cellSigSampling[cluster] = max_samp;
                  moments_arr->firstEta[cluster] = 0.f;
                  moments_arr->secondR[cluster] = 0.f;
                  CMCHack::get_temporary_array<LongitudinalNormalization>(moments_arr)[cluster] = 0.f;
                }
                break;
              case 2:
                {
                  const float norm_LAr = CMCHack::get_temporary_array<AverageLArQNorm>(moments_arr)[cluster];
                  moments_arr->avgLArQ[cluster] /= (norm_LAr > 0.f ? norm_LAr : 1.0f);
                  const float norm_Tile = CMCHack::get_temporary_array<AverageTileQNorm>(moments_arr)[cluster];
                  moments_arr->avgTileQ[cluster] /= (norm_Tile > 0.f ? norm_Tile : 1.0f);
                  moments_arr->secondLambda[cluster] = 0.f;
                  moments_arr->lateral[cluster] = 0.f;
                }
                break;
              case 3:
                {
                  const float old = moments_arr->PTD[cluster];
                  moments_arr->PTD[cluster] = 1.0f / ((sum_energies > 0.f ? sum_energies : 1.f) * rsqrtf(old));
                  //See before: maybe to be revised?
                  const float time_norm = CMCHack::get_temporary_array<TimeNormalization>(moments_arr)[cluster];
                  if (time_norm != 0.f)
                    {
                      const float real_norm = 1.0f / time_norm;
                      const float time = moments_arr->time[cluster] * real_norm;
                      const float second_sum = moments_arr->secondTime[cluster];
                      moments_arr->time[cluster] = time;
                      moments_arr->secondTime[cluster] = (second_sum * real_norm) - (time * time);
                    }
                  else
                    {
                      moments_arr->time[cluster] = 0.f;
                      moments_arr->secondTime[cluster] = 0.f;
                    }
                  moments_arr->longitudinal[cluster] = 0.f;
                  moments_arr->nExtraCellSampling[cluster] = 0;
                }
                break;
              default:
                break;
            }
        }
    }
}

/******************************************************************************
 * Third pass.                                                                *
 ******************************************************************************/

__global__ static
void thirdCellPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                         const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                         const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                         const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                         const bool use_abs_energy, const float eta_inner_wheel,
                         const float min_l_longitudinal, const float min_r_lateral)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  const int grid_size = gridDim.x * blockDim.x;
  const int in_warp_index = threadIdx.x % WarpSize;

  for (int cell = index / WarpSize; cell < NCaloCells; cell += grid_size / WarpSize)
    {
      const ClusterTag tag = cell_state_arr->clusterTag[cell];
      if (tag.is_part_of_cluster())
        {
          const float energy         = cell_info_arr->energy[cell];
          const float abs_energy     = fabsf(energy);
          const float moments_energy = ((use_abs_energy || energy > 0.f) ? abs_energy : 0.f);
          const float x              = geometry->x[cell];
          const float y              = geometry->y[cell];
          const float z              = geometry->z[cell];
          const float eta            = geometry->eta[cell];
          const float phi            = geometry->phi[cell];
          const int   sampling       = geometry->sampling(cell);

          auto accumulateForCluster = [&](const int cluster, const float weight, const int this_moment)
          {
            const float weighted_energy = moments_energy * weight;
            const float center_x        = moments_arr->centerX[cluster];
            const float center_y        = moments_arr->centerY[cluster];
            const float center_z        = moments_arr->centerZ[cluster];
            const float axis_x          = CMCHack::get_temporary_array<ShowerAxisX>(moments_arr)[cluster];
            const float axis_y          = CMCHack::get_temporary_array<ShowerAxisY>(moments_arr)[cluster];
            const float axis_z          = CMCHack::get_temporary_array<ShowerAxisZ>(moments_arr)[cluster];
            const int   max_cell        = CMCHack::get_temporary_array<MaxCells>(moments_arr)[cluster];
            const int   second_max_cell = CMCHack::get_temporary_array<SecondMaxCells>(moments_arr)[cluster];

            auto dot_p = [](const float x1, const float x2, const float x3,
                            const float y1, const float y2, const float y3)
            {
              return x1 * y1 + x2 * y2 + x3 * y3;
            };

            //d\vec{v} = \vec{r}_{cell} - \vec{r}_{center}
            //
            //  r      = ||d\vec{v} \cross \vec{axis}||
            //  lambda = d\vec{v} \dot \vec{axis}
            //
            //  lambda = ||d\vec{v}||  cos (\theta)
            //  r      = ||d\vec{v}|| |sin(\theta)|
            //
            //  |sin(\theta)| = sqrt(1 - cos(\theta)^2)

            const float dx = x - center_x;
            const float dy = y - center_y;
            const float dz = z - center_z;

            const float d_mag_inv = rnorm3df(dx, dy, dz);

            const float lambda = dot_p(dx, dy, dz, axis_x, axis_y, axis_z);

            const float cos_theta = lambda * d_mag_inv;

            const float r = 1.0f / (rsqrtf(1 - cos_theta * cos_theta) * d_mag_inv);

            switch (this_moment)
              {
                case 0:
                  {
                    const float phi_0 = CMCHack::get_temporary_array<SeedCellPhi>(moments_arr)[cluster];
                    const float phi_real = Helpers::regularize_angle(phi, phi_0);
                    atomicAdd(&(moments_arr->firstPhi[cluster]), weighted_energy * phi_real);
                  }
                  break;
                case 1:
                  atomicAdd(&(moments_arr->firstEta[cluster]), weighted_energy * eta);
                  break;
                case 2:
                  atomicAdd(&(moments_arr->secondR[cluster]), weighted_energy * r * r);
                  break;
                case 3:
                  atomicAdd(&(moments_arr->secondLambda[cluster]), weighted_energy * lambda * lambda);
                  break;
                case 4:
                  atomicAdd(&(moments_arr->etaPerSample[sampling][cluster]), abs_energy * weight * eta);
                  break;
                case 5:
                  {
                    const float phi_0 = CMCHack::get_temporary_array<SeedCellPhi>(moments_arr)[cluster];
                    const float phi_real = Helpers::regularize_angle(phi, phi_0);
                    atomicAdd(&(moments_arr->phiPerSample[sampling][cluster]), abs_energy * weight * phi_real);
                  }
                  break;
                case 6:
                  atomicAdd(&(CMCHack::get_temporary_array<AbsoluteEnergyPerSample>(moments_arr)[sampling][cluster]), abs_energy * weight);
                  break;
                case 7:
                  if (sampling == CaloSampling::EME2 && fabsf(eta) > eta_inner_wheel)
                    {
                      atomicAdd(&(moments_arr->nExtraCellSampling[cluster]), 1);
                    }
                  break;
                case 8:
                  if (cell != max_cell && cell != second_max_cell)
                    {
                      atomicAdd(&(moments_arr->lateral[cluster]), weighted_energy * r * r);
                    }
                  break;
                case 9:
                  if (cell != max_cell && cell != second_max_cell)
                    {
                      atomicAdd(&(CMCHack::get_temporary_array<LateralNormalization>(moments_arr)[cluster]), weighted_energy * r * r);
                    }
                  else
                    {
                      const float real_r = max(r, min_r_lateral);
                      atomicAdd(&(CMCHack::get_temporary_array<LateralNormalization>(moments_arr)[cluster]), weighted_energy * real_r * real_r);
                    }
                  break;
                case 10:
                  if (cell != max_cell && cell != second_max_cell)
                    {
                      atomicAdd(&(moments_arr->longitudinal[cluster]), weighted_energy * lambda * lambda);
                    }
                  break;
                case 11:
                  if (cell != max_cell && cell != second_max_cell)
                    {
                      atomicAdd(&(CMCHack::get_temporary_array<LongitudinalNormalization>(moments_arr)[cluster]), weighted_energy * lambda * lambda);
                    }
                  else
                    {
                      const float real_lambda = max(lambda, min_l_longitudinal);
                      atomicAdd(&(CMCHack::get_temporary_array<LongitudinalNormalization>(moments_arr)[cluster]), weighted_energy * real_lambda * real_lambda);
                    }
                  break;
                case 12:
                  {
                    const unsigned int energy_pattern = __float_as_uint(energy * weight);
                    unsigned long long int E_and_cell = FloatingPointHelpers::StandardFloat::to_total_ordering(energy_pattern);
                    E_and_cell = (E_and_cell << 32) | cell;
                    atomicMax(&(CMCHack::get_temporary_array<MaxEnergyAndCellPerSample>(moments_arr)[sampling][cluster]), E_and_cell);
                  }
                  break;
                default:
                  break;
              }
          };

          if (tag.is_shared_between_clusters())
            {
              const float secondary_weight = __uint_as_float(tag.secondary_cluster_weight());
              if (in_warp_index >= WarpSize / 2)
                {
                  accumulateForCluster(tag.secondary_cluster_index(), secondary_weight, in_warp_index - WarpSize / 2);
                }
              else
                {
                  accumulateForCluster(tag.cluster_index(), 1.0f - secondary_weight, in_warp_index);
                }
            }
          else
            {
              accumulateForCluster(tag.cluster_index(), 1.0f, in_warp_index);
            }
        }
    }
}

__global__ static
void thirdClusterPassKernel(Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                            Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const bool skip_invalid)
{
  const int cluster_number = clusters_arr->number;

  const int index   = blockIdx.x * blockDim.x + threadIdx.x;
  const int moment  = threadIdx.x % WarpSize;
  const int grid_size = gridDim.x * blockDim.x;

  for (int cluster = index / WarpSize; cluster < cluster_number; cluster += grid_size / WarpSize)
    {
      if (skip_invalid && clusters_arr->seedCellID[cluster] < 0)
        {
          continue;
        }
      const float sum_energies = moments_arr->engPos[cluster];
      if (moment < NumSamplings)
        {
          const int sampling = moment;
          const float sampling_normalization = CMCHack::get_temporary_array<AbsoluteEnergyPerSample>(moments_arr)[sampling][cluster];

          const unsigned long long int energy_and_cell = CMCHack::get_temporary_array<MaxEnergyAndCellPerSample>(moments_arr)[sampling][cluster];

          const int cell = sampling_normalization > 0.f ? (int) (energy_and_cell & 0x7FFFFFFF) : -1;

          CMCHack::get_temporary_array<MaxECellPerSample>(moments_arr)[sampling][cluster] = cell;

          const float eta_phi_normalization = 1.0f / (sampling_normalization != 0.f ? sampling_normalization : 1.0f);

          moments_arr->etaPerSample[sampling][cluster] *= eta_phi_normalization;

          const float old_phi = moments_arr->phiPerSample[sampling][cluster];

          moments_arr->phiPerSample[sampling][cluster] = Helpers::regularize_angle(old_phi * eta_phi_normalization, 0.f);
        }
      else
        {
          const int thread_id = moment - NumSamplings;

          const float center_x        = moments_arr->centerX[cluster];
          const float center_y        = moments_arr->centerY[cluster];
          const float center_z        = moments_arr->centerZ[cluster];
          const float axis_x          = CMCHack::get_temporary_array<ShowerAxisX>(moments_arr)[cluster];
          const float axis_y          = CMCHack::get_temporary_array<ShowerAxisY>(moments_arr)[cluster];
          const float axis_z          = CMCHack::get_temporary_array<ShowerAxisZ>(moments_arr)[cluster];

          const float center_phi = Helpers::regularize_angle(atan2f(center_y, center_x));

          const float center_eta = Helpers::eta_from_coordinates(center_x, center_y, center_z);

          const int first_attempt_cell = geometry->get_closest_cell(CaloSampling::EMB1, center_eta, center_phi);

          float lambda_c = 0.f;

          if (first_attempt_cell >= 0)
            //Condition on CPU is r_calo == 0,
            //but, by definition, I'd expect no cell
            //to potentially overlap with the center
            //of the detector, right?!
            {
              if (thread_id == 0)
                {
                  const float r_calo = geometry->r[first_attempt_cell] - geometry->dr[first_attempt_cell] /
                                       (geometry->is_tile(first_attempt_cell) ? 2.f : 1.f);

                  const float axis_r = axis_x * axis_x + axis_y * axis_y;

                  if (axis_r > 0)
                    {
                      const float rev_axis_r = 1.0f / axis_r;
                      const float axis_and_center_r = axis_x * center_x + axis_y * center_y;
                      const float center_r = center_x * center_x + center_y * center_y - r_calo * r_calo;
                      const float det = axis_and_center_r * axis_and_center_r * (rev_axis_r * rev_axis_r) - center_r * rev_axis_r;
                      if (det > 0)
                        {
                          const float quot = -axis_and_center_r * rev_axis_r;
                          const float rootdet = sqrtf(det);
                          const float branch_1 = quot + rootdet;
                          const float branch_2 = quot - rootdet;
                          lambda_c = min(fabsf(branch_1), fabsf(branch_2));
                        }
                    }

                }
            }
          else
            {
              int this_sampling = -1;
              switch (thread_id)
                {
                  case 0:
                    this_sampling = CaloSampling::EME1;
                    break;
                  case 1:
                    this_sampling = CaloSampling::EME2;
                    break;
                  case 2:
                    this_sampling = CaloSampling::FCAL0;
                    break;
                  case 3:
                    this_sampling = CaloSampling::HEC0;
                    break;
                  default:
                    break;
                }
              if (this_sampling >= 0)
                {
                  const int this_cell = geometry->get_closest_cell(this_sampling, center_eta, center_phi);
                  float this_calc = 0.f;

                  if (this_cell >= 0)
                    {
                      const float this_z = geometry->z[this_cell];
                      this_calc = this_z + (this_z >= 0 ? -geometry->dz[this_cell] : geometry->dz[this_cell]) /
                                  (geometry->is_tile(this_cell) ? 2.f : 1.f);
                    }

                  const unsigned int mask = 0xF0000000U;
                  //The last 4 threads.

                  const float new_one = __shfl_down_sync(mask, this_calc, 1);

                  this_calc += new_one * (this_calc == 0.f);
                  //(0, 1) and (2, 3) get the wanted between the both of them.

                  const float new_two = __shfl_down_sync(mask, this_calc, 2);

                  this_calc += new_two * (this_calc == 0.f);
                  //0 got the correct one.

                  if (this_calc != 0.f && axis_z != 0.f)
                    {
                      lambda_c = fabsf( (this_calc - center_z) / axis_z );
                    }
                }
            }

          switch (thread_id)
            {
              case 0:
                if (first_attempt_cell < 0)
                  {
                    moments_arr->lateral[cluster] /= CMCHack::get_temporary_array<LateralNormalization>(moments_arr)[cluster];
                  }
                moments_arr->centerLambda[cluster] = lambda_c;
                break;
              case 1:
                if (first_attempt_cell >= 0)
                  {
                    moments_arr->lateral[cluster] /= CMCHack::get_temporary_array<LateralNormalization>(moments_arr)[cluster];
                  }
                moments_arr->longitudinal[cluster] /= CMCHack::get_temporary_array<LongitudinalNormalization>(moments_arr)[cluster];
                break;
              case 2:
                {
                  const float energy_normalization = 1.0f / (sum_energies > 0.f ? sum_energies : 1.f);
                  moments_arr->secondR[cluster]      *= energy_normalization;
                  moments_arr->secondLambda[cluster] *= energy_normalization;
                  break;
                }
              case 3:
                {
                  const float old_first_phi = moments_arr->firstPhi[cluster];
                  const float energy_normalization = 1.0f / (sum_energies > 0.f ? sum_energies : 1.f);
                  moments_arr->firstPhi[cluster] = Helpers::regularize_angle(old_first_phi * energy_normalization);
                  moments_arr->firstEta[cluster] *= energy_normalization;
                }
                break;
              default:
                break;
            }

        }
    }
}

/******************************************************************************
 * Final cleanup.                                                                *
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
          const int max_cell = CMCHack::get_temporary_array<MaxECellPerSample>(moments_arr)[sampling][cluster];
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
      else if (sum_energies <= 0.f)
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
    }
}

/******************************************************************************
 * Actual kernel calling code.                                                *
 ******************************************************************************/

__global__ static
void calculateClusterPropertiesAndMomentsDeferKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                     Helpers::CUDA_kernel_object<ClusterMomentsArr> moments_arr,
                                                     Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                                     const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                                     const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                                     const Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                                                     const Helpers::CUDA_kernel_object<ClusterMomentCalculationOptions> opts,
                                                     const int i_dimBlockClusters,  const int i_dimBlockCells)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    {
      const int cluster_number = clusters_arr->number;

      const int i_dimGridClusters = Helpers::int_ceil_div(cluster_number, Helpers::int_floor_div(i_dimBlockClusters, WarpSize));
      const int i_dimGridCells = Helpers::int_ceil_div(NCaloCells, Helpers::int_floor_div(i_dimBlockCells, WarpSize));

      const int i_dimGridAxis = Helpers::int_ceil_div(cluster_number, i_dimBlockClusters);

      zerothClusterPassKernel <<< i_dimGridClusters, i_dimBlockClusters>>>(moments_arr, clusters_arr, geometry,
                                                                           opts->skip_invalid_clusters);


      firstCellPassKernel <<< i_dimGridCells, i_dimBlockCells>>>(moments_arr, clusters_arr, cell_state_arr, cell_info_arr,
                                                                 geometry, opts->use_abs_energy);
      firstClusterPassKernel <<< i_dimGridClusters, i_dimBlockClusters>>>(moments_arr, clusters_arr, opts->skip_invalid_clusters);


      secondCellPassKernel <<< i_dimGridCells, i_dimBlockCells>>>(moments_arr, cell_state_arr, cell_info_arr, geometry, noise_arr,
                                                                  opts->use_abs_energy, opts->use_two_gaussian_noise, opts->min_LAr_quality);
      showerAxisPassKernel <<< i_dimGridAxis, i_dimBlockClusters>>>(moments_arr, clusters_arr,
                                                                    opts->max_axis_angle, opts->skip_invalid_clusters);
      secondClusterPassKernel <<< i_dimGridClusters, i_dimBlockClusters>>>(moments_arr, clusters_arr, opts->skip_invalid_clusters);


      thirdCellPassKernel <<< i_dimGridCells, i_dimBlockCells>>>(moments_arr, cell_state_arr, cell_info_arr, geometry, opts->use_abs_energy,
                                                                 opts->eta_inner_wheel, opts->min_l_longitudinal, opts->min_r_lateral);
      thirdClusterPassKernel <<< i_dimGridClusters, i_dimBlockClusters>>>(moments_arr, clusters_arr, geometry,
                                                                          opts->skip_invalid_clusters);


      finalClusterPassKernel <<< i_dimGridClusters, i_dimBlockClusters>>>(moments_arr, clusters_arr, cell_state_arr, cell_info_arr,
                                                                          geometry, opts->skip_invalid_clusters);


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
                                                                    const bool synchronize,
                                                                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream,
                                                                    const bool defer_instead_of_oversize)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const CUDAKernelLaunchConfiguration cfg_0_clu = optimizer.get_launch_configuration("ClusterMomentsCalculator", 0);
  const CUDAKernelLaunchConfiguration cfg_1_cel = optimizer.get_launch_configuration("ClusterMomentsCalculator", 1);
  const CUDAKernelLaunchConfiguration cfg_1_clu = optimizer.get_launch_configuration("ClusterMomentsCalculator", 2);
  const CUDAKernelLaunchConfiguration cfg_2_cel = optimizer.get_launch_configuration("ClusterMomentsCalculator", 3);
  const CUDAKernelLaunchConfiguration cfg_2_axs = optimizer.get_launch_configuration("ClusterMomentsCalculator", 4);
  const CUDAKernelLaunchConfiguration cfg_2_clu = optimizer.get_launch_configuration("ClusterMomentsCalculator", 5);
  const CUDAKernelLaunchConfiguration cfg_3_cel = optimizer.get_launch_configuration("ClusterMomentsCalculator", 6);
  const CUDAKernelLaunchConfiguration cfg_3_clu = optimizer.get_launch_configuration("ClusterMomentsCalculator", 7);
  const CUDAKernelLaunchConfiguration cfg_f_clu = optimizer.get_launch_configuration("ClusterMomentsCalculator", 8);

  if (optimizer.use_minimal_kernel_sizes() && optimizer.can_use_dynamic_parallelism())
    {
      const CUDAKernelLaunchConfiguration cfg_blocks = optimizer.get_launch_configuration("ClusterMomentsCalculator", 0);
      const CUDAKernelLaunchConfiguration cfg_finalize  = optimizer.get_launch_configuration("ClusterMomentsCalculator", 1);

      calculateClusterPropertiesAndMomentsDeferKernel <<< 1, 1, 0, stream_to_use>>>(holder.m_clusters_dev,
                                                                                    holder.m_moments_dev,
                                                                                    holder.m_cell_state_dev,
                                                                                    holder.m_cell_info_dev,
                                                                                    instance_data.m_geometry_dev,
                                                                                    instance_data.m_cell_noise_dev,
                                                                                    options.m_options_dev,
                                                                                    cfg_2_clu.block_x, cfg_3_cel.block_x);
    }
  else
    {

      zerothClusterPassKernel <<< cfg_0_clu.grid_x, cfg_0_clu.block_x, 0, stream_to_use>>>(holder.m_moments_dev,
                                                                                           holder.m_clusters_dev,
                                                                                           instance_data.m_geometry_dev,
                                                                                           options.m_options->skip_invalid_clusters);


      firstCellPassKernel <<< cfg_1_cel.grid_x, cfg_1_cel.block_x, 0, stream_to_use>>>(holder.m_moments_dev,
                                                                                       holder.m_clusters_dev,
                                                                                       holder.m_cell_state_dev,
                                                                                       holder.m_cell_info_dev,
                                                                                       instance_data.m_geometry_dev,
                                                                                       options.m_options->use_abs_energy);
      firstClusterPassKernel <<< cfg_1_clu.grid_x, cfg_1_clu.block_x, 0, stream_to_use>>>(holder.m_moments_dev,
                                                                                          holder.m_clusters_dev,
                                                                                          options.m_options->skip_invalid_clusters);


      secondCellPassKernel <<< cfg_2_cel.grid_x, cfg_2_cel.block_x, 0, stream_to_use>>>(holder.m_moments_dev,
                                                                                        holder.m_cell_state_dev,
                                                                                        holder.m_cell_info_dev,
                                                                                        instance_data.m_geometry_dev,
                                                                                        instance_data.m_cell_noise_dev,
                                                                                        options.m_options->use_abs_energy,
                                                                                        options.m_options->use_two_gaussian_noise,
                                                                                        options.m_options->min_LAr_quality);
      showerAxisPassKernel <<< cfg_2_axs.grid_x, cfg_2_axs.block_x, 0, stream_to_use>>>(holder.m_moments_dev,
                                                                                        holder.m_clusters_dev,
                                                                                        options.m_options->max_axis_angle,
                                                                                        options.m_options->skip_invalid_clusters);
      secondClusterPassKernel <<< cfg_2_clu.grid_x, cfg_2_clu.block_x, 0, stream_to_use>>>(holder.m_moments_dev,
                                                                                           holder.m_clusters_dev,
                                                                                           options.m_options->skip_invalid_clusters);


      thirdCellPassKernel <<< cfg_3_cel.grid_x, cfg_3_cel.block_x, 0, stream_to_use>>>(holder.m_moments_dev,
                                                                                       holder.m_cell_state_dev,
                                                                                       holder.m_cell_info_dev,
                                                                                       instance_data.m_geometry_dev,
                                                                                       options.m_options->use_abs_energy,
                                                                                       options.m_options->eta_inner_wheel,
                                                                                       options.m_options->min_l_longitudinal,
                                                                                       options.m_options->min_r_lateral);
      thirdClusterPassKernel <<< cfg_3_clu.grid_x, cfg_3_clu.block_x, 0, stream_to_use>>>(holder.m_moments_dev,
                                                                                          holder.m_clusters_dev,
                                                                                          instance_data.m_geometry_dev,
                                                                                          options.m_options->skip_invalid_clusters);


      finalClusterPassKernel <<< cfg_f_clu.grid_x, cfg_f_clu.block_x, 0, stream_to_use>>>(holder.m_moments_dev,
                                                                                          holder.m_clusters_dev,
                                                                                          holder.m_cell_state_dev,
                                                                                          holder.m_cell_info_dev,
                                                                                          instance_data.m_geometry_dev,
                                                                                          options.m_options->skip_invalid_clusters);
    }

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/*******************************************************************************************************************************/

void ClusterMomentsCalculator::register_kernels(IGPUKernelSizeOptimizer & optimizer)
{
  void * kernels[] = { (void *) zerothClusterPassKernel,
                       (void *) firstCellPassKernel,
                       (void *) firstClusterPassKernel,
                       (void *) secondCellPassKernel,
                       (void *) showerAxisPassKernel,
                       (void *) secondClusterPassKernel,
                       (void *) thirdCellPassKernel,
                       (void *) thirdClusterPassKernel,
                       (void *) finalClusterPassKernel
                     };

  int blocksizes[] = { ClusterPassBlockSize,
                       CellPassBlockSize,
                       ClusterPassBlockSize,
                       CellPassBlockSize,
                       ClusterPassBlockSize,
                       ClusterPassBlockSize,
                       CellPassBlockSize,
                       ClusterPassBlockSize,
                       ClusterPassBlockSize,
                     };

  int  gridsizes[] = { Helpers::int_ceil_div(NMaxClusters, Helpers::int_floor_div(ClusterPassBlockSize, WarpSize)),
                       Helpers::int_ceil_div(NCaloCells,   Helpers::int_floor_div(CellPassBlockSize,    WarpSize)),
                       Helpers::int_ceil_div(NMaxClusters, Helpers::int_floor_div(ClusterPassBlockSize, WarpSize)),
                       Helpers::int_ceil_div(NCaloCells,   Helpers::int_floor_div(CellPassBlockSize,    WarpSize)),
                       Helpers::int_ceil_div(NMaxClusters,                                   ClusterPassBlockSize),
                       Helpers::int_ceil_div(NMaxClusters, Helpers::int_floor_div(ClusterPassBlockSize, WarpSize)),
                       Helpers::int_ceil_div(NCaloCells,   Helpers::int_floor_div(CellPassBlockSize,    WarpSize)),
                       Helpers::int_ceil_div(NMaxClusters, Helpers::int_floor_div(ClusterPassBlockSize, WarpSize)),
                       Helpers::int_ceil_div(NMaxClusters, Helpers::int_floor_div(ClusterPassBlockSize, WarpSize)),
                     };

  int   maxsizes[] = { NMaxClusters * WarpSize,
                       NCaloCells   * WarpSize,
                       NMaxClusters * WarpSize,
                       NCaloCells   * WarpSize,
                       NMaxClusters,
                       NMaxClusters * WarpSize,
                       NCaloCells   * WarpSize,
                       NMaxClusters * WarpSize,
                       NMaxClusters * WarpSize
                     };

  optimizer.register_kernels("ClusterMomentsCalculator", 9, kernels, blocksizes, gridsizes, maxsizes);
}
