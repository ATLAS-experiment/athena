# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( Campaigns )

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Declare tests
atlas_add_test( CampaignEnum
                SCRIPT test/testCampaignEnum.py
                POST_EXEC_SCRIPT nopost.sh )
