# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkMuons )


atlas_add_component( DerivationFrameworkMuons
   src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthContainers AthenaBaseComps AthenaKernel CaloEvent DerivationFrameworkInterfaces 
                  FourMomUtils GaudiKernel ICaloTrkMuIdTools MCTruthClassifierLib 
                  MuonAnalysisInterfacesLib MuonDetDescrUtils RecoToolInterfaces StoreGateLib 
                  TriggerMatchingToolLib TrkExInterfaces TrkSurfaces TruthUtils muonEvent xAODBase xAODEventInfo 
                  xAODJet xAODMuon xAODPrimitives xAODTracking xAODTruth xAODMuonViews)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( data/*.txt )
