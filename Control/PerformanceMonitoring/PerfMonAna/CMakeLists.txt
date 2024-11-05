# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PerfMonAna )

# External dependencies:
find_package( matplotlib )
find_package( numpy )
find_package( pandas )
find_package( sqlalchemy )
find_package( zipp )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Install files from the package:
atlas_install_scripts( bin/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
