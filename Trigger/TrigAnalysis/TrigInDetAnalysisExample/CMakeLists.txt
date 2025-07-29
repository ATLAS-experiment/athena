# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigInDetAnalysisExample )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree Hist RIO )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Component(s) in the package:
atlas_add_library( TrigInDetAnalysisExampleLib
                   src/*.cxx
                   PUBLIC_HEADERS TrigInDetAnalysisExample
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthenaMonitoringKernelLib AthenaMonitoringLib AtlasHepMCLib EventInfo GaudiKernel GeneratorObjects  McParticleEvent TrigCompositeUtilsLib TrigDecisionToolLib TrigAnalysisHelpersLib TrigInDetAnalysis TrigInDetAnalysisUtils TrigSteeringEvent VxVertex egammaEvent muonEvent tauEvent xAODEventInfo xAODTracking
                   PRIVATE_LINK_LIBRARIES TrkParameters TrkTrack xAODTruth )

atlas_add_component( TrigInDetAnalysisExample
                     src/components/*.cxx
                     LINK_LIBRARIES TrigInDetAnalysisExampleLib )
