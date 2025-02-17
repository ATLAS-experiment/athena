# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonHitCsvDump )


# Component(s) in the package:
atlas_add_component( MuonHitCsvDump
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps GeoPrimitives MuonIdHelpersLib  
                                     MuonSimEvent xAODMuonSimHit StoreGateLib MuonTruthHelpers
                                     MuonSpacePoint xAODMuon MuonPatternHelpers)
                                     
atlas_install_python_modules( python/*.py)

atlas_add_test( testCsvDumper
                SCRIPT python -m MuonHitCsvDump.csvHitDump --nEvents 1 --noSTGC --noMM
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)
