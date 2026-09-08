# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EventPick )

# External dependencies:


# Install files from the package:
atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


# Test(s) in the package:
atlas_add_test( EventPick_AOD_run2_data
                SCRIPT test/EventPick_AOD_run2_data.sh "${CMAKE_CURRENT_SOURCE_DIR}"
                LOG_SELECT_PATTERN "^[0-9]+ [0-9]+$"
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 900 )

atlas_add_test( EventPick_AOD_run2_mc
                SCRIPT test/EventPick_AOD_run2_mc.sh "${CMAKE_CURRENT_SOURCE_DIR}"
                LOG_SELECT_PATTERN "^[0-9]+ [0-9]+$"
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 900 )

atlas_add_test( EventPick_AOD_run3_data
                SCRIPT test/EventPick_AOD_run3_data.sh "${CMAKE_CURRENT_SOURCE_DIR}"
                LOG_SELECT_PATTERN "^[0-9]+ [0-9]+$"
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 900 )

atlas_add_test( EventPick_AOD_run3_mc
                SCRIPT test/EventPick_AOD_run3_mc.sh "${CMAKE_CURRENT_SOURCE_DIR}"
                LOG_SELECT_PATTERN "^[0-9]+ [0-9]+$"
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 900 )

atlas_add_test( EventPick_ESD_run3_mc
                SCRIPT test/EventPick_ESD_run3_mc.sh "${CMAKE_CURRENT_SOURCE_DIR}"
                LOG_SELECT_PATTERN "^[0-9]+ [0-9]+$"
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 900 )

atlas_add_test( EventPick_ESD_run3_data22
                SCRIPT test/EventPick_ESD_run3_data22.sh "${CMAKE_CURRENT_SOURCE_DIR}"
                LOG_SELECT_PATTERN "^[0-9]+ [0-9]+$"
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 900 )

# tests that should not run in AthGeneration, AthSimulation or analysis releases
if( NOT GENERATIONBASE AND NOT SIMULATIONBASE AND NOT XAOD_ANALYSIS )
    atlas_add_test( EventPick_RAW_run2_data15
                    SCRIPT test/EventPick_RAW_run2_data15.sh "${CMAKE_CURRENT_SOURCE_DIR}"
                    LOG_SELECT_PATTERN "^[0-9]+ [0-9]+$"
                    PRIVATE_WORKING_DIRECTORY
                    PROPERTIES TIMEOUT 900 )
endif()
