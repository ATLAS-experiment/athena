# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( JetAnalysisAlgorithms )

# FastJet - needed for jet reclustering
find_package( FastJet COMPONENTS fastjetplugins fastjettools )

atlas_add_library( JetAnalysisAlgorithmsLib
   JetAnalysisAlgorithms/*.h JetAnalysisAlgorithms/*.icc Root/*.cxx
   INCLUDE_DIRS ${FASTJET_INCLUDE_DIRS}
   PUBLIC_HEADERS JetAnalysisAlgorithms
   LINK_LIBRARIES xAODJet xAODMuon SelectionHelpersLib SystematicsHandlesLib
   AnaAlgorithmLib JetCalibToolsLib JetInterface
   JetCPInterfaces JetAnalysisInterfacesLib AsgDataHandlesLib ${FASTJET_LIBRARIES}
   PRIVATE_LINK_LIBRARIES METUtilitiesLib xAODMuon )

atlas_add_component( JetAnalysisAlgorithms
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AnaAlgorithmLib JetAnalysisInterfacesLib JetCPInterfaces JetCalibToolsLib
   JetInterface SelectionHelpersLib SystematicsHandlesLib xAODJet JetAnalysisAlgorithmsLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( data/* )

atlas_add_test( TestCPRun2Jets
   SCRIPT CPRun.py  --text-config JetAnalysisAlgorithms/test_configuration_Run2_Jets.yaml -e 30 --input-list $ASG_TEST_FILE_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400 
)

atlas_add_test( TestCPRun3Jets
   SCRIPT CPRun.py  --text-config JetAnalysisAlgorithms/test_configuration_Run3_Jets.yaml -e 30 --input-list $ASG_TEST_FILE_RUN3_MC
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 400
)

