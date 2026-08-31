# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HLTSeeding )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( HLTSeedingLib
                   PUBLIC_HEADERS HLTSeeding
                   INTERFACE 
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel StoreGateLib TrigCompositeUtilsLib TrigConfData BeamSpotConditionsData )
                   
atlas_add_component( HLTSeeding
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} 
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaKernel AthenaMonitoringKernelLib
                     GaudiKernel L1TopoAlgorithms L1TopoCoreSim HLTSeedingLib StoreGateLib TrigCompositeUtilsLib TrigConfxAODLib TrigCostMonitorLib 
                     TrigSteeringEvent TrigT1Interfaces TrigT1Result TrigTimeAlgsLib xAODEventInfo xAODTrigger xAODCore RoiDescriptor )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests:
atlas_add_test( HLTSeedingConfig
                SCRIPT python -m HLTSeeding.HLTSeedingConfig
                POST_EXEC_SCRIPT noerror.sh )
