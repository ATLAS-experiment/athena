# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( AsgAnalysisAlgorithms )

find_package( ROOT COMPONENTS Core Hist Tree )

atlas_add_library( AsgAnalysisAlgorithmsLib
   AsgAnalysisAlgorithms/*.h AsgAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS AsgAnalysisAlgorithms
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES AthContainers AthContainersInterfaces AsgTools AsgMessagingLib AsgServicesLib CxxUtils xAODBase
     xAODEventInfo xAODCutFlow SelectionHelpersLib SystematicsHandlesLib PATCoreLib
     AnaAlgorithmLib AsgAnalysisInterfaces AssociationUtilsLib GammaORToolsLib
     EventBookkeeperToolsLib IsolationSelectionLib PMGAnalysisInterfacesLib FakeBkgToolsLib
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PATInterfaces xAODCore xAODMetaData xAODJet xAODMuon xAODTau xAODEgamma xAODMissingET
     xAODTracking xAODTruth xAODRootAccess RootCoreUtils TruthUtils )

atlas_add_dictionary( AsgAnalysisAlgorithmsDict
   AsgAnalysisAlgorithms/AsgAnalysisAlgorithmsDict.h
   AsgAnalysisAlgorithms/selection.xml
   LINK_LIBRARIES AsgAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( AsgAnalysisAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AsgAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py )

find_package( GTest )
find_package( GMock )

atlas_add_test( gt_BootstrapGenerator
   SOURCES test/gt_BootstrapGenerator.cxx
   INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
   LINK_LIBRARIES ${GTEST_LIBRARIES} AsgTestingLib AsgAnalysisAlgorithmsLib CxxUtils
   POST_EXEC_SCRIPT nopost.sh )

if( NOT XAOD_STANDALONE )

   atlas_install_scripts(share/test*)

   atlas_add_test( PMGTruthWeightsAlg
    SCRIPT testPMGTruthWeightsAlg.py --max-events 10
    POST_EXEC_SCRIPT nopost.sh )

   atlas_add_test( PileupReweightingAlg
    SCRIPT testPileupReweightingAlgConfig.py --max-events 10
    POST_EXEC_SCRIPT nopost.sh )

endif()
