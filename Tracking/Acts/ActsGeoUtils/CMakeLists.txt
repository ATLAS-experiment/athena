# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: ActsGeoUtils
################################################################################

# Declare the package name:
atlas_subdir( ActsGeoUtils )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel )


atlas_add_library( ActsGeoUtils
                   src/*.cxx
                   PUBLIC_HEADERS ActsGeoUtils
                   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} GeoPrimitives Identifier 
                                  ActsGeometryInterfacesLib GeoModelUtilities CxxUtils)

if( NOT SIMULATIONBASE )                                                            
    # we also add unit tests. 
    # Build them from the files in test/ 
    file(GLOB_RECURSE tests "test/*.cxx")

    foreach(_theTestSource ${tests})
        get_filename_component(_theTest ${_theTestSource} NAME_WE)
        atlas_add_test( ${_theTest} SOURCES ${_theTestSource}
                        INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                        LINK_LIBRARIES ${ROOT_LIBRARIES} ActsGeoUtils 
                        POST_EXEC_SCRIPT nopost.sh)
    endforeach()
endif()

    

