# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EventGUIDLookup )

# External dependencies:


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


# Test(s) in the package:
atlas_add_test( ExtractGUID_AOD_mc
                SCRIPT test/ExtractGUID_AOD_mc.sh "${CMAKE_CURRENT_SOURCE_DIR}"
                LOG_SELECT_PATTERN "^[0-9]+ [0-9]+ [0-9A-Fa-f]{8}(-[0-9A-Fa-f]{4}){3}-[0-9A-Fa-f]{12}$"
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300 )
