# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkCore )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkCore
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AsgAnalysisInterfaces AthAnalysisBaseCompsLib AthContainers AthLinks AthenaBaseComps AthenaKernel CxxUtils DerivationFrameworkInterfaces GoodRunsListsLib PathResolver SGTools xAODEventInfo StoreGateLib GaudiKernel )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
