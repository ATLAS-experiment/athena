# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( ActsGPUPatternRecognition )

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
  GaudiKernel
  AthenaBaseComps
  AthDeviceInterfacesLib
  PathResolver
  InDetIdentifier
  PixelReadoutGeometryLib
  ActsGeometryInterfacesLib
  ActsGPUEventLib
  ActsGPUInterfacesLib
  TrigInDetToolInterfacesLib
  TrigInDetPattRecoToolsLib
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

atlas_add_component( ActsGPUPatternRecognition
  ${_sources}
  DEFINITIONS
    ${GPU_TRACKING_DEFINITIONS}
  LINK_LIBRARIES
    ${_libraries}
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
