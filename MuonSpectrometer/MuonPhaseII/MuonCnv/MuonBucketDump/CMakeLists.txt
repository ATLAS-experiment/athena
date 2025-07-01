# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonBucketDump )


# Component(s) in the package:
atlas_add_component( MuonBucketDump
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps GeoPrimitives MuonIdHelpersLib  MuonRecToolInterfacesR4
                                     xAODMuonSimHit StoreGateLib xAODMuonPrepData MuonSpacePoint xAODMuon MuonTesterTreeLib
                                     MuonPatternEvent MuonTrackEvent MuonTruthHelpers MuonPRDTestR4Lib)
                                     
atlas_install_python_modules( python/*.py)

atlas_add_test( testMuonBucketDump
                SCRIPT python -m MuonBucketDump.muonBucketDump --nEvents 5 --noSTGC
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)
