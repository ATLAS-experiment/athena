# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


# The name of the package:
atlas_subdir( TauAnalysisAlgorithms )

atlas_add_library( TauAnalysisAlgorithmsLib
   TauAnalysisAlgorithms/*.h TauAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS TauAnalysisAlgorithms
   LINK_LIBRARIES xAODTau SelectionHelpersLib DiTauMassToolsLib
   SystematicsHandlesLib
   AnaAlgorithmLib TauAnalysisToolsLib )

atlas_add_component( TauAnalysisAlgorithms src/components/*.cxx
   LINK_LIBRARIES TauAnalysisAlgorithmsLib )

atlas_install_python_modules( python/*.py )

atlas_install_data( data/*.conf )
atlas_install_data( data/* )

atlas_add_test( TestCPRun2Taus
   SCRIPT CPRun.py --text-config TauAnalysisAlgorithms/test_configuration_Run2_Taus.yaml -e 30 --input-list $ASG_TEST_FILE_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400 
)

atlas_add_test( TestCPRun2TausExpert
   SCRIPT CPRun.py --text-config TauAnalysisAlgorithms/test_configuration_Run2_Taus_expert.yaml -e 30 --input-list $ASG_TEST_FILE_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400
)

atlas_add_test( TestCPRun2DiTaus
   SCRIPT CPRun.py --text-config TauAnalysisAlgorithms/test_configuration_Run2_DiTaus.yaml -e 30 --input-list $ASG_TEST_FILE_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400
)

atlas_add_test( TestCPRun3Taus
   SCRIPT CPRun.py --text-config TauAnalysisAlgorithms/test_configuration_Run3_Taus.yaml -e 30 --input-list $ASG_TEST_FILE_RUN3_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400
)

atlas_add_test( TestCPRun3TausExpert
   SCRIPT CPRun.py --text-config TauAnalysisAlgorithms/test_configuration_Run3_Taus_expert.yaml -e 30 --input-list $ASG_TEST_FILE_RUN3_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400
)

atlas_add_test( TestCPRun3DiTaus
   SCRIPT CPRun.py --text-config TauAnalysisAlgorithms/test_configuration_Run3_DiTaus.yaml -e 30 --input-list $ASG_TEST_FILE_RUN3_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400
)


