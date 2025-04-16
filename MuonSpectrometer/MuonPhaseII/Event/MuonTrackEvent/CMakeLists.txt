# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonTrackEvent )


find_package( Acts COMPONENTS Core )

atlas_add_library(  MuonTrackEvent
                   src/*.cxx 
                   PUBLIC_HEADERS  MuonTrackEvent
                   LINK_LIBRARIES ActsCore Identifier MuonReadoutGeometryR4 MuonPatternEvent
                                  xAODMuon MuonSpacePoint MuonStationIndexLib )
