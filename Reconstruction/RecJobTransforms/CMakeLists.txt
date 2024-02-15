# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RecJobTransforms )

# Install python modules
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
# Install job options
atlas_install_joboptions( share/*.py )

# Install scripts
atlas_install_runtime( scripts/*.py )

atlas_install_data( share/*.ref )
