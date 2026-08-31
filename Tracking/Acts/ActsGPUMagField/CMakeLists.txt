# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
atlas_subdir( ActsGPUMagField )
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
atlas_add_component( ActsGPUMagField
  ${_sources}
  DEFINITIONS
    ${GPU_TRACKING_DEFINITIONS}
  LINK_LIBRARIES
    ${_libraries}
)
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
