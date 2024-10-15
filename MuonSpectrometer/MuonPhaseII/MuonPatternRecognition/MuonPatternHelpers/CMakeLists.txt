# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonPatternHelpers )


find_package( Acts COMPONENTS Core )

atlas_add_library( MuonPatternHelpers
                    Root/*.cxx
                    PUBLIC_HEADERS MuonPatternHelpers
                    LINK_LIBRARIES xAODMuonPrepData ActsCore Identifier MuonReadoutGeometryR4 xAODMeasurementBase  
                                   MuonPatternEvent MuonStationIndexLib FourMomUtils MuonSpacePoint MuonRecToolInterfacesR4)

atlas_add_test( CombinatorialSeedSolverTest
    SOURCES test/CombinatorialSeedSolverTest.cxx 
    LINK_LIBRARIES ${ROOT_LIBRARIES} MuonPatternHelpers
    POST_EXEC_SCRIPT nopost.sh)
