# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrfTestsARTPlots )

# Install files from the package:
atlas_install_scripts( test/*.sh )
atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
atlas_install_data( test/*.css test/*.php )
