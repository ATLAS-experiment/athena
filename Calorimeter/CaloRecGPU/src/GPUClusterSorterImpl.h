//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_GPUCLUSTERSORTERIMPL_H
#define CALORECGPU_GPUCLUSTERSORTERIMPL_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

namespace GPUClusterSorting
{
  void register_kernels(IGPUKernelSizeOptimizer & optimizer);


  void initialPropertiesCalculation(CaloRecGPU::EventDataHolder & holder,
                                    const CaloRecGPU::ConstantDataHolder & instance_data,
                                    const IGPUKernelSizeOptimizer & optimizer,
                                    const bool synchronize = false,
                                    const bool cut_in_absolute_ET = true, const float absolute_ET_threshold = -1,
                                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

  void sortClusters(CaloRecGPU::EventDataHolder & holder,
                    const CaloRecGPU::ConstantDataHolder & instance_data,
                    const IGPUKernelSizeOptimizer & optimizer,
                    const bool synchronize = false,
                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});
                    
  void finalizeClusterAssignment(CaloRecGPU::EventDataHolder & holder,
                                 const CaloRecGPU::ConstantDataHolder & instance_data,
                                 const IGPUKernelSizeOptimizer & optimizer,
                                 const bool synchronize = false,
                                 CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});
}
#endif //CALORECGPU_GPUCLUSTERSORTERIMPL_H
