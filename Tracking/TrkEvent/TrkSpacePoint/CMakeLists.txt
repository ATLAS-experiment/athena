# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkSpacePoint )

# Component(s) in the package:
atlas_add_library( TrkSpacePoint
                   src/*.cxx
                   PUBLIC_HEADERS TrkSpacePoint
                   LINK_LIBRARIES AthContainers AthenaKernel Identifier GaudiKernel TrkMeasurementBase EventContainers
                   PRIVATE_LINK_LIBRARIES TrkDetElementBase TrkSurfaces TrkEventPrimitives TrkPrepRawData )

# Some functions here make heavy use of Eigen and are thus very slow in debug
# builds.  Set up to allow forcing them to compile with optimization and
# inlining, even in debug builds.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/SpacePoint.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
     COMPILE_DEFINITIONS "FLATTEN" )
endif()

