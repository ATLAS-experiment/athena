# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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
     AnaAlgorithmLib AsgAnalysisInterfaces AssociationUtilsLib
     EventBookkeeperToolsLib IsolationSelectionLib PMGAnalysisInterfacesLib FakeBkgToolsLib
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PATInterfaces xAODCore xAODMetaData xAODJet xAODMuon xAODTau xAODEgamma xAODMissingET
     xAODTracking xAODTruth xAODRootAccess RootCoreUtils )

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
atlas_install_joboptions( share/*_jobOptions.py )
atlas_install_scripts( share/*_eljob.py )

find_package( GTest )
find_package( GMock )

atlas_add_test( gt_BootstrapGenerator
   SOURCES test/gt_BootstrapGenerator.cxx
   INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
   LINK_LIBRARIES ${GTEST_LIBRARIES} AsgTestingLib AsgAnalysisAlgorithmsLib CxxUtils
   POST_EXEC_SCRIPT nopost.sh )

if( XAOD_STANDALONE )

   atlas_add_test( EventAlgsTestJobData
      SCRIPT EventAlgorithmsTest_eljob.py --data-type data --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( EventAlgsTestJobFullSim
      SCRIPT EventAlgorithmsTest_eljob.py --data-type mc --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( EventAlgsTestJobFastSim
      SCRIPT EventAlgorithmsTest_eljob.py --data-type afii --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( OverlapRemovalTestJobData
      SCRIPT OverlapAlgorithmsTest_eljob.py --data-type data --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( OverlapRemovalTestJobFullSim
      SCRIPT OverlapAlgorithmsTest_eljob.py --data-type mc --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( OverlapRemovalTestJobFastSim
      SCRIPT OverlapAlgorithmsTest_eljob.py --data-type afii --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( GeneratorAlgsTestJobFullSim
      SCRIPT GeneratorAlgorithmsTest_eljob.py --data-type mc --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( PileupAlgsTestJobFullSim
      SCRIPT PileupAlgorithmsTest_eljob.py --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

else()

   atlas_install_scripts(share/test*)

   atlas_add_test( PMGTruthWeightsAlg
    SCRIPT testPMGTruthWeightsAlg.py --max-events 10
    POST_EXEC_SCRIPT nopost.sh )

   atlas_add_test( PileupReweightingAlg
    SCRIPT testPileupReweightingAlgConfig.py --max-events 10
    POST_EXEC_SCRIPT nopost.sh )

   atlas_add_test( EventAlgsTestJobData
      SCRIPT athena.py
             AsgAnalysisAlgorithms/EventAlgorithmsTest_jobOptions.py - --data-type data
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( EventAlgsTestJobFullSim
      SCRIPT athena.py
             AsgAnalysisAlgorithms/EventAlgorithmsTest_jobOptions.py - --data-type mc
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( EventAlgsTestJobFastSim
      SCRIPT athena.py
             AsgAnalysisAlgorithms/EventAlgorithmsTest_jobOptions.py - --data-type afii
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( GeneratorAlgsTestJob
      SCRIPT athena.py
               AsgAnalysisAlgorithms/GeneratorAlgorithmsTest_jobOptions.py - --data-type mc
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( PileupAlgsTestJob
      SCRIPT athena.py
               AsgAnalysisAlgorithms/PileupAlgorithmsTest_jobOptions.py
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   if( NOT "${CMAKE_PROJECT_NAME}" STREQUAL "AthDerivation" )
      atlas_add_test( OverlapRemovalTestJobData
         SCRIPT athena.py
                AsgAnalysisAlgorithms/OverlapAlgorithmsTest_jobOptions.py - --data-type data
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 600 )
      atlas_add_test( OverlapRemovalTestJobFullSim
         SCRIPT athena.py
                AsgAnalysisAlgorithms/OverlapAlgorithmsTest_jobOptions.py - --data-type mc
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 600 )
      atlas_add_test( OverlapRemovalTestJobFastSim
         SCRIPT athena.py
                AsgAnalysisAlgorithms/OverlapAlgorithmsTest_jobOptions.py - --data-type afii
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 600 )
   endif()
endif()
