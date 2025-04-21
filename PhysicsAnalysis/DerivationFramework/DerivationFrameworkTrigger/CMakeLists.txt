# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkTrigger )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkTrigger
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel CxxUtils DerivationFrameworkInterfaces FourMomUtils GaudiKernel StoreGateLib TrigDecisionToolLib TriggerMatchingToolLib xAODBase xAODCaloEvent xAODCore xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
