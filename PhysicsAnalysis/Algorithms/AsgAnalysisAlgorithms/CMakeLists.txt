# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

atlas_add_component( AsgAnalysisAlgorithms
   src/components/*.cxx
   LINK_LIBRARIES AsgAnalysisAlgorithmsLib )

atlas_install_data( data/* )
atlas_install_python_modules( python/*.py )

find_package( GTest )
find_package( GMock )

if (NOT COLUMNAR_ANALYSIS)

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

      atlas_install_scripts(share/testOutputContent.py)
      # a test job that runs the "standard" OR configuration: exactly one OR where the OR decisions are propagated to all other selections
      # and no OR branches are created in the output
      atlas_add_test( TestSingleOR
         PRE_EXEC_SCRIPT "CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_single_OR.yaml -e 150 -o output_singleOR.root"
         SCRIPT testOutputContent.py --input_file output_singleOR.root --tree_name analysis --forbidden-branches el_select_passesOR_NOSYS jet_select_passesOR_NOSYS mu_select_passesOR_NOSYS ph_select_passesOR_NOSYS tau_select_passesOR_NOSYS
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 400
         )

      # a test job that runs the "standard" OR configuration but does not enforce it everywhere: exactly one OR where the OR decisions are a dedicated selection
      # and OR branches are created in the output
      atlas_add_test( TestSingleORNoEnforce
         PRE_EXEC_SCRIPT "CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_single_OR_noEnforce.yaml -e 150 -o output_singleOR_noEnforce.root"
         SCRIPT testOutputContent.py --input_file output_singleOR_noEnforce.root --tree_name analysis --branches el_select_passesOR_NOSYS jet_select_passesOR_NOSYS mu_select_passesOR_NOSYS ph_select_passesOR_NOSYS tau_select_passesOR_NOSYS
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 400
         )

      # a test job that includes multiple ORs and checks if the associated
      # branches are created in the output
      atlas_add_test( TestMultipleORs
         PRE_EXEC_SCRIPT "CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_multiple_ORs.yaml -e 150 -o output_multipleORs.root"
         SCRIPT testOutputContent.py --input_file output_multipleORs.root --tree_name analysis --branches el_select_passesOR_postJvt_NOSYS el_select_passesOR_preJvt_NOSYS jet_select_passesOR_postJvt_NOSYS jet_select_passesOR_preJvt_NOSYS mu_select_passesOR_postJvt_NOSYS mu_select_passesOR_preJvt_NOSYS ph_select_passesOR_postJvt_NOSYS ph_select_passesOR_preJvt_NOSYS tau_select_passesOR_postJvt_NOSYS tau_select_passesOR_preJvt_NOSYS
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 400
         )

   endif()

   # a test job that includes both the Final MET terms and all MET terms
   # in the output file
   atlas_add_test( TestFullTermsOutput
      SCRIPT CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_full_terms_output.yaml -e 150
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 400
      )

   if( XAOD_STANDALONE )

      atlas_install_scripts(share/testOutputContent.py)
      # a test job that runs the "standard" OR configuration: exactly one OR where the OR decisions are propagated to all other selections
      # and no OR branches are created in the output
      atlas_add_test( TestSingleOR
         PRE_EXEC_SCRIPT "CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_single_OR.yaml -e 150 -o output_singleOR.root --work-dir singleOR"
         SCRIPT testOutputContent.py --input_file singleOR/data-ANALYSIS/output_singleOR.root --tree_name analysis --forbidden-branches el_select_passesOR_NOSYS jet_select_passesOR_NOSYS mu_select_passesOR_NOSYS ph_select_passesOR_NOSYS tau_select_passesOR_NOSYS
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 400
         )

      # a test job that runs the "standard" OR configuration but does not enforce it everywhere: exactly one OR where the OR decisions are a dedicated selection
      # and OR branches are created in the output
      atlas_add_test( TestSingleORNoEnforce
         PRE_EXEC_SCRIPT "CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_single_OR_noEnforce.yaml -e 150 -o output_singleOR_noEnforce.root --work-dir singleOR_noEnforce"
         SCRIPT testOutputContent.py --input_file singleOR_noEnforce/data-ANALYSIS/output_singleOR_noEnforce.root --tree_name analysis --branches el_select_passesOR_NOSYS jet_select_passesOR_NOSYS mu_select_passesOR_NOSYS ph_select_passesOR_NOSYS tau_select_passesOR_NOSYS
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 400
         )

      # a test job that includes multiple ORs and checks if the associated
      # branches are created in the output
      atlas_add_test( TestMultipleORs
         PRE_EXEC_SCRIPT "CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_multiple_ORs.yaml -e 150 -o output_multipleORs.root  --work-dir multipleORs"
         SCRIPT testOutputContent.py --input_file multipleORs/data-ANALYSIS/output_multipleORs.root --tree_name analysis --branches el_select_passesOR_postJvt_NOSYS el_select_passesOR_preJvt_NOSYS jet_select_passesOR_postJvt_NOSYS jet_select_passesOR_preJvt_NOSYS mu_select_passesOR_postJvt_NOSYS mu_select_passesOR_preJvt_NOSYS ph_select_passesOR_postJvt_NOSYS ph_select_passesOR_preJvt_NOSYS tau_select_passesOR_postJvt_NOSYS tau_select_passesOR_preJvt_NOSYS
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 400
         )
   endif()

endif()