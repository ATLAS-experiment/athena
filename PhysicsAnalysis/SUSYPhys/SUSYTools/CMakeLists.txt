# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SUSYTools )

# Extra dependencies based on the build environment:
set( extra_libs )
if( NOT XAOD_STANDALONE )
   set( extra_libs GaudiKernel AthAnalysisBaseCompsLib )
endif()

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )
find_package( GTest )

# Libraries in the package:
atlas_add_library( SUSYToolsLib
   Root/*.cxx
   PUBLIC_HEADERS SUSYTools
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES AsgTools PATCoreLib xAODCore xAODEgamma xAODEventInfo
   xAODJet xAODMissingET xAODMuon xAODTau xAODTracking xAODTruth
   AssociationUtilsLib BoostedJetTaggersLib GoodRunsListsLib PATInterfaces TrigDecisionToolLib
   MCTruthClassifierLib JetJvtEfficiencyLib JetSubStructureUtils
   ParticleJetToolsLib PMGAnalysisInterfacesLib AnaAlgorithmLib
   MuonAnalysisInterfacesLib EgammaAnalysisInterfacesLib
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES}
   AthContainers EventPrimitives FourMomUtils xAODBTagging xAODBase
   xAODPrimitives xAODRootAccess
   EgammaAnalysisHelpersLib FTagAnalysisInterfacesLib IsolationSelectionLib 
   IsolationCorrectionsLib JetAnalysisInterfacesLib JetSelectorToolsLib
   JetCPInterfaces JetCalibToolsLib JetInterface
   InDetTrackSystematicsToolsLib
   # JetResolutionLib
   JetUncertaintiesLib JetMomentToolsLib METInterface METUtilitiesLib
   PileupReweightingLib PathResolver TauAnalysisToolsLib
   TriggerAnalysisInterfaces
   TrigGlobalEfficiencyCorrectionLib TriggerMatchingToolLib TrigConfInterfaces TrigConfxAODLib
   xAODTrigMissingET xAODMetaData xAODCutFlow ${extra_libs} )

if( NOT XAOD_STANDALONE )
   atlas_add_component( SUSYTools
      src/*.cxx src/components/*.cxx
      NOCLIDDB
      PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthAnalysisBaseCompsLib SUSYToolsLib TauAnalysisToolsLib )
   atlas_add_dictionary( SUSYToolsDict
           SUSYTools/SUSYToolsDict.h
           SUSYTools/selection.xml
           LINK_LIBRARIES SUSYToolsLib )
else()
   atlas_add_library( SUSYTools src/SUSYToolsAlg.cxx SUSYTools/SUSYToolsAlg.h
           NO_PUBLIC_HEADERS
           PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
           PRIVATE_LINK_LIBRARIES SUSYToolsLib TauAnalysisToolsLib
           )
   atlas_add_dictionary( SUSYToolsDict
           SUSYTools/SUSYToolsDict.h
           SUSYTools/selection.xml
           LINK_LIBRARIES SUSYToolsLib SUSYTools )
endif()



# Executable(s) in the package:
set( extra_libs )
if( NOT XAOD_STANDALONE )
   set( extra_libs POOLRootAccessLib )
endif()

if( XAOD_STANDALONE )
   atlas_add_executable( SUSYToolsTester
      util/SUSYToolsTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo xAODMuon
      xAODEgamma xAODTau xAODTruth xAODJet xAODCaloEvent xAODCore xAODMissingET
      FTagAnalysisInterfacesLib xAODBase TauAnalysisToolsLib GoodRunsListsLib
      PileupReweightingLib PATInterfaces PathResolver METUtilitiesLib xAODCutFlow
      SUSYToolsLib ${extra_libs} )

   atlas_add_executable( SUSYTruthTester
      util/SUSYTruthTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo
      xAODTruth xAODCore xAODBase PATInterfaces PathResolver xAODCutFlow
      SUSYToolsLib ${extra_libs} )

   atlas_add_executable( SUSYTools_check_xsections
      util/check_xsections.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} SUSYToolsLib )

   atlas_add_executable( SUSYTools_check_syst
      util/check_syst.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess PATInterfaces
      SUSYToolsLib )
endif()

atlas_add_test( SUSYCrossSection_test
  SOURCES  test/SUSYCrossSection_test.cxx
  LINK_LIBRARIES SUSYToolsLib
  POST_EXEC_SCRIPT nopost.sh
)

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )
atlas_install_data( data/* )
atlas_install_scripts( scripts/TestSUSYToolsAlg.py )
