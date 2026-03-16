# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigStorageDefinitions )

# Component(s) in the package:
atlas_add_library( TrigStorageDefinitions
                   PUBLIC_HEADERS TrigStorageDefinitions
                   LINK_LIBRARIES AthLinks AthContainers Navigation xAODBTagging xAODCaloEvent xAODEgamma xAODHIEvent xAODJet xAODMuon xAODTau xAODTracking xAODTrigBphys xAODTrigCalo xAODTrigEgamma xAODTrigL1Calo xAODTrigMinBias xAODTrigMissingET xAODTrigMuon xAODTrigRinger xAODTrigger )

# Tests:
atlas_add_test( ListMap_test
                SOURCES test/ListMap_test.cxx
                LINK_LIBRARIES TrigStorageDefinitions
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( EDMMap_test
                SOURCES test/EDMMap_test.cxx
                LINK_LIBRARIES TrigStorageDefinitions
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( Basic_test
                SOURCES test/Basic_test.cxx
                LINK_LIBRARIES TrigStorageDefinitions
                POST_EXEC_SCRIPT nopost.sh )
