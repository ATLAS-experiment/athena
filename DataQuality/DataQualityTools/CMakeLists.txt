# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DataQualityTools )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist )

# Component(s) in the package:
atlas_add_component( DataQualityTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthenaMonitoringKernelLib AthenaMonitoringLib GaudiKernel InDetIdentifier InDetPrepRawData InDetRawData LArRawEvent LArRecEvent LUCID_RawEvent LWHists MCTruthClassifierLib MagFieldConditions MagFieldElements MuonAnalysisInterfacesLib MuonRDO RecBackgroundEvent StoreGateLib TileEvent TrigT1Result TriggerMatchingToolLib TrkSpacePoint xAODEgamma xAODEventInfo xAODMuon xAODTracking xAODTruth )

# Install files from the package:
atlas_install_python_modules( python/*.py )
