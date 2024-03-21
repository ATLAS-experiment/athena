//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "GPUClusterInfoAndMomentsCalculator.h"
#include "GPUClusterInfoAndMomentsCalculatorImpl.h"


using namespace CaloRecGPU;
using namespace ClusterMomentsCalculator;

GPUClusterInfoAndMomentsCalculator::GPUClusterInfoAndMomentsCalculator(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent)
{
  declareInterface<CaloClusterGPUProcessor> (this);
}

StatusCode GPUClusterInfoAndMomentsCalculator::initialize_non_CUDA()
{
  m_options.allocate();
    
  m_options.m_options->use_abs_energy         = m_absOpt;
  m_options.m_options->use_two_gaussian_noise = m_twoGaussianNoise;
  m_options.m_options->skip_invalid_clusters  = m_skipInvalidClusters;
  m_options.m_options->min_LAr_quality        = m_minBadLArQuality;
  m_options.m_options->max_axis_angle         = m_maxAxisAngle;
  m_options.m_options->eta_inner_wheel        = m_etaInnerWheel;
  m_options.m_options->min_l_longitudinal     = m_minLLongitudinal;
  m_options.m_options->min_r_lateral          = m_minRLateral;
  
  
  ATH_CHECK( m_kernelSizeOptimizer.retrieve() );
  
  return StatusCode::SUCCESS;
}

StatusCode GPUClusterInfoAndMomentsCalculator::initialize_CUDA()
{
  m_options.sendToGPU();
  register_kernels( *(m_kernelSizeOptimizer.get()) );
  
  return StatusCode::SUCCESS;
}

StatusCode GPUClusterInfoAndMomentsCalculator::execute(const EventContext & /*ctx*/, const ConstantDataHolder & constant_data,
                                                       EventDataHolder & event_data, void * /*temporary_buffer*/) const
{
  calculateClusterPropertiesAndMoments(event_data, constant_data, m_options, *(m_kernelSizeOptimizer.get()), m_measureTimes);
  return StatusCode::SUCCESS;

}

StatusCode GPUClusterInfoAndMomentsCalculator::finalize()
{
  return StatusCode::SUCCESS;
}

