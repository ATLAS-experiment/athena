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
         PROPERTIES TIMEOUT 900 )

   endfunction()

   function( add_test_compare NAME DATA_TYPE NAME1 NAME2 )

      atlas_add_test( ${NAME}
         SCRIPT acmd.py diff-root --error-mode resilient -t analysis TestJobOutput-${NAME1}.hist.root TestJobOutput-${NAME2}.hist.root
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 900
         DEPENDS ${NAME1} ${NAME2})

   endfunction()

endif()



add_test_job( TestJobDataSequence data --for-compare --no-systematics )
add_test_job( TestJobDataConfig data --for-compare --block-config --no-systematics )
add_test_job( TestJobDataTextConfig data --for-compare --text-config ${CONFIG_PATH} --no-systematics )
add_test_job( TestJobDataFull data --block-config --no-systematics )
add_test_job( TestJobDataNominalOR data --block-config --no-systematics --only-nominal-or )
add_test_compare( TestJobDataCompare data TestJobDataSequence TestJobDataConfig )
add_test_compare( TestJobDataCompareConfig data TestJobDataConfig TestJobDataTextConfig )

add_test_job( TestJobFullSimSequence fullsim --for-compare )
add_test_job( TestJobFullSimConfig fullsim --for-compare --block-config )
add_test_job( TestJobFullSimTextConfig fullsim --for-compare --text-config ${CONFIG_PATH} )
add_test_job( TestJobFullSimFull fullsim --block-config )
add_test_job( TestJobFullSimNominalOR fullsim --block-config --only-nominal-or )
add_test_compare( TestJobFullSimCompare fullsim TestJobFullSimSequence TestJobFullSimConfig )
add_test_compare( TestJobFullSimCompareConfig fullsim TestJobFullSimConfig TestJobFullSimTextConfig )

add_test_job( TestJobFastSimSequence fastsim --for-compare )
add_test_job( TestJobFastSimConfig fastsim --for-compare --block-config )
add_test_job( TestJobFastSimTextConfig fastsim --for-compare --text-config ${CONFIG_PATH} )
add_test_job( TestJobFastSimFull fastsim --block-config )
add_test_job( TestJobFastSimNominalOR fastsim --block-config --only-nominal-or )
add_test_compare( TestJobFastSimCompare fastsim TestJobFastSimSequence TestJobFastSimConfig )
# add_test_compare( TestJobFastSimCompareConfig fastsim TestJobFastSimConfig TestJobFastSimTextConfig )

# FIX ME: There are some issues with the PHYSLITE tests that need to
# be investigated, but until that happens I run some tests only in
# AnalysisBase, and disable comparisons alltogether.

add_test_job( TestJobDataSequenceLite data --for-compare --physlite --no-systematics )
add_test_job( TestJobDataConfigLite data --for-compare --block-config --physlite --no-systematics )
add_test_job( TestJobDataTextConfigLite data --for-compare --text-config ${CONFIG_PATH} --physlite --no-systematics )
add_test_job( TestJobDataFullLite data --block-config --physlite --no-systematics )
add_test_job( TestJobDataNominalORLite data --block-config --only-nominal-or --physlite )
# add_test_compare( TestJobDataCompareLite data TestJobDataSequenceLite TestJobDataConfigLite )
# add_test_compare( TestJobDataCompareConfigLite data TestJobDataConfigLite TestJobDataTextConfigLite )


if( XAOD_STANDALONE )

   add_test_job( TestJobFullSimSequenceLite fullsim --for-compare --physlite )
   add_test_job( TestJobFullSimConfigLite fullsim --for-compare --block-config --physlite )
   add_test_job( TestJobFullSimTextConfigLite fullsim --for-compare --text-config ${CONFIG_PATH} --physlite )
   add_test_job( TestJobFullSimFullLite fullsim --block-config --physlite )
   add_test_job( TestJobFullSimNominalORLite fullsim --block-config --only-nominal-or --physlite )
   add_test_compare( TestJobFullSimCompareLite fullsim TestJobFullSimSequenceLite TestJobFullSimConfigLite )

   add_test_job( TestJobFastSimSequenceLite fastsim --for-compare --physlite )
   add_test_job( TestJobFastSimConfigLite fastsim --for-compare --block-config --physlite )
   add_test_job( TestJobFastSimTextConfigLite fastsim --for-compare --text-config ${CONFIG_PATH} --physlite )
   add_test_job( TestJobFastSimFullLite fastsim --block-config --physlite )
   add_test_job( TestJobFastSimNominalORLite fastsim --block-config --only-nominal-or --physlite )
   add_test_compare( TestJobFastSimCompareLite fastsim TestJobFastSimSequenceLite TestJobFastSimConfigLite )

   # this test is for testing that the algorithm monitors defined in EventLoop
   # don't break a job of reasonable complexity.  they are tested here instead of
   # in the EventLoop package, because we have a much more complex payload here.
   add_test_job( TestJobDataConfigMonitors data --for-compare --block-config --no-systematics --algorithm-timer --algorithm-memory )
   add_test_compare( TestJobDataCompareMonitor data TestJobDataConfig TestJobDataConfigMonitors )

endif()
