# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigLongLivedParticlesHypo )

# External dependencies:
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( TrigLongLivedParticlesHypo
                     src/*.cxx
                     src/components/*.cxx
		     LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthViews AthenaBaseComps AthenaKernel AthenaMonitoringKernelLib CxxUtils DecisionHandlingLib FourMomUtils GaudiKernel LumiBlockCompsLib LumiBlockData MVAUtils PathResolver StoreGateLib TrigCompositeUtilsLib TrigSteeringEvent TrkTrack xAODJet xAODTracking xAODTrigger BeamSpotConditionsData TrigInDetToolInterfacesLib TrigInDetEvent TrkPrepRawData TrkRIO_OnTrack xAODTrigger TrigInDetPattRecoToolsLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )
