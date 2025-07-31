# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( AnalysisAlgorithmsConfig )

atlas_install_python_modules( python/*.py )
atlas_install_data( data/* )

atlas_install_scripts( scripts/CPRun.py )
atlas_install_scripts( scripts/CPGridRun.py )

if( XAOD_STANDALONE )
   atlas_install_scripts( scripts/*_eljob.py )
else()
   atlas_install_scripts( scripts/*_CA.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
endif()

atlas_add_test( ConfigTextCompareBuilder
   SCRIPT python/ConfigText_unitTest.py --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml --compare-builder --check-order
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 30 )

atlas_add_test( ConfigTextCompareBlock
   SCRIPT python/ConfigText_unitTest.py --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml --compare-block --check-order
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 30 )
atlas_add_test( TestCPRunFullSim
   SCRIPT CPRun.py --input-list $ASG_TEST_FILE_MC --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml -e 150
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 1000
   )
atlas_add_test( TestCPRunMultiMuon
   SCRIPT CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AnalysisAlgorithmsConfig/test_configuration_multi_muon.yaml -e 150
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 1000
   )
