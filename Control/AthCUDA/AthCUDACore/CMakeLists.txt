# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthCUDACore )

# Bail out if CUDA is not available.
if( NOT CMAKE_CUDA_COMPILER )
   return()
endif()

# Necessary external(s).
find_package( TBB )
find_package( Threads )
find_package( CUDAToolkit )

# Component(s) in the package.
atlas_add_library( AthCUDACoreLib
   AthCUDACore/*.h AthCUDACore/*.cuh AthCUDACore/*.icc src/*.cu src/*.cxx
   SHARED
   PUBLIC_HEADERS AthCUDACore
   INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
   LINK_LIBRARIES ${TBB_LIBRARIES} GaudiKernel CUDA::cudart
   PRIVATE_LINK_LIBRARIES Threads::Threads AthCUDAInterfacesLib CxxUtils )

# Unit test(s) in the package.
atlas_add_test( Memory
   SOURCES test/test_Memory.cu
   LINK_LIBRARIES AthCUDACoreLib
   POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( Info
   SOURCES test/test_Info.cxx
   LINK_LIBRARIES AthCUDACoreLib
   POST_EXEC_SCRIPT nopost.sh )
