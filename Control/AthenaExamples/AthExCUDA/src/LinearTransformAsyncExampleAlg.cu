// Copyright (c) 2024 CERN for the benefit of the ATLAS collaboration

// Local include
#include "LinearTransformAsyncExampleAlg.h"

// Gaudi
#include <Gaudi/CUDA/CUDAStream.h>


/// CUDA kernel implementing computation
__global__ void linearTransform_kernel(float* arr, std::size_t size,
                                       float multiplier) {
  std::size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx >= size) {
    return;
  }
  arr[idx] *= multiplier;
}

namespace AthCUDAExamples {
StatusCode LinearTransformAsyncExampleAlg::linearTransform(std::vector<float>& arr, float multiplier) const {
   // Create stream
   ATH_MSG_INFO("Creating stream");
   Gaudi::CUDA::Stream stream(this);

   // Allocate array on device
   ATH_MSG_INFO("Allocating device memory");
   float* d_arr;
   std::size_t size = sizeof(float) * arr.size();
   cudaMallocAsync(&d_arr, size, stream);

   // Copy input
   cudaMemcpyAsync(d_arr, arr.data(), size, cudaMemcpyHostToDevice, stream);

   // Run computation
   static const int blockSize = 256;
   const int numBlocks = ( arr.size() + blockSize - 1 ) / blockSize;
   static const std::size_t sharedMemPerBlock = 0;
   ATH_MSG_INFO("Kernel Launch");
   linearTransform_kernel<<<numBlocks, blockSize, sharedMemPerBlock, stream>>>(d_arr, arr.size(), multiplier);

   // Copy output back
   cudaMemcpyAsync(arr.data(), d_arr, size, cudaMemcpyDeviceToHost, stream);

   // Explicit wait
   ATH_CHECK(stream.await());
   
   return StatusCode::SUCCESS;
}
}
