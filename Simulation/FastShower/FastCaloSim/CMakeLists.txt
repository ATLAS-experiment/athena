# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FastCaloSim )

# Component(s) in the package:
# Avoid colliding with the external libFastCaloSim library.
atlas_add_component( FastCaloSimAthenaComponents
                     src/CaloCellContainerFCSFinalizerTool.cxx
                     src/EmptyCellBuilderTool.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthAllocators AthenaBaseComps CaloEvent CaloDetDescrLib CaloIdentifier CaloInterfaceLib GaudiKernel StoreGateLib TileEvent )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
