# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonPatternEvent )


atlas_add_library( MuonPatternEvent
    MuonPatternEvent/*.h  Root/*.cxx
    PUBLIC_HEADERS MuonPatternEvent
    LINK_LIBRARIES xAODMuonPrepData  Identifier MuonReadoutGeometryR4 xAODMeasurementBase MuonSpacePoint MuonStationIndexLib)
