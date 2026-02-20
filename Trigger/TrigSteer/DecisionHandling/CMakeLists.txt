# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DecisionHandling )

atlas_add_library( DecisionHandlingLib
                   src/ComboHypo.cxx
                   src/ComboHypoToolBase.cxx
                   src/DebugComboHypoTool.cxx
                   src/DumpDecisions.cxx
                   src/HypoBase.cxx
                   src/InputMakerBase.cxx
		           src/ITestHypoTool.cxx
                   PUBLIC_HEADERS DecisionHandling
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel StoreGateLib TrigCompositeUtilsLib TrigSteeringEvent xAODTrigger
                   PRIVATE_LINK_LIBRARIES AthViews )

# Component(s) in the package:
atlas_add_component( DecisionHandling
                     src/components/*.cxx
                     src/DeltaRRoIComboHypoTool.cxx
                     src/InputMakerForRoI.cxx
                     src/TriggerSummaryAlg.cxx
                     src/RoRSeqFilter.cxx
                     src/ViewCreator*.cxx
                     src/TestRecoAlg.cxx
                     src/TestInputMaker.cxx
                     src/TestHypoTool.cxx
                     src/TestHypoAlg.cxx
                     src/PassFilter.cxx
		             src/Roi*.cxx
                     LINK_LIBRARIES AthContainers AthViews AthenaKernel AthenaMonitoringKernelLib CxxUtils DecisionHandlingLib HLTSeedingLib TrigCompositeUtilsLib TrigCostMonitorLib TrigSteeringEvent TrigTimeAlgsLib xAODJet xAODMuon xAODTracking xAODTrigCalo xAODTrigger )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
#atlas_install_joboptions( share/*.py )
atlas_install_data( share/*.ref)

# Tests

atlas_add_test( test_ComboHypoTool
    SOURCES test/test_ComboHypoTool.cxx
    LINK_LIBRARIES AthenaKernel CxxUtils DecisionHandlingLib GaudiKernel SGTools StoreGateLib TestTools TrigCompositeUtilsLib xAODBase xAODEgamma xAODJet xAODMuon xAODTrigger
    LOG_SELECT_PATTERN "^Combo|^HLT_"
    ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share" )
