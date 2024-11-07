# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( AnalysisAlgorithmsConfig )

atlas_install_python_modules( python/*.py )

set( CONFIG_PATH "${CMAKE_CURRENT_LIST_DIR}/data/for_compare.yaml" )

if( XAOD_STANDALONE )

   atlas_install_scripts( scripts/*_eljob.py )

   function( add_test_job NAME DATA_TYPE )

      atlas_add_test( ${NAME}
         SCRIPT FullCPAlgorithmsTest_eljob.py --data-type ${DATA_TYPE} --direct-driver --submission-dir submitDir-${NAME} ${ARGN}
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 900 )

   endfunction()

   function( add_test_compare NAME DATA_TYPE NAME1 NAME2 )

      atlas_add_test( ${NAME}
         SCRIPT compareFlatTrees --require-same-branches analysis submitDir-${NAME1}/data-ANALYSIS/${DATA_TYPE}.root submitDir-${NAME2}/data-ANALYSIS/${DATA_TYPE}.root
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 900
         DEPENDS ${NAME1} ${NAME2})

   endfunction()

else()

   atlas_install_scripts( scripts/*_CA.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

   function( add_test_job NAME DATA_TYPE )

      atlas_add_test( ${NAME}
         SCRIPT FullCPAlgorithmsTest_CA.py --evtMax=500 --output-file TestJobOutput-${NAME}.hist.root --dump-config ${NAME}.pkl --data-type ${DATA_TYPE} ${ARGN}
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 1800 )

   endfunction()

   function( add_test_compare NAME DATA_TYPE NAME1 NAME2 )

      atlas_add_test( ${NAME}
         SCRIPT acmd.py diff-root --error-mode resilient --exact-branches -t analysis TestJobOutput-${NAME1}.hist.root TestJobOutput-${NAME2}.hist.root
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 1800
         DEPENDS ${NAME1} ${NAME2})

   endfunction()

endif()

atlas_add_test( ConfigTextCompareBuilder
   SCRIPT python/ConfigText_unitTest.py --text-config ${CONFIG_PATH} --compare-builder --check-order
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 30 )

atlas_add_test( ConfigTextCompareBlock
   SCRIPT python/ConfigText_unitTest.py --text-config ${CONFIG_PATH} --compare-block --check-order
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 30 )

add_test_job( TestJobDataConfig data --for-compare --no-systematics --run 2 )
add_test_job( TestJobDataTextConfig data --for-compare --text-config ${CONFIG_PATH} --no-systematics --run 2 )
add_test_job( TestJobDataFull data --no-systematics --run 2 )
add_test_job( TestJobDataFullR3 data --no-systematics --run 3 )
add_test_job( TestJobDataNominalOR data --no-systematics --only-nominal-or --run 2 )
add_test_compare( TestJobDataCompareConfig data TestJobDataConfig TestJobDataTextConfig )

add_test_job( TestJobFullSimConfig fullsim --for-compare --run 2 )
add_test_job( TestJobFullSimTextConfig fullsim --for-compare --text-config ${CONFIG_PATH} --run 2 )
add_test_job( TestJobFullSimFull fullsim --run 2 )
add_test_job( TestJobFullSimFullR3 fullsim --run 3 )
add_test_job( TestJobFullSimNominalOR fullsim --only-nominal-or --run 2 )
add_test_compare( TestJobFullSimCompareConfig fullsim TestJobFullSimConfig TestJobFullSimTextConfig )

add_test_job( TestJobFastSimConfig fastsim --for-compare --run 2 )
add_test_job( TestJobFastSimTextConfig fastsim --for-compare --text-config ${CONFIG_PATH} --run 2 )
add_test_job( TestJobFastSimFull fastsim --run 2 )
add_test_job( TestJobFastSimFullR3 fastsim --run 3 )
add_test_job( TestJobFastSimNominalOR fastsim --only-nominal-or --run 2 )
add_test_compare( TestJobFastSimCompareConfig fastsim TestJobFastSimConfig TestJobFastSimTextConfig )

add_test_job( TestJobDataConfigLite data --for-compare --physlite --no-systematics --run 2 )
add_test_job( TestJobDataTextConfigLite data --for-compare --text-config ${CONFIG_PATH} --physlite --no-systematics --run 2 )
add_test_job( TestJobDataFullLite data --physlite --no-systematics --run 2 )
add_test_job( TestJobDataFullLiteR3 data --physlite --no-systematics --run 3 )
add_test_job( TestJobDataNominalORLite data --only-nominal-or --physlite --run 2 )
add_test_compare( TestJobDataCompareConfigLite data TestJobDataConfigLite TestJobDataTextConfigLite )

add_test_job( TestJobFullSimConfigLite fullsim --for-compare --physlite --run 2 )
add_test_job( TestJobFullSimTextConfigLite fullsim --for-compare --text-config ${CONFIG_PATH} --physlite --run 2 )
add_test_job( TestJobFullSimFullLite fullsim --physlite --run 2 )
add_test_job( TestJobFullSimFullLiteR3 fullsim --physlite --run 3 )
add_test_job( TestJobFullSimNominalORLite fullsim --only-nominal-or --physlite --run 2 )
add_test_compare( TestJobFullSimCompareConfigLite fullsim TestJobFullSimConfigLite TestJobFullSimTextConfigLite )

add_test_job( TestJobFastSimConfigLite fastsim --for-compare --physlite --run 2 )
add_test_job( TestJobFastSimTextConfigLite fastsim --for-compare --text-config ${CONFIG_PATH} --physlite --run 2 )
add_test_job( TestJobFastSimFullLite fastsim --physlite --run 2 )
add_test_job( TestJobFastSimFullLiteR3 fastsim --physlite --run 3 )
add_test_job( TestJobFastSimNominalORLite fastsim --only-nominal-or --physlite --run 2 )
add_test_compare( TestJobFastSimCompareConfigLite fastsim TestJobFastSimConfigLite TestJobFastSimTextConfigLite )


if( XAOD_STANDALONE )

   # this test is for testing that the algorithm monitors defined in EventLoop
   # don't break a job of reasonable complexity.  they are tested here instead of
   # in the EventLoop package, because we have a much more complex payload here.
   add_test_job( TestJobDataConfigMonitors data --for-compare --no-systematics --algorithm-timer --algorithm-memory )
   add_test_compare( TestJobDataCompareMonitor data TestJobDataConfig TestJobDataConfigMonitors )

endif()
