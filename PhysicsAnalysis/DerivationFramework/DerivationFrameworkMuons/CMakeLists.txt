# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkMuons )

# Component(s) in the package:
#the library is used by the MCP software https://gitlab.cern.ch/atlas-mcp/MuonPerformanceAnalysis
atlas_add_library( DerivationFrameworkMuonsLib
   DerivationFrameworkMuons/*.h
   INTERFACE
   PUBLIC_HEADERS DerivationFrameworkMuons
   LINK_LIBRARIES StoreGateLib )

atlas_add_component( DerivationFrameworkMuons
   src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthContainers AthenaBaseComps AthenaKernel CaloEvent DerivationFrameworkInterfaces DerivationFrameworkMuonsLib FourMomUtils GaudiKernel ICaloTrkMuIdTools MCTruthClassifierLib MuonAnalysisInterfacesLib MuonDetDescrUtils RecoToolInterfaces StoreGateLib TriggerMatchingToolLib TrkExInterfaces TrkSurfaces TruthUtils muonEvent xAODBase xAODEventInfo xAODJet xAODMuon xAODPrimitives xAODTracking xAODTruth )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( data/*.txt )
