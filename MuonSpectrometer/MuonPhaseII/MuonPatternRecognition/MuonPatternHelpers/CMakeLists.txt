# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonPatternHelpers )


find_package( Acts COMPONENTS Core )

atlas_add_library( MuonPatternHelpers
                    Root/*.cxx
                    PUBLIC_HEADERS MuonPatternHelpers
                    LINK_LIBRARIES xAODMuonPrepData Acts::Core Identifier MuonReadoutGeometryR4 xAODMeasurementBase  
                                   MuonPatternEvent MuonStationIndexLib FourMomUtils MuonSpacePoint MuonRecToolInterfacesR4)


file(GLOB_RECURSE files "test/*.cxx")
foreach(_exeFile ${files})
    get_filename_component(_theExec ${_exeFile} NAME_WE)
    get_filename_component(_theLoc ${_exeFile} DIRECTORY)
    atlas_add_test( ${_theExec} 
                    SOURCES  ${_exeFile}
                    LINK_LIBRARIES MuonPatternHelpers
                    POST_EXEC_SCRIPT nopost.sh) 
endforeach()
