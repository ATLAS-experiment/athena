# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: DigitizationConfig
################################################################################

# Declare the package name:
atlas_subdir( DigitizationConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.py )
