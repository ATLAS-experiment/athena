# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonReadoutGeometry
################################################################################

# Declare the package name:
atlas_subdir( MuonReadoutGeometry )

# Extra dependencies, based on the environment (no MuonCondSvc needed in AthSimulation):
set( extra_libs )
if( NOT SIMULATIONBASE )
  set( extra_libs MuonCondSvcLib )
endif()

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel )

# Component(s) in the package:
atlas_add_library( MuonReadoutGeometry
                   src/*.c*
                   PUBLIC_HEADERS MuonReadoutGeometry
                   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps AthenaKernel GeoPrimitives Identifier GaudiKernel 
                   MuonAlignmentData TrkDetElementBase TrkSurfaces MuonIdHelpersLib StoreGateLib GeoModelUtilities CxxUtils ${extra_libs}
                   PRIVATE_LINK_LIBRARIES AGDDModel MuonAGDDDescription)


# Code in this file makes heavy use of eigen and runs orders of magnitude
# more slowly without optimization.  So force this to be optimized even
# in debug builds.  If you need to debug it you might want to change this.
# Specifying optimization via an attribute on the particular
# function didn't work, because that still didn't allow inlining.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/MdtReadoutElement.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/RpcReadoutElement.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/MuonStation.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
     COMPILE_DEFINITIONS "FLATTEN" )
endif()

# we also add unit tests. 
# Build them from the files in test/ 
file(GLOB_RECURSE tests "test/*.cxx")
foreach(_theTestSource ${tests})
    get_filename_component(_theTest ${_theTestSource} NAME_WE)

    atlas_add_test( ${_theTest} SOURCES ${_theTestSource}
    LINK_LIBRARIES MuonReadoutGeometry 
    POST_EXEC_SCRIPT nopost.sh 
    )
    

endforeach()
