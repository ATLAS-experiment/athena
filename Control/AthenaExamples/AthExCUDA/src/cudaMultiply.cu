//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "cudaMultiply.h"

// CUDA include(s).
#ifdef __CUDACC__
#   include <cuda.h>
#endif // __CUDACC__

// System include(s).
#include <iostream>

#ifdef __CUDACC__
inline bool 
cuda_check_bool(cudaError_t err, const char* expr, const char* file, int line){
    if (err != cudaSuccess) {
        std::cerr << "CUDA error at " << file << ":" << line << '\n'
                  << "Expression: " << expr << '\n'
                  << "Reason: " << cudaGetErrorString(err) << '\n';
        return false;
    }
    return true;
}

#define CUDA_CHECK(EXPR) \
    cuda_check_bool((EXPR), #EXPR, __FILE__, __LINE__)

namespace AthCUDAExamples {

   /// Very simple kernel performing a multiplication on an array.
   __global__
   void cudaMultiplyKernel( int n, float* array, float multiplier ) {

      const int index = blockIdx.x * blockDim.x + threadIdx.x;
      if( index >= n ) {
         return;
      }

      array[ index ] *= multiplier;
      return;
   }

   /// GPU implementation of @c cudaMultiply
   void cudaMultiply( std::vector< float >& array, float multiplier ) {

      // If no CUDA device is available, complain.
      int nCudaDevices = 0;
      if (!CUDA_CHECK( cudaGetDeviceCount( &nCudaDevices ) )) return;
      if( nCudaDevices == 0 ) {
         return;
      }

      // Allocate the array on the/a device, and copy the host array's content
      // to the device.
      float* deviceArray = nullptr;
      bool ok{true};
      ok = CUDA_CHECK( cudaMalloc( &deviceArray, sizeof( float ) * array.size() ) );
      if (not ok){
        cudaFree( deviceArray );
        return;
      }
      ok = CUDA_CHECK( cudaMemcpy( deviceArray, array.data(),
                              sizeof( float ) * array.size(),
                              cudaMemcpyHostToDevice ) );
      if (not ok){
        cudaFree( deviceArray );
        return;
      }                        

      // Run the kernel.
      static const int blockSize = 256;
      const int numBlocks = ( array.size() + blockSize - 1 ) / blockSize;
      cudaMultiplyKernel<<< numBlocks, blockSize >>>( array.size(),
                                                      deviceArray,
                                                      multiplier );
      ok = CUDA_CHECK( cudaDeviceSynchronize() );
      if (not ok){
        cudaFree( deviceArray );
        return;
      }                        


      // Copy the array back to the host's memory.
      ok = CUDA_CHECK( cudaMemcpy( array.data(), deviceArray,
                              sizeof( float ) * array.size(),
                              cudaMemcpyDeviceToHost ) );
      
      // Free the memory on the device.
      CUDA_CHECK( cudaFree( deviceArray ) );
      return;
   }

} // namespace AthCUDAExamples

#else

namespace AthCUDAExamples {

   /// CPU implementation of @c cudaMultiply
   void cudaMultiply( std::vector< float >& array, float multiplier ) {

      for( float& element : array ) {
         element *= multiplier;
      }
   }

} // namespace AthCUDAExamples

#endif // __CUDACC__
