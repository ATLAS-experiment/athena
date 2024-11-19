# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonReadoutGeometryR4
################################################################################

# Declare the package name:
atlas_subdir( MuonReadoutGeometryR4 )

# Extra dependencies, based on the environment (no MuonCondSvc needed in AthSimulation):
set( extra_libs )

if( NOT SIMULATIONBASE )
    find_package(Acts CONFIG COMPONENTS PluginGeoModel)
    set( extra_libs MuonAlignmentDataR4 ActsPluginGeoModel MuonAlignmentData)
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
    

