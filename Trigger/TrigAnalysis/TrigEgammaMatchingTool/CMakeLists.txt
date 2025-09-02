# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigEgammaMatchingTool )

# External dependencies:
find_package( Boost )

# Component(s) in the package:
atlas_add_library( TrigEgammaMatchingToolLib
                   Root/*.cxx
                   PUBLIC_HEADERS TrigEgammaMatchingTool
                   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES DecisionHandlingLib TrigCompositeUtilsLib TrigSteeringEvent xAODRootAccess
                   LINK_LIBRARIES ${Boost_LIBRARIES} AsgTools TrigNavStructure TrigDecisionToolLib xAODCaloEvent xAODEgamma xAODTrigRinger xAODTrigCalo xAODTrigEgamma xAODTrigger )

atlas_add_component( TrigEgammaMatchingTool
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaMonitoringLib GaudiKernel StoreGateLib TrigCompositeUtilsLib TrigEgammaMatchingToolLib xAODMuon xAODTau )
