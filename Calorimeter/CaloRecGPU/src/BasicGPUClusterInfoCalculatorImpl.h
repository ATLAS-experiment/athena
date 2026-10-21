//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_BASICGPUCLUSTERINFOCALCULATORIMPL_H
#define CALORECGPU_BASICGPUCLUSTERINFOCALCULATORIMPL_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

namespace BasicClusterInfoCalculator
{
  void register_kernels(IGPUKernelSizeOptimizer & optimizer);

  void updateSeedCellProperties(CaloRecGPU::EventDataHolder & holder,
                                const CaloRecGPU::ConstantDataHolder & instance_data,
                                const IGPUKernelSizeOptimizer & optimizer,
                                CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use,
                                const bool synchronize = false);

  void calculateClusterProperties(CaloRecGPU::EventDataHolder & holder,
                                  const CaloRecGPU::ConstantDataHolder & instance_data,
                                  const IGPUKernelSizeOptimizer & optimizer,
                                  CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use,
                                  const bool synchronize = false,
                                  const bool cut_in_absolute_ET = true, const float absolute_ET_threshold = -1);

}
#endif //CALORECGPU_BASICGPUCLUSTERINFOCALCULATORIMPL_H
