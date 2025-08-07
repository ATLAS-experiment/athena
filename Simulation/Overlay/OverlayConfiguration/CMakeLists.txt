# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( OverlayConfiguration )

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( scripts/*.py )
atlas_install_runtime( test/OverlayTest.py )

# Setup and run tests
atlas_add_test( OverlayTest_MC_Run2
                SCRIPT test/OverlayTest.py -r RUN2
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 900
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( OverlayTest_MC_Run3
                SCRIPT test/OverlayTest.py -r RUN3
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 900
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( OverlayTest_MC_Run4
                SCRIPT test/OverlayTest.py -r RUN4
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 900
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( OverlayTest_data
                SCRIPT test/OverlayTest.py -d
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1000
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( OverlayMetadataConfig_test
                SCRIPT test/OverlayMetadataConfig_test.py
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh )
