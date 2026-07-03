# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( ActsGPUDataPreparation )

find_package(Acts)

# placeholder for what will eventually be Acts::PluginTraccc
if (NOT TARGET Acts::PluginDetray)
   # Without PluginDetray, the traccc code will not be available.
   return()
endif()

find_package(traccc)
find_package(vecmem)

# Common source files — always compiled
set( _sources
   src/*.h src/*.cxx
   src/components/*.cxx
)

# Common link libraries — always required
set( _libraries
  vecmem::core
  traccc::core
  traccc::device_common
  traccc::io
  GaudiKernel
  AthenaBaseComps
  AthDeviceInterfacesLib
  PathResolver
  InDetIdentifier
  ActsGeometryInterfacesLib
  ActsGPUEventLib
  ActsGPUInterfacesLib
  PixelReadoutGeometryLib
  SCT_ReadoutGeometry
)


# CUDA-only sources and libraries
if( CMAKE_CUDA_COMPILER )
  list( APPEND _sources
    src/cuda/*.h
    src/cuda/*.cxx

  )
  list( APPEND _libraries
    traccc::cuda
    vecmem::cuda
    AthCUDAInterfacesLib
  )
  list( APPEND GPU_TRACKING_DEFINITIONS
    ACTSTRACK_HAVE_CUDA
  )
endif()

atlas_add_component( ActsGPUDataPreparation
  ${_sources}
  DEFINITIONS
    ${GPU_TRACKING_DEFINITIONS}
  LINK_LIBRARIES
    ${_libraries}
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# depends on having the configurations for EDM converters
# atlas_add_test( ActsDeviceClusterizationTest
#     SCRIPT athena.py --threads=1 ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsDeviceClusterizationTest.py
#     POST_EXEC_SCRIPT nopost.sh
#     PROPERTIES SKIP_RETURN_CODE 2)