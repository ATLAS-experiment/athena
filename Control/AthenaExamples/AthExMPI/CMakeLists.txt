# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir(AthExMPI)

# External dependencies.
# Keep CUDA helpers separate from the C++23 RPC templates.
if(CMAKE_CUDA_COMPILER)
   find_package(CUDAToolkit REQUIRED)
   find_package(CCCL CONFIG REQUIRED COMPONENTS CUB)
   atlas_add_library(AthExMPICUDALib
      src/cuda/*.h src/cuda/*.cu
      SHARED NO_PUBLIC_HEADERS
      PRIVATE_LINK_LIBRARIES CUDA::cudart CCCL::CUB)
   set_target_properties(AthExMPICUDALib PROPERTIES
      CUDA_STANDARD 20 CUDA_STANDARD_REQUIRED ON)
endif()

# Component(s) in the package.
atlas_add_component(AthExMPI
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthenaMPIGPULib AthenaBaseComps AthenaKernel GaudiKernel)

if(TARGET AthExMPICUDALib)
   target_compile_definitions(AthExMPI PRIVATE ATHEXMPI_HAVE_CUDA)
   target_link_libraries(AthExMPI PRIVATE AthExMPICUDALib)
endif()

# Install files from the package.
atlas_install_python_modules(python/*.py)
