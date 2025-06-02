# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkHI)

# Components in the package:
atlas_add_component( DerivationFrameworkHI
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps DerivationFrameworkInterfaces GaudiKernel StoreGateLib PATCoreLib InDetTrackSelectionToolLib xAODHIEvent AsgTools PathResolver)

atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Install files from the package:
atlas_install_joboptions( python/*.py )
