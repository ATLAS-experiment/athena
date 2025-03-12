# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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

atlas_add_dictionary( JetAnalysisAlgorithmsDict
   JetAnalysisAlgorithms/JetAnalysisAlgorithmsDict.h
   JetAnalysisAlgorithms/selection.xml
   LINK_LIBRARIES JetAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( JetAnalysisAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AnaAlgorithmLib JetAnalysisInterfacesLib JetCPInterfaces JetCalibToolsLib
      JetInterface SelectionHelpersLib SystematicsHandlesLib xAODJet JetAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( data/* )

atlas_add_test( TestCPRunJets
   SCRIPT CPRun.py --work-dir cmake_test_jetsDir --text-config JetAnalysisAlgorithms/test_configuration_Run2_Jets.yaml -e 30 --input-file $ASG_TEST_FILE_MC
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 400 
)