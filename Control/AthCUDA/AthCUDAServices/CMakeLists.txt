# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthCUDAServices )

# Check if CUDA compilation is available.
if( NOT CMAKE_CUDA_COMPILER )
   message( STATUS "CUDA not available, not building package" )
   return()
endif()

# External(s).
find_package( TBB )
find_package( CUDAToolkit )
find_package( vecmem COMPONENTS CUDA )

# Component(s) in the package.
atlas_add_component( AthCUDAServices
   src/*.hxx src/*.cxx src/*.cuh src/*.cu src/components/*.cxx
   INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
   LINK_LIBRARIES ${TBB_LIBRARIES} GaudiKernel AthCUDAInterfacesLib AthenaInterprocess
   AthCUDACoreLib AthDeviceInterfacesLib AthenaBaseComps CxxUtils
   CUDA::cudart vecmem::cuda )


atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
