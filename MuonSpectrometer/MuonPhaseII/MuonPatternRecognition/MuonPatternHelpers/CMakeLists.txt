# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonPatternHelpers )


find_package( Acts COMPONENTS Core )

atlas_add_library( MuonPatternHelpers
        Root/*.cxx
    PUBLIC_HEADERS MuonPatternHelpers
    LINK_LIBRARIES xAODMuonPrepData ActsCore Identifier MuonReadoutGeometryR4 xAODMeasurementBase MuonSpacePoint 
                   MuonPatternEvent MuonStationIndexLib
    PRIVATE_LINK_LIBRARIES MuonSpacePointCalibratorLib )
