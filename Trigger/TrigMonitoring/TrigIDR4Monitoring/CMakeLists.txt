# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigIDR4Monitoring )

# External dependencies:

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_component( TrigIDR4Monitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaMonitoringKernelLib AthenaMonitoringLib AtlasHepMCLib EventInfo GaudiKernel GeneratorObjects  McParticleEvent TrigCompositeUtilsLib TrigDecisionToolLib TrigInDetAnalysis TrigInDetAnalysisUtils TrigInDetAnalysisExampleLib TrigSteeringEvent xAODEventInfo xAODTracking )

