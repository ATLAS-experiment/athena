# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AsgTools AthenaBaseComps AthenaMonitoringLib ElectronPhotonSelectorToolsLib EventPrimitives GaudiKernel GeneratorObjects ITrackToVertex IdDictDetDescr BeamSpotConditionsData InDetIdentifier InDetPrepRawData InDetRIO_OnTrack InDetTrackSelectionToolLib JetInterface MuonAnalysisInterfacesLib Particle StoreGateLib TRT_ReadoutGeometry TrigDecisionToolLib TrigConfxAODLib TriggerMatchingToolLib TrkEventPrimitives TrkExInterfaces TrkParameters TrkParticleBase TrkTrack TrkTrackSummary TrkTruthData TrkV0Vertex TrkVertexAnalysisUtilsLib TrkVertexFitterInterfaces TrackVertexAssociationToolLib egammaEvent egammaInterfacesLib xAODCaloEvent xAODEgamma xAODEventInfo xAODJet xAODMissingET xAODMuon xAODTracking xAODTruth TruthUtils )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
