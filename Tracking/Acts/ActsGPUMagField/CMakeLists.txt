# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
atlas_subdir( ActsGPUMagField )
if (NOT (CMAKE_CUDA_COMPILER OR CMAKE_HIP_COMPILER))
   message( STATUS "Neither CUDA nor HIP are available, not building package" )
endif()

find_package(Acts)
# placeholder for what will eventually be Acts::PluginTraccc
if (NOT TARGET Acts::PluginDetray)
   # Without PluginDetray, the traccc code will not be available.
   return()
endif()
find_package(traccc)
# Common source files — always compiled
set( _sources
   src/*.h src/*.cxx
   src/components/*.cxx
)
# Common link libraries — always required
set( _libraries
  traccc::core
  traccc::io
  GaudiKernel
  AthenaBaseComps
  PathResolver
  ActsGPUEventLib
)
# CUDA-only sources and libraries
if( CMAKE_CUDA_COMPILER )
  list( APPEND _sources
    src/cuda/*.h
    src/cuda/*.cxx
  )
  list( APPEND _libraries
    traccc::cuda_utils
  )
  list( APPEND GPU_TRACKING_DEFINITIONS
    ACTSTRACK_HAVE_CUDA
  )
endif()

# HIP-only sources and libraries
if( CMAKE_HIP_COMPILER )
  list( APPEND _sources
    src/hip/*.h
    src/hip/*.cxx
  )
  list( APPEND _libraries
    traccc::hip_utils
  )
  list( APPEND GPU_TRACKING_DEFINITIONS
    ACTSTRACK_HAVE_HIP
  )
endif()
atlas_add_component( ActsGPUMagField
  ${_sources}
  DEFINITIONS
    ${GPU_TRACKING_DEFINITIONS}
  LINK_LIBRARIES
    ${_libraries}
)

atlas_install_python_modules( python/*.py test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


#Need to set environment to change PathResolver ERROR about using the dev area to a WARNING (Locating dev file dev/ACTS/detray-itk/ITk_bfield.cvf. Do not let this propagate to a release!)

#need to check for presence of compilers again because the library might not have been built
if (CMAKE_CUDA_COMPILER)
atlas_add_test( CUDAGPUMagFieldExample
   PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"
   ENVIRONMENT "PATHRESOLVER_DEVAREARESPONSE=WARNING"
   SCRIPT ActsDeviceMagFieldTest.py Device.Backend=DeviceBackend.CUDA
   POST_EXEC_SCRIPT noerror.sh)
endif()

if (CMAKE_HIP_COMPILER)
atlas_add_test( HIPGPUMagFieldExample
   PRE_EXEC_SCRIPT "amd-check.sh || exit 2"
   ENVIRONMENT "PATHRESOLVER_DEVAREARESPONSE=WARNING"
   SCRIPT ActsDeviceMagFieldTest.py Device.Backend=DeviceBackend.HIPAMD
   POST_EXEC_SCRIPT noerror.sh)
endif()

