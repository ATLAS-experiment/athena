# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TgcRawDataMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore )

atlas_add_component( TgcRawDataMonitoring
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthenaMonitoringLib FourMomUtils GaudiKernel GeoPrimitives GoodRunsListsLib MuonAnalysisInterfacesLib MuonIdHelpersLib MuonPrepRawData MuonReadoutGeometry MuonTrigCoinData StoreGateLib TrigConfData TrigDecisionToolLib TrkExInterfaces TrkSurfaces TruthUtils xAODMuon xAODTracking xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

