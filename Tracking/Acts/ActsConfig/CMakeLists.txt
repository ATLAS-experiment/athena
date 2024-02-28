# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.sh test/*.py)

atlas_add_test( ActsTrackingComponents
  SCRIPT test/ActsTrackingComponents.py
  PROPERTIES TIMEOUT 600 )

atlas_add_test( ActsCheckObjectCounts
  SCRIPT test/ActsCheckObjectCounts.sh
  PROPERTIES TIMEOUT 600 )
