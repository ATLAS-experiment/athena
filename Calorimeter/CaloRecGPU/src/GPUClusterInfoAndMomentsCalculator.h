//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_GPUCLUSTERINFOANDMOMENTSCALCULATOR_H
#define CALORECGPU_GPUCLUSTERINFOANDMOMENTSCALCULATOR_H

#include "CxxUtils/checker_macros.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "GPUClusterInfoAndMomentsCalculatorImpl.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizerSvc.h"

#include "GaudiKernel/ServiceHandle.h"

#include "CaloRecGPU/CaloGPUCUDAInitialization.h"

#include "CLHEP/Units/SystemOfUnits.h"

/**
 * @class GPUClusterInfoAndMomentsCalculator
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 01 March 2023
 * @brief Standard tool to calculate cluster info (energy, transverse energy, pseudo-rapidity and azimuthal angle).
 */


class GPUClusterInfoAndMomentsCalculator:
  public extends<AthAlgTool, CaloClusterGPUProcessor>, public CaloGPUCUDAInitialization
{
 public:

  GPUClusterInfoAndMomentsCalculator(const std::string & type, const std::string & name, const IInterface * parent);

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

  virtual ~GPUClusterInfoAndMomentsCalculator() = default;

 private:


  /** @brief the maximal allowed deviation from the
   * IP-to-ClusterCenter-axis. */
  Gaudi::Property<double> m_maxAxisAngle {this, "MaxAxisAngle", 20 * CLHEP::deg, "The maximal allowed deviation from the IP-to-ClusterCenter-axis"};

  /**
   * @brief the minimal \f$r\f$ in the definition of the Lateral moment
   *
   * This defines the minimal distance the two leading cells might
   * have before this value is used instead of their real distance in
   * the normalization of the LATERAL moment. */
  Gaudi::Property<double> m_minRLateral {this, "MinRLateral", 4 * CLHEP::cm, "The minimal r in the definition of the Lateral moment"};  

  /**
   * @brief the minimal \f$\lambda\f$ in the definition of the
   * Longitudinal moment
   *
   * This defines the minimal distance along the shower axis from the
   * cluster center the two leading cells might have before this value
   * is used instead of their real distance in the normalization of
   * the LONGITUDINAL moment. */
  Gaudi::Property<double> m_minLLongitudinal {this, "MinLLongitudinal", 10 * CLHEP::cm, "The minimal lambda in the definition of the Longitudinal moment"};

  /**
   * @brief the minimal cell quality in the LAr for declaring a cell bad
   *
   * This defines the minimal quality (large values mean worse shape)
   * a cell needs to exceed in order to be considered as not
   * compatible with a normal ionization signal. */
  Gaudi::Property<double> m_minBadLArQuality {this, "MinBadLArQuality", 4000, "The minimal cell quality in the LAr for declaring a cell bad"};

  /**
   * @brief if set to true use abs E value of cells to calculate 
   * cluster moments */
   Gaudi::Property<bool> m_absOpt {this, "WeightingOfNegClusters", false, "If set to true use abs E value of cells to calculate cluster moments"};

   /**
    * @brief Transition from outer to inner wheel in EME2 */
   Gaudi::Property<double> m_etaInnerWheel {this, "EMECAbsEtaWheelTransition", 2.52, "Transition from outer to inner wheel in EME2"};

  /**
   * @brief if set to true use 2-gaussian noise description for
   * TileCal  */
   Gaudi::Property<bool> m_twoGaussianNoise {this, "TwoGaussianNoise", false, "If set to true use 2-gaussian noise description for TileCal"};
  
  /** @brief If @p false, do the moment calculation even for invalid clusters (which may waste computation).
   *  Defaults to @p true.
   */
  Gaudi::Property<bool> m_skipInvalidClusters {this, "SkipInvalidClusters", true, "Check for and skip invalid clusters during moments calculation."};
  
  /** @brief Options for the algorithm, held in a GPU-friendly way.
  */
  ClusterMomentsCalculator::CMCOptionsHolder m_options;
  
  
  /** @brief If @p true, synchronize the kernel calls to ensure accurate per-step/per-tool time measurements.
   *  Defaults to @p false.
   */
  Gaudi::Property<bool> m_measureTimes {this, "MeasureTimes", false, "Synchronize for time measurements"};
  
  /** @brief Handle to the CUDA kernel block and grid size optimization service. */
  ServiceHandle<IGPUKernelSizeOptimizerSvc> m_kernelSizeOptimizer { this, "KernelSizeOptimizer", "GPUKernelSizeOptimizerSvc", "CUDA kernel size optimization service." };
};

#endif //CALORECGPU_TOPOAUTOMATONCLUSTERING_H
