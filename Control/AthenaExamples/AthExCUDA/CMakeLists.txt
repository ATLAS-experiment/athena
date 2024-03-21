# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthExCUDA )

# Check if CUDA compilation is available.
if( NOT CMAKE_CUDA_COMPILER )
   message( STATUS "CUDA not available, not building package" )
   return()
endif()

# External dependencies.
find_package( vecmem COMPONENTS CUDA )

# Add a component library that has some CUDA code in it.
atlas_add_component( AthExCUDA
   src/*.h src/*.cxx src/*.cu src/components/*.cxx
   DEFINITIONS EIGEN_NO_CUDA
   LINK_LIBRARIES vecmem::core vecmem::cuda AthenaBaseComps GaudiKernel
   StoreGateLib xAODTracking AthCUDAInterfacesLib AthCUDAKernelLib )

# Install extra files from the package.
atlas_install_joboptions( share/*.py )
atlas_install_python_modules( python/*.py )

# Test(s) in the package.
atlas_add_test( LinearTransformTask
   SCRIPT athena.py AthExCUDA/LinearTransformTaskExample_jobOptions.py
   POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( LinearTransformTaskMT
   SCRIPT athena.py --threads=4 AthExCUDA/LinearTransformTaskExample_jobOptions.py
   POST_EXEC_SCRIPT nopost.sh )
