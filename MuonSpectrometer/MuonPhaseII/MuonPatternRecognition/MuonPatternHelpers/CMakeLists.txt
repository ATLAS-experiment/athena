# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonPatternHelpers )


find_package( Acts COMPONENTS Core )

atlas_add_library( MuonPatternHelpers
                    Root/*.cxx
                    PUBLIC_HEADERS MuonPatternHelpers
                    LINK_LIBRARIES xAODMuonPrepData Acts::Core Identifier MuonReadoutGeometryR4 ActsCalibBase
                                                    MuonPatternEvent MuonStationIndexLib MuonSpacePoint 
                                                    MuonRecToolInterfacesR4 CxxUtils)
