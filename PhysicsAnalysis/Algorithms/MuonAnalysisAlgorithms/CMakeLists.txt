# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( MuonAnalysisAlgorithms )

atlas_add_library( MuonAnalysisAlgorithmsLib
   MuonAnalysisAlgorithms/*.h MuonAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS MuonAnalysisAlgorithms
   LINK_LIBRARIES xAODEventInfo xAODMuon SelectionHelpersLib
   SystematicsHandlesLib MuonAnalysisInterfacesLib IsolationSelectionLib
   AnaAlgorithmLib
   PRIVATE_LINK_LIBRARIES RootCoreUtils )

atlas_add_component( MuonAnalysisAlgorithms
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES MuonAnalysisAlgorithmsLib )

atlas_install_python_modules( python/*.py )
atlas_install_data( data/* )

atlas_add_test( TestCPRun2Muons
   SCRIPT CPRun.py --text-config MuonAnalysisAlgorithms/test_configuration_Run2_Muons.yaml -e 30 --input-list $ASG_TEST_FILE_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400 
)

atlas_add_test( TestCPRun3Muons
   SCRIPT CPRun.py --text-config MuonAnalysisAlgorithms/test_configuration_Run3_Muons.yaml -e 30 --input-list $ASG_TEST_FILE_RUN3_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400
)



