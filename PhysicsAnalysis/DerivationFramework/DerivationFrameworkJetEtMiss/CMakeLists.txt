# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkJetEtMiss )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkJetEtMiss
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AthLinks AthenaBaseComps DerivationFrameworkInterfaces FTagAnalysisInterfacesLib GaudiKernel InDetTrackSelectionToolLib JetAnalysisInterfacesLib JetInterface LumiBlockData PFlowUtilsLib ParticleJetToolsLib PathResolver StoreGateLib TrackVertexAssociationToolLib xAODCaloEvent xAODCore xAODEgamma xAODEventInfo xAODJet xAODMissingET xAODMuon xAODPFlow xAODTau xAODTracking xAODTrigger xAODTruth )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
