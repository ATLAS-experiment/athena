# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( AnalysisAlgorithmsConfig )

atlas_install_python_modules( python/*.py )
atlas_install_data( data/* )

set( CONFIG_PATH "${CMAKE_CURRENT_LIST_DIR}/data/test_configuration_Run2.yaml" )
set( ASG_TEST_FILE_MC "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p6490/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13167_r13146_p6490/DAOD_PHYS.41651753._000001.pool.root.1")

atlas_install_scripts( scripts/CPRun.py )

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
   SCRIPT CPRun.py --work-dir cmake_test --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml -e 150 --input-file ${ASG_TEST_FILE_MC}
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 400 
   )