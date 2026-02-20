//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "GPUClusterSorter.h"
#include "GPUClusterSorterImpl.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;
using namespace GPUClusterSorting;

GPUClusterSorter::GPUClusterSorter(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent),
  CaloGPUTimed(this)
{
}

StatusCode GPUClusterSorter::initialize_non_CUDA()
{ 
  ATH_CHECK( m_kernelSizeOptimizer.retrieve() );
  return StatusCode::SUCCESS;
}

StatusCode GPUClusterSorter::initialize_CUDA()
{ 
  register_kernels( *(m_kernelSizeOptimizer.get()) );
  return StatusCode::SUCCESS;
}

StatusCode GPUClusterSorter::execute(const EventContext & ctx, const ConstantDataHolder & constant_data,
                                     EventDataHolder & event_data, void * /*temporary_buffer*/) const
{
  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };
  
  const auto start = clock_type::now();
  
  const auto before_properties = clock_type::now();
  
  initialPropertiesCalculation(event_data, constant_data, *(m_kernelSizeOptimizer.get()), m_measureTimes, m_cutClustersInAbsE, m_clusterETThreshold);
  
  const auto before_sort = clock_type::now();
  
  sortClusters(event_data, constant_data, *(m_kernelSizeOptimizer.get()), m_measureTimes);
  
  const auto before_finalize = clock_type::now();
  
  finalizeClusterAssignment(event_data, constant_data, *(m_kernelSizeOptimizer.get()), m_measureTimes);
  
  const auto end = clock_type::now();
  
  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, before_properties),
                   time_cast(before_properties, before_sort),
                   time_cast(before_sort, before_finalize),
                   time_cast(before_finalize, end)
                  );
    }


  return StatusCode::SUCCESS;

}

StatusCode GPUClusterSorter::finalize()
{
  if (m_measureTimes)
    {
      print_times("Preprocessing Calculating_ET Sorting_Clusters Finalizing_Clusters", 4);
    }
  return StatusCode::SUCCESS;
}

