# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonViews )


find_package(Acts CONFIG COMPONENTS Core)

atlas_add_library( xAODMuonViews
    xAODMuonViews/*.h
    INTERFACE
    PUBLIC_HEADERS xAODMuonViews
    LINK_LIBRARIES xAODMeasurementBase Identifier MuonIdHelpersLib Acts::Core)


   
file(GLOB_RECURSE tests "test/*.cxx")

foreach(_theTestSource ${tests})
    get_filename_component(_theTest ${_theTestSource} NAME_WE)
    atlas_add_test( ${_theTest} SOURCES ${_theTestSource}
                    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                    LINK_LIBRARIES ${ROOT_LIBRARIES} xAODMuonViews AthenaBaseComps TestTools
                    POST_EXEC_SCRIPT nopost.sh)
    

endforeach()
