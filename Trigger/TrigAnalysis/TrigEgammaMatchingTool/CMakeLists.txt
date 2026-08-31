# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigEgammaMatchingTool )

# Component(s) in the package:
atlas_add_library( TrigEgammaMatchingToolLib
                   Root/*.cxx
                   PUBLIC_HEADERS TrigEgammaMatchingTool
                   PRIVATE_LINK_LIBRARIES DecisionHandlingLib TrigCompositeUtilsLib TrigSteeringEvent
                   LINK_LIBRARIES AsgTools TrigNavStructure TrigDecisionToolLib xAODCaloEvent xAODEgamma xAODTrigRinger xAODTrigCalo xAODTrigEgamma xAODTrigger )

atlas_add_component( TrigEgammaMatchingTool
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaMonitoringLib GaudiKernel StoreGateLib TrigCompositeUtilsLib TrigEgammaMatchingToolLib )
