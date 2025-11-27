//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloRecGPU/Helpers.h"

void * CaloRecGPU::CUDA_Helpers::allocate(const size_t num)
{
  void * ret;
  CUDA_ERRCHECK(cudaMalloc(&ret, num));
  return ret;
}

void CaloRecGPU::CUDA_Helpers::deallocate(void * address)
{
  CUDA_ERRCHECK(cudaFree(address));
}


void * CaloRecGPU::CUDA_Helpers::allocate_pinned(const size_t num)
{
  void * ret;
  CUDA_ERRCHECK(cudaMallocHost(&ret, num));
  return ret;
}

void CaloRecGPU::CUDA_Helpers::deallocate_pinned(void * address)
{
  CUDA_ERRCHECK(cudaFreeHost(address));
}


void CaloRecGPU::CUDA_Helpers::GPU_to_CPU(void * dest, const void * const source, const size_t num)
{
  CUDA_ERRCHECK(cudaMemcpy(dest, source, num, cudaMemcpyDeviceToHost));
}

void CaloRecGPU::CUDA_Helpers::CPU_to_GPU(void * dest, const void * const source, const size_t num)
{
  CUDA_ERRCHECK(cudaMemcpy(dest, source, num, cudaMemcpyHostToDevice));
}

void CaloRecGPU::CUDA_Helpers::GPU_to_GPU(void * dest, const void * const source, const size_t num)
{
  CUDA_ERRCHECK(cudaMemcpy(dest, source, num, cudaMemcpyDeviceToDevice));
}



void CaloRecGPU::CUDA_Helpers::GPU_to_CPU_async(void * dest, const void * const source, const size_t num, CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);
  CUDA_ERRCHECK(cudaMemcpyAsync(dest, source, num, cudaMemcpyDeviceToHost, cudaStreamPerThread));
}

void CaloRecGPU::CUDA_Helpers::CPU_to_GPU_async(void * dest, const void * const source, const size_t num, CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);
  CUDA_ERRCHECK(cudaMemcpyAsync(dest, source, num, cudaMemcpyHostToDevice, stream_to_use));
}

void CaloRecGPU::CUDA_Helpers::GPU_to_GPU_async(void * dest, const void * const source, const size_t num, CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);
  CUDA_ERRCHECK(cudaMemcpyAsync(dest, source, num, cudaMemcpyDeviceToDevice, stream_to_use));
}

void CaloRecGPU::CUDA_Helpers::GPU_synchronize(CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream ? * ((cudaStream_t *) stream) : cudaStreamPerThread);
  CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
}

void CaloRecGPU::CUDA_Helpers::optimize_block_and_grid_size(void * func, int & block_size, int & grid_size, const int dynamic_memory, const int block_size_limit)
{
  CUDA_ERRCHECK(cudaOccupancyMaxPotentialBlockSize(&grid_size, &block_size, func, dynamic_memory, block_size_limit));
}

void CaloRecGPU::CUDA_Helpers::optimize_block_and_grid_size_for_cooperative_launch(void * func, int & block_size, int & grid_size, const int dynamic_memory, const int block_size_limit)
{
  CUDA_ERRCHECK(cudaOccupancyMaxPotentialBlockSize(&grid_size, &block_size, func, dynamic_memory, block_size_limit));
  CUDA_ERRCHECK(cudaOccupancyMaxActiveBlocksPerMultiprocessor(&grid_size, func, block_size, dynamic_memory));

  int multi_processor_count = 1;

  CUDA_ERRCHECK(cudaDeviceGetAttribute(&multi_processor_count, cudaDevAttrMultiProcessorCount, 0));

  grid_size *= multi_processor_count;
}


bool CaloRecGPU::CUDA_Helpers::supports_cooperative_launches()
{
  int ret = 0;
  
  CUDA_ERRCHECK(cudaDeviceGetAttribute(&ret, cudaDevAttrCooperativeLaunch, 0));
  
  return ret;
}

bool CaloRecGPU::CUDA_Helpers::supports_dynamic_parallelism()
{
  int major = 0, minor = 0;
  
  CUDA_ERRCHECK(cudaDeviceGetAttribute(&major, cudaDevAttrComputeCapabilityMajor, 0));
  CUDA_ERRCHECK(cudaDeviceGetAttribute(&minor, cudaDevAttrComputeCapabilityMinor, 0));
  
  return (major > 3 || (major == 3 && minor >= 5));

}

std::string CaloRecGPU::CUDA_Helpers::GPU_name()
{
  cudaDeviceProp properties;
  CUDA_ERRCHECK(cudaGetDeviceProperties(&properties, 0));
  return properties.name;
}
