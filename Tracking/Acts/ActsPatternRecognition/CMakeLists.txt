# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsPatternRecognition )

# External dependencies:
find_package( Acts COMPONENTS Core )

atlas_add_component( ActsPatternRecognition
                     src/*.h src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
               Acts::Core
               ActsEventLib
               ActsGeometryInterfacesLib
               ActsInteropLib
               ActsToolInterfacesLib
               AthenaBaseComps
               AthenaMonitoringKernelLib
               BeamSpotConditionsData
               CxxUtils
               GaudiKernel
               InDetPrepRawData
               InDetReadoutGeometry
               InDetRecToolInterfaces
               MagFieldConditions
               MagFieldElements
               PixelReadoutGeometryLib
               SiSPSeededTrackFinderData
               SiSpacePoint
               StoreGateLib
               TrkEventUtils
               TrkParameters
               TrkSpacePoint
               xAODInDetMeasurement
               TrigInDetToolInterfacesLib )



if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/SeedingTool.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/GbtsSeedingTool.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/OrthogonalSeedingTool.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -UNDEBUG -fvisibility-inlines-hidden "
     COMPILE_DEFINITIONS "FLATTEN" )
endif()
