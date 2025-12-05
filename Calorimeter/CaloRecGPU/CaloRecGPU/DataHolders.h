//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_DATAHOLDERS_H
#define CALORECGPU_DATAHOLDERS_H

#include <vector>

#include "Helpers.h"
#include "CUDAFriendlyClasses.h"

#define CALORECGPU_USE_PINNED_MEMORY 1

namespace CaloRecGPU
{

  ///@class ConstantDataHolder
  ///Holds CPU and GPU versions of the geometry and cell noise information,
  ///which are assumed to be constant throughout the run.
  ///(The former is for sure constant by design, the latter may require
  /// future adjustment if changing the noise constants mid-run
  /// ever becomes desirable.)
  class ConstantDataHolder
  {
   public:

    void sendToGPU(const bool clear_CPU = true);

    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> m_geometry;

    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> m_cell_noise;

    CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::GeometryArr> m_geometry_dev;

    CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::CellNoiseArr> m_cell_noise_dev;

  };

  ///@class MomentsOptionsArray
  ///Holds an array of bools to represent the different moments
  ///that may be calculated and transferred to/from the GPU.
  ///While the GPU calculation itself does not skip the computation of any moments
  ///(especially since there are some complex inter-dependencies),
  ///if only a subset of them is needed, it can cut down on data transfer
  ///and especially data conversion times to not consider the rest.
  struct MomentsOptionsArray
  {
    static constexpr int num_moments = 72;
    bool array[num_moments + 1]{};
    //Initialize to false.
    //(We could consider using
    //some form of bitset here,
    //but I'm not sure there would be
    //a significant performance difference...)

    static int moment_to_linear(const int moment);

    bool & operator[] (const int moment);

    bool operator[] (const int  moment) const;
    
    static MomentsOptionsArray all();
  };

  ///@class EventDataHolder
  ///Holds the mutable per-event information (clusters and cells)
  ///and provides utilities to convert between this representation
  ///and the Athena data structures (i. e. `xAOD::CaloClusterContainer`).
  class EventDataHolder
  {
   public:
   
    ///We are using a void* for API to make this able to compile on the GPU
    ///without Athena-specific dependencies. The user should pass a
    ///`const CaloCellCollection *`.
    void importCells(const void * cell_collection, const std::vector<int> & extra_cells_to_fill = {});

    ///We are using a void* for API to make this able to compile on the GPU
    ///without Athena-specific dependencies. The user should pass a
    ///`const xAOD::CaloClusterContainer *`.
    ///@p moments_to_add specifies which moments we will import.
    void importClusters(const void * cluster_collection, 
                        const MomentsOptionsArray & moments_to_add,
                        const bool output_tags = true,
                        const bool consider_shared_cells = true,
                        const bool output_moments = false,
                        const bool output_extra_moments = false,
                        const std::vector<int> & extra_cells_to_fill = {});

    ///This function is asynchronous if @p clear_CPU and @p synchronize are @c false.
    ///@p moments_to_add specifies which moments we will transfer to the GPU.
    ///If @p full_copy is @c true, we will memcopy the whole structure
    ///in bulk instead of enqueuing separate transfers up to the necessary size
    ///(this will also ignore the @p moments_to_add).
    ///If @p clear_CPU is @c true, we will wait until the transfers are finished
    ///before deallocating. If @p synchronize is @c true, we will synchronize anyway.
    void sendToGPU(const MomentsOptionsArray & moments_to_add,
                   const bool full_copy = false,
                   const bool clear_CPU = false,
                   const bool synchronize = false,
                   CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream = {});

    ///@p moments_to_add specifies which moments we will transfer from the GPU.
    ///If @p full_copy is @c true, we will memcopy the whole structure
    ///in bulk instead of enqueuing separate transfers up to the necessary size
    ///(this will also ignore the @p moments_to_add).
    ///If @p full_copy is @c false, we will have a synchronization point before
    ///enqueuing the transfers to know the sizes.
    ///If @p clear_GPU is @c true, we will wait until the transfers are finished
    ///before deallocating. If @p synchronize is @c true, we will synchronize anyway.
    void returnToCPU(const MomentsOptionsArray & moments_to_add,
                     const bool full_copy = false,
                     const bool clear_GPU = false,
                     const bool synchronize = false,
                     CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream = {},
                     const bool also_return_cells = false);
    
    ///We are using a void* for API to make this able to compile on the GPU
    ///without Athena-specific dependencies. The user should pass a
    ///`xAOD::CaloClusterContainer *` and a `const DataLink<CaloCellContainer> *`.
    ///@p moments_to_add specifies which moments we will import.
    ///If @p sort_clusters is @c true, the clusters will be sorted based on
    ///the transverse energy, otherwise the same order will be kept.
    ///If @p save_uncalibrated, the basic cluster information will be saved
    ///as raw.
    ///If @c time_measurements is not null, separate time measurements
    ///for the five stages (cell link creation, cell assignment,
    ///sorting, basic info filling and moments filling) will be stored.
    void exportClusters(void * cluster_collection,
                        const void * cell_collection_link,
                        const MomentsOptionsArray & moments_to_add,
                        const bool sort_clusters = true,
                        const bool save_uncalibrated = true,
                        const bool output_extra_moments = false,
                        const std::vector<int> & extra_cells_to_fill = {},
                        size_t * time_measurements = nullptr);
    
    ///We are using a void* for API to make this able to compile on the GPU
    ///without Athena-specific dependencies. The user should pass a
    ///`xAOD::CaloClusterContainer *` and a `const DataLink<CaloCellContainer> *`.
    ///@p moments_to_add specifies which moments we will import.
    ///If @p sort_clusters is @c true, the clusters will be sorted based on
    ///the transverse energy, otherwise the same order will be kept.
    ///If @p save_uncalibrated, the basic cluster information will be saved
    ///as raw.
    ///If @c time_measurements is not null, separate time measurements
    ///for the six stages (initial transfer, cell link creation, cell assignment,
    ///sorting, basic info filling and moments filling) will be stored.
    void returnAndExportClusters(void * cluster_collection,
                                 const void * cell_collection_link,
                                 const MomentsOptionsArray & moments_to_add,
                                 const bool sort_clusters = true,
                                 const bool save_uncalibrated = true,
                                 const bool output_extra_moments = false,
                                 const std::vector<int> & extra_cells_to_fill = {},
                                 size_t * time_measurements = nullptr,
                                 CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream = {});
    
    void allocate(const bool also_GPU = true);
    
    void clear_GPU();

#if CALORECGPU_USE_PINNED_MEMORY

    CaloRecGPU::Helpers::CUDA_pinned_CPU_object<CaloRecGPU::CellInfoArr> m_cell_info;
    CaloRecGPU::Helpers::CUDA_pinned_CPU_object<CaloRecGPU::ClusterInfoArr> m_clusters;

#else

    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> m_cell_info;
    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> m_clusters;

#endif

    CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::CellInfoArr> m_cell_info_dev;
    CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::ClusterInfoArr> m_clusters_dev;

  };

}

#endif //CALORECGPU_DATAHOLDERS_H
