//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_IGPUKERNELSIZEOPTIMIZERSVC_H
#define CALORECGPU_IGPUKERNELSIZEOPTIMIZERSVC_H

#include "AthenaBaseComps/AthService.h"
#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

/**
 * @class  IGPUKernelSizeOptimizerSvc
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 04 August 2023
 * @brief Actual Athena inteface for the @c IGPUKernelSizeOptimizer.
 */

class IGPUKernelSizeOptimizerSvc : virtual public IService, virtual public IGPUKernelSizeOptimizer
{
 public:

  DeclareInterfaceID( IGPUKernelSizeOptimizerSvc, 1, 0);

  virtual ~IGPUKernelSizeOptimizerSvc() = default;

};
#endif
