# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: ActsGeoUtils
################################################################################

# Declare the package name:
atlas_subdir( ActsGeoUtils )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel )


set( extra_libs )

if( NOT SIMULATIONBASE )
    find_package(Acts COMPONENTS Core)
    set( extra_libs xAODTracking Acts::Core)
endif()


atlas_add_library( ActsGeoUtils
                   src/*.cxx
                   PUBLIC_HEADERS ActsGeoUtils
                   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} GeoPrimitives Identifier 
                                  ActsGeometryInterfacesLib GeoModelUtilities CxxUtils ${extra_libs})

if( NOT SIMULATIONBASE )                                                            
    # we also add unit tests. 
    # Build them from the files in test/ 
    file(GLOB_RECURSE tests "test/*.cxx")

    foreach(_theTestSource ${tests})
        get_filename_component(_theTest ${_theTestSource} NAME_WE)
        atlas_add_test( ${_theTest} SOURCES ${_theTestSource}
                        LINK_LIBRARIES ActsGeoUtils 
                        POST_EXEC_SCRIPT nopost.sh)
    endforeach()

# This test makes heavy use of Eigen and is thus very slow in debug
# builds.  Set up to allow forcing it to compile with optimization and
# inlining, even in debug builds.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/test/test_AlignStoreAccess.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
     COMPILE_DEFINITIONS "FLATTEN" )
endif()

endif()

    

