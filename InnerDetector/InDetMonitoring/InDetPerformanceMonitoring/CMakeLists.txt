# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetPerformanceMonitoring )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist Physics Graf )

# Component(s) in the package:
atlas_add_component( InDetPerformanceMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AsgTools AthContainers AthenaBaseComps AthenaKernel AthenaMonitoringLib ElectronPhotonSelectorToolsLib EventPrimitives GaudiKernel GeneratorObjects ITrackToVertex BeamSpotConditionsData InDetTrackSelectionToolLib JetInterface MuonAnalysisInterfacesLib StoreGateLib TrigDecisionToolLib TrigConfxAODLib TriggerMatchingToolLib TrkEventPrimitives TrkExInterfaces TrkParameters TrkTrack TrkTruthData TrkVertexFitterInterfaces TrackVertexAssociationToolLib egammaInterfacesLib xAODCaloEvent xAODEgamma xAODEventInfo xAODJet xAODMissingET xAODMuon xAODTracking xAODTruth TruthUtils )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
