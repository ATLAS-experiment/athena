# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonReadoutGeometryR4
################################################################################

# Declare the package name:
atlas_subdir( MuonReadoutGeometryR4 )

# Extra dependencies, based on the environment (no MuonCondSvc needed in AthSimulation):
set( extra_libs )

if( NOT SIMULATIONBASE )
    find_package(Acts COMPONENTS Core)
    set( extra_libs MuonAlignmentDataR4  MuonAlignmentData Acts::Core)
endif()

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelHelpers )

atlas_add_library( MuonReadoutGeometryR4
                   src/*.cxx
                   PUBLIC_HEADERS MuonReadoutGeometryR4
                   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps AthenaKernel GeoPrimitives Identifier  
                                  ActsGeometryInterfacesLib ActsGeoUtils MuonIdHelpersLib GaudiKernel 
                                  StoreGateLib GeoModelUtilities CxxUtils ${extra_libs})

# Some functions here make heavy use of Eigen and are thus very slow in debug
# builds.  Set up to allow forcing them to compile with optimization and
# inlining, even in debug builds.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/RpcReadoutElement.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/MmReadoutElement.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/MdtReadoutElement.cxx
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
                    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                    LINK_LIBRARIES ${ROOT_LIBRARIES} MuonReadoutGeometryR4 
                    POST_EXEC_SCRIPT nopost.sh)
    

endforeach()
    

