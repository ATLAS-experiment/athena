# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonPatternEvent )


find_package( Acts COMPONENTS Core )

atlas_add_library( MuonPatternEvent
                   src/*.cxx 
                   PUBLIC_HEADERS MuonPatternEvent
                   LINK_LIBRARIES xAODMuonPrepData Acts::Core Identifier MuonReadoutGeometryR4 xAODMeasurementBase 
                                  xAODMuon MuonSpacePoint MuonStationIndexLib)
