// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// CUDA include(s).
#include <cuda_runtime.h>

// System include(s).
#include <vector>

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
void linearTransform(cudaStream_t stream, std::vector<float>& arr, float multiplier) {

   // Allocate array on device
   float* d_arr;
   std::size_t size = sizeof(float) * arr.size();
   cudaMallocAsync(&d_arr, size, stream);

   // Copy input
   cudaMemcpyAsync(d_arr, arr.data(), size, cudaMemcpyHostToDevice, stream);

   // Run computation
   static const int blockSize = 256;
   const int numBlocks = ( arr.size() + blockSize - 1 ) / blockSize;
   static const std::size_t sharedMemPerBlock = 0;
   linearTransform_kernel<<<numBlocks, blockSize, sharedMemPerBlock, stream>>>(d_arr, arr.size(), multiplier);

   // Copy output back
   cudaMemcpyAsync(arr.data(), d_arr, size, cudaMemcpyDeviceToHost, stream);

   // Free device memory
   cudaFreeAsync(d_arr, stream);
}
}
