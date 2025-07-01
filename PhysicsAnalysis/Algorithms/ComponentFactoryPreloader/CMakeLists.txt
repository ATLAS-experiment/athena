# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

if( XAOD_STANDALONE )

atlas_subdir( ComponentFactoryPreloader )

atlas_add_library( ComponentFactoryPreloaderLib
   ComponentFactoryPreloader/*.h ComponentFactoryPreloader/*.icc Root/*.cxx
   PUBLIC_HEADERS ComponentFactoryPreloader
   LINK_LIBRARIES
   PRIVATE_LINK_LIBRARIES AsgTools
      AsgAnalysisAlgorithmsLib EgammaAnalysisAlgorithmsLib KLFitterAnalysisAlgorithmsLib EventSelectionAlgorithmsLib FTagAnalysisAlgorithmsLib JetAnalysisAlgorithmsLib MetAnalysisAlgorithmsLib MuonAnalysisAlgorithmsLib StandaloneAnalysisAlgorithmsLib TauAnalysisAlgorithmsLib TrackingAnalysisAlgorithmsLib TriggerAnalysisAlgorithmsLib TruthParticleLevelAnalysisAlgorithmsLib
      AssociationUtilsLib ElectronPhotonFourMomentumCorrectionLib ElectronPhotonSelectorToolsLib IsolationCorrectionsLib IsolationSelectionLib MuonMomentumCorrectionsLib MuonSelectorToolsLib PileupReweightingLib TauAnalysisToolsLib xAODBTaggingEfficiencyLib
      JetCalibToolsLib JetJvtEfficiencyLib JetMomentToolsLib JetUncertaintiesLib METUtilitiesLib egammaMVACalibLib tauRecToolsLib
      GoodRunsListsLib InDetTrackSystematicsToolsLib
      TrigConfxAODLib TrigDecisionToolLib TrigGlobalEfficiencyCorrectionLib TriggerMatchingToolLib
      LRTElectronAnalysisToolsLib LRTMuonAnalysisToolsLib PhotonEfficiencyCorrectionLib MuonEfficiencyCorrectionsLib PMGToolsLib
   )

atlas_add_dictionary( ComponentFactoryPreloaderDict
   ComponentFactoryPreloader/ComponentFactoryPreloaderDict.h
   ComponentFactoryPreloader/selection.xml
   LINK_LIBRARIES ComponentFactoryPreloaderLib )

endif ()
