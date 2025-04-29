/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cuda.h>
#include <cuda_runtime.h>
#include <iterator>
#include <sstream>
#include <atomic>
#include <iostream>
#include <unistd.h>
#include "gpu_helpers.h"

int GPUHelpers::getNumberOfGPUs() {
  pid_t childpid;
  int fd[2];
  // create pipe descriptors
  pipe(fd);

  childpid = fork();
  if(childpid != 0) {  // parent
    close(fd[1]);
    // read the data (blocking operation)
    int maxDev;
    read(fd[0], &maxDev, sizeof(maxDev));
    // close the read-descriptor
    close(fd[0]);
    return maxDev;
  }
  else {  // child
    // writing only, no need for read-descriptor
    close(fd[0]);
    int maxDevice = 0;
    cudaGetDeviceCount(&maxDevice);
    cudaError_t error = cudaGetLastError();
    if(error != cudaSuccess) {
      maxDevice = 0;
      std::cout << "ERROR: " << cudaGetErrorString( error ) << std::endl;
    }
    // send the value on the write-descriptor
    write(fd[1], &maxDevice, sizeof(maxDevice)); 
    // close the write descriptor
    close(fd[1]);
    exit(0);
  }
}

int GPUHelpers::getNumberOfCores(int major, int minor) {

  int ncores = 0;

  if ((major == 7) && (minor == 5)) {
    ncores = 64;//Turing
  }
  if ((minor == 1) || (minor == 2)) {
    ncores = 128;
  }
  else if (minor == 0) {
    ncores = 64;
  }
  else if ((major == 8) && (minor == 6)) {
    ncores = 32;
  }
  else if ((major == 8) && (minor == 9)) {
    ncores = 128; //Ada, see https://images.nvidia.com/aem-dam/en-zz/Solutions/technologies/NVIDIA-ADA-GPU-PROVIZ-Architecture-Whitepaper_1.1.pdf
  }

  if(ncores == 0) {
    std::cout<<"Cannot determine the number of cores: unknown device type, major="<<major<<" minor="<<minor<<std::endl;
  }
  return ncores;
}

