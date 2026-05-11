# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthExCUDA )

# Check if CUDA compilation is available.
if( NOT CMAKE_CUDA_COMPILER )
   message( STATUS "CUDA not available, not building package" )
   return()
endif()

# External dependencies.
find_package( vecmem COMPONENTS CUDA )
find_package( CUDAToolkit )

# Add a component library that has some CUDA code in it.
atlas_add_component( AthExCUDA
   src/*.h src/*.cxx src/*.cu src/components/*.cxx
   DEFINITIONS EIGEN_NO_CUDA EIGEN_DONT_VECTORIZE
   LINK_LIBRARIES vecmem::core vecmem::cuda AthenaBaseComps GaudiKernel Gaudi::GaudiCUDALib
   StoreGateLib xAODTracking AthDeviceInterfacesLib AthCUDAInterfacesLib AthCUDAKernelLib )

# Install extra files from the package.
atlas_install_python_modules( python/*.py )

# Test(s) in the package.
atlas_add_test( LinearTransform
   PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"
   SCRIPT athena.py --threads=1 AthExCUDA/LinearTransformStandaloneExampleConfig.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES SKIP_RETURN_CODE 2 )

atlas_add_test( LinearTransformAsync
   PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"
   SCRIPT athena.py --threads=4 AthExCUDA/LinearTransformAsyncExampleConfig.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES SKIP_RETURN_CODE 2 )
