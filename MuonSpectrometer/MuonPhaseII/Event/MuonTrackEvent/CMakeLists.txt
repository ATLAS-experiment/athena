# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonTrackEvent )


find_package( Acts COMPONENTS Core )

atlas_add_library(  MuonTrackEvent
                   src/*.cxx 
                   PUBLIC_HEADERS  MuonTrackEvent
                   LINK_LIBRARIES Acts::Core Identifier MuonReadoutGeometryR4 MuonPatternEvent
                                  xAODMuon MuonSpacePoint
                   PRIVATE_LINK_LIBRARIES MuonStationIndexLib FourMomUtils MuonDetDescrUtils
                                          MuonTruthHelpers ActsCalibratorsLib ActsEventLib )

file(GLOB_RECURSE tests "test/*.cxx")

foreach(_theTestSource ${tests})
    get_filename_component(_theTest ${_theTestSource} NAME_WE)
    atlas_add_test( ${_theTest} SOURCES ${_theTestSource}
                    LINK_LIBRARIES MuonTrackEvent FourMomUtils
                    POST_EXEC_SCRIPT noerror.sh)
endforeach()
    
