//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//


#ifndef CALORECGPU_GPUCLUSTERSORTER_H
#define CALORECGPU_GPUCLUSTERSORTER_H

#include "CxxUtils/checker_macros.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "GPUClusterSorterImpl.h"
#include "CaloRecGPU/CaloGPUCUDAInitialization.h"

#include "GaudiKernel/ServiceHandle.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizerSvc.h"

#include "CLHEP/Units/SystemOfUnits.h"

/**
 * @class GPUClusterSorter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 26 October 2025
 * @brief Sorts clusters by ET (with possible cut) and creates the list of cells per cluster.
 */


class GPUClusterSorter:
  public extends<AthAlgTool, CaloClusterGPUProcessor>, public CaloGPUTimed, public CaloGPUCUDAInitialization
{
 public:

  GPUClusterSorter(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override
  {
    return CaloGPUCUDAInitialization::initialize();
  }
  
  virtual StatusCode initialize_non_CUDA() override;
  
  virtual StatusCode initialize_CUDA() override;

  virtual StatusCode execute (const EventContext & ctx,
                              const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data,
                              void * temporary_buffer) const override;

  virtual StatusCode finalize() override;

  virtual ~GPUClusterSorter() = default;
  
 private:

  /**
  * @brief if set to @p true cluster cuts are on \f$|E|_\perp\f$, if @p false on \f$E_\perp\f$. Default is @p true.
  *
  */
  Gaudi::Property<bool> m_cutClustersInAbsE {this, "ClusterCutsInAbsEt", true, "Do cluster cuts in Abs Et instead of Et"};

  /**
   * @brief \f$E_\perp\f$ cut on the clusters.
   *
   * The clusters have to pass this cut (which is on \f$E_\perp\f$
   * or \f$|E|_\perp\f$ of the cluster depending on the above switch)
   * in order to be inserted into the CaloClusterContainer.  */

  Gaudi::Property<float> m_clusterETThreshold {this, "ClusterEtorAbsEtCut", 0.*CLHEP::MeV, "Cluster E_t or Abs E_t cut"};

  /** @brief Handle to the CUDA kernel block and grid size optimization service. */
  ServiceHandle<IGPUKernelSizeOptimizerSvc> m_kernelSizeOptimizer { this, "KernelSizeOptimizer", "GPUKernelSizeOptimizerSvc", "CUDA kernel size optimization service." };

};

#endif //CALORECGPU_GPUCLUSTERSORTER_H
