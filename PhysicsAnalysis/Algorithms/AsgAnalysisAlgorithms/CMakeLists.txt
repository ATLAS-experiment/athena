# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( AsgAnalysisAlgorithms )

find_package( ROOT COMPONENTS Core Hist Tree ROOTNTuple )
find_package( VDT )
find_package( Lhapdf )

atlas_add_library( AsgAnalysisAlgorithmsLib
  AsgAnalysisAlgorithms/*.h AsgAnalysisAlgorithms/*.icc Root/*.cxx
  PUBLIC_HEADERS AsgAnalysisAlgorithms
  PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${LHAPDF_INCLUDE_DIRS}
  LINK_LIBRARIES AthContainers AthContainersInterfaces AsgTools AsgMessagingLib AsgServicesLib CxxUtils xAODBase
  xAODEventInfo xAODCutFlow SelectionHelpersLib SystematicsHandlesLib PATCoreLib
  AnaAlgorithmLib AsgAnalysisInterfaces AssociationUtilsLib GammaORToolsLib
  EventBookkeeperToolsLib InDetTrackSystematicsToolsLib IsolationSelectionLib PMGAnalysisInterfacesLib FakeBkgToolsLib xAODMissingET
  PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PATInterfaces xAODCore xAODMetaData xAODJet xAODMuon xAODTau xAODEgamma
  xAODTracking xAODTruth xAODRootAccess RootCoreUtils TruthUtils ${LHAPDF_LIBRARIES})


if( XAOD_STANDALONE )
  target_link_libraries( AsgAnalysisAlgorithmsLib PUBLIC EventLoop )
endif()

atlas_add_dictionary( AsgAnalysisAlgorithmsDict
  AsgAnalysisAlgorithms/AsgAnalysisAlgorithmsDict.h
  AsgAnalysisAlgorithms/selection.xml
  LINK_LIBRARIES AsgAnalysisAlgorithmsLib )

atlas_add_component( AsgAnalysisAlgorithms
  src/components/*.cxx
  INCLUDE_DIRS  ${LHAPDF_INCLUDE_DIRS} ${VDT_INCLUDE_DIRS}
  LINK_LIBRARIES AsgAnalysisAlgorithmsLib ${LHAPDF_LIBRARIES} ${ROOT_LIBRARIES} )

atlas_install_data( data/* )
atlas_install_python_modules( python/*.py )

find_package( GTest )
find_package( GMock )

if (NOT COLUMNAR_ANALYSIS)

  atlas_add_test( gt_BootstrapGenerator
    SOURCES test/gt_BootstrapGenerator.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} AsgTestingLib AsgAnalysisAlgorithmsLib CxxUtils
    POST_EXEC_SCRIPT noerror.sh )


  if( NOT XAOD_STANDALONE )

    atlas_install_scripts(share/test*)

    atlas_add_test( PMGTruthWeightsAlg
      SCRIPT testPMGTruthWeightsAlg.py --max-events 10
      POST_EXEC_SCRIPT noerror.sh )

    atlas_add_test( PileupReweightingAlg
      SCRIPT testPileupReweightingAlgConfig.py --max-events 10
      POST_EXEC_SCRIPT noerror.sh )

  endif()

  # a test job that includes both the Final MET terms and all MET terms
  # in the output file
  atlas_add_test( TestFullTermsOutput
    SCRIPT CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_full_terms_output.yaml -e 150
    POST_EXEC_SCRIPT noerror.sh
    PRIVATE_WORKING_DIRECTORY
    PROPERTIES TIMEOUT 400
  )

  # a test job that writes out an n-tuple with event shape variables
  # in the output file
  atlas_add_test( TestEventShapeOutput
    SCRIPT CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_event_shape_output.yaml -e 150
    POST_EXEC_SCRIPT noerror.sh
    PRIVATE_WORKING_DIRECTORY
    PROPERTIES TIMEOUT 400
  )

  atlas_install_scripts(share/testOutputContent.py test/AsgAnalysisAlgorithms_TestSingleOR.sh test/AsgAnalysisAlgorithms_TestSingleORNoEnforce.sh test/AsgAnalysisAlgorithms_TestMultipleORs.sh)
  # a test job that runs the "standard" OR configuration: exactly one OR where the OR decisions are propagated to all other selections
  # and no OR branches are created in the output
  atlas_add_test( TestSingleOR
    SCRIPT AsgAnalysisAlgorithms_TestSingleOR.sh
    POST_EXEC_SCRIPT noerror.sh
    PRIVATE_WORKING_DIRECTORY
    PROPERTIES TIMEOUT 400
  )

  # a test job that runs the "standard" OR configuration but does not enforce it everywhere: exactly one OR where the OR decisions are a dedicated selection
  # and OR branches are created in the output
  atlas_add_test( TestSingleORNoEnforce
    SCRIPT AsgAnalysisAlgorithms_TestSingleORNoEnforce.sh
    POST_EXEC_SCRIPT noerror.sh
    PRIVATE_WORKING_DIRECTORY
    PROPERTIES TIMEOUT 400
  )

  # a test job that includes multiple ORs and checks if the associated
  # branches are created in the output
  atlas_add_test( TestMultipleORs
    SCRIPT AsgAnalysisAlgorithms_TestMultipleORs.sh
    POST_EXEC_SCRIPT noerror.sh
    PRIVATE_WORKING_DIRECTORY
    PROPERTIES TIMEOUT 400
  )

endif()
