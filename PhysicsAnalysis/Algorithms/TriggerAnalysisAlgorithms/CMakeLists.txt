# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Tadej Novak

atlas_subdir( TriggerAnalysisAlgorithms )

atlas_add_library( TriggerAnalysisAlgorithmsLib
   TriggerAnalysisAlgorithms/*.h TriggerAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS TriggerAnalysisAlgorithms
   LINK_LIBRARIES xAODEventInfo SelectionHelpersLib SystematicsHandlesLib
      AnaAlgorithmLib AsgTools AsgAnalysisInterfaces EventBookkeeperToolsLib TrigDecisionInterface
      xAODEgamma xAODMuon TriggerAnalysisInterfaces TrigCompositeUtilsLib TrigGlobalEfficiencyCorrectionLib TriggerMatchingToolLib
   PRIVATE_LINK_LIBRARIES RootCoreUtils )

atlas_add_component( TriggerAnalysisAlgorithms
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES TriggerAnalysisAlgorithmsLib )

atlas_install_python_modules( python/*.py )
