# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkMCTruth )

# External dependencies:
find_package( FastJet )
find_package( ROOT COMPONENTS Core Hist )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkMCTruth
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${FASTJET_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${FASTJET_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthContainers AthenaKernel AtlasHepMCLib AtlasHepMCsearchLib CxxUtils DerivationFrameworkInterfaces EventInfoMgtLib ExpressionEvaluationLib GaudiKernel GenInterfacesLib GeneratorObjects MCTruthClassifierLib StoreGateLib TauAnalysisToolsLib TruthUtils xAODBase xAODEgamma xAODEventInfo xAODEventShape xAODJet xAODMuon xAODTruth xAODCore AthenaPoolUtilities)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
