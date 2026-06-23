# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthCUDAInterfaces )

# Bail out if CUDA is not available.
if( NOT CMAKE_CUDA_COMPILER )
   return()
endif()

# Package dependencies.
find_package( CUDAToolkit )

# Component(s) in the package.
atlas_add_library( AthCUDAInterfacesLib
   AthCUDAInterfaces/*.h src/*.cxx
   PUBLIC_HEADERS AthCUDAInterfaces
   LINK_LIBRARIES CUDA::cudart GaudiKernel )
