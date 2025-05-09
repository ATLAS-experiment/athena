# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( AnalysisAlgorithmsConfig )

atlas_install_python_modules( python/*.py )

set( CONFIG_PATH "${CMAKE_CURRENT_LIST_DIR}/data/for_compare.yaml" )

if( XAOD_STANDALONE )

   atlas_install_scripts( share/*_eljob.py )

   # FIX ME: Ideally we'd pass this into all test jobs, but there seems to be an
   # issue when passing this into data jobs.
   set ( USE_FLAGS --use-flags)

   function( add_test_job NAME DATA_TYPE )

      atlas_add_test( ${NAME}
         SCRIPT FullCPAlgorithmsTest_eljob.py --data-type ${DATA_TYPE} --direct-driver --submission-dir submitDir-${NAME} ${ARGN}
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 1500 )

   endfunction()

   function( add_test_compare NAME DATA_TYPE NAME1 NAME2 )

      atlas_add_test( ${NAME}
         SCRIPT compareFlatTrees --require-same-branches analysis submitDir-${NAME1}/data-ANALYSIS/${DATA_TYPE}.root submitDir-${NAME2}/data-ANALYSIS/${DATA_TYPE}.root
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 900
         DEPENDS ${NAME1} ${NAME2})

   endfunction()

else()

   atlas_install_joboptions( share/*_jobOptions.py )

   atlas_install_scripts( share/*_CA.py )

   set ( USE_FLAGS )

   function( add_test_job NAME DATA_TYPE )

      atlas_add_test( ${NAME}
         SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py --evtMax=500 - --force-output TestJobOutput-${NAME}.hist.root --data-type ${DATA_TYPE} ${ARGN}
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 1500 )

   endfunction()

   function( add_test_job_CA NAME DATA_TYPE )

      atlas_add_test( ${NAME}_CA
         SCRIPT FullCPAlgorithmsTest_CA.py --evtMax=500 --force-output TestJobOutput-${NAME}-CA.hist.root --data-type ${DATA_TYPE} ${ARGN}
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

   add_test_job_CA( TestJobDataConfig   data --for-compare --block-config --no-systematics )
   add_test_job_CA( TestJobFullSimConfig   mc --for-compare --block-config )
   add_test_job_CA( TestJobFastSimConfig   afii --for-compare --block-config )

endif()



add_test_job( TestJobDataSequence data --for-compare --no-systematics )
add_test_job( TestJobDataConfig   data --for-compare --block-config --no-systematics )
add_test_job( TestJobDataTextConfig data --for-compare --text-config ${CONFIG_PATH} --no-systematics )
add_test_job( TestJobDataFull     data --block-config --no-systematics )
add_test_job( TestJobDataNominalOR   data --block-config --no-systematics --only-nominal-or )
add_test_compare( TestJobDataCompare data TestJobDataSequence TestJobDataConfig )
add_test_compare( TestJobDataCompareConfig data TestJobDataConfig TestJobDataTextConfig )

add_test_job( TestJobFullSimSequence mc ${USE_FLAGS} --for-compare )
add_test_job( TestJobFullSimConfig   mc ${USE_FLAGS} --for-compare --block-config )
add_test_job( TestJobFullSimTextConfig  mc ${USE_FLAGS} --for-compare --text-config ${CONFIG_PATH} )
add_test_job( TestJobFullSimFull     mc ${USE_FLAGS} --block-config )
add_test_job( TestJobFullSimNominalOR   mc ${USE_FLAGS} --block-config --only-nominal-or )
add_test_compare( TestJobFullSimCompare mc TestJobFullSimSequence TestJobFullSimConfig )
add_test_compare( TestJobFullSimCompareConfig mc TestJobFullSimConfig TestJobFullSimTextConfig )

add_test_job( TestJobFastSimSequence afii ${USE_FLAGS} --for-compare )
add_test_job( TestJobFastSimConfig   afii ${USE_FLAGS} --for-compare --block-config )
add_test_job( TestJobFastSimTextConfig   afii ${USE_FLAGS} --for-compare --text-config ${CONFIG_PATH} )
add_test_job( TestJobFastSimFull     afii ${USE_FLAGS} --block-config )
add_test_job( TestJobFastSimNominalOR   afii ${USE_FLAGS} --block-config --only-nominal-or )
add_test_compare( TestJobFastSimCompare afii TestJobFastSimSequence TestJobFastSimConfig )

# FIX ME: the text configuration currently does not create weight_pileup_%SYS%
# for FastSim.  Not quite sure why, but the practical solution for now is to
# disable this comparison.
#add_test_compare( TestJobFastSimCompareConfig afii TestJobFastSimConfig TestJobFastSimTextConfig )




# FIX ME: There are some issues with the PHYSLITE tests that need to
# be investigated, but until that happens I run some tests only in
# AnalysisBase, and disable comparisons alltogether.

add_test_job( TestJobDataSequenceLite data --for-compare --physlite --no-physlite-broken --no-systematics )
add_test_job( TestJobDataConfigLite   data --for-compare --block-config --physlite --no-physlite-broken --no-systematics )
add_test_job( TestJobDataTextConfigLite data --for-compare --text-config ${CONFIG_PATH} --physlite --no-physlite-broken --no-systematics )
add_test_job( TestJobDataFullLite     data --block-config --physlite --no-physlite-broken --no-systematics )
add_test_job( TestJobDataNominalORLite    data --block-config --only-nominal-or --physlite --no-physlite-broken )
#add_test_compare( TestJobDataCompareLite data TestJobDataSequenceLite TestJobDataConfigLite )
#add_test_compare( TestJobDataCompareConfigLite data TestJobDataConfigLite TestJobDataTextConfigLite )

if( XAOD_STANDALONE )

add_test_job( TestJobFullSimSequenceLite mc ${USE_FLAGS} --for-compare --physlite --no-physlite-broken )
add_test_job( TestJobFullSimConfigLite   mc ${USE_FLAGS} --for-compare --block-config --physlite --no-physlite-broken )
add_test_job( TestJobFullSimTextConfigLite mc ${USE_FLAGS} --for-compare --text-config ${CONFIG_PATH} --physlite --no-physlite-broken )
add_test_job( TestJobFullSimFullLite     mc ${USE_FLAGS} --block-config --physlite --no-physlite-broken )
add_test_job( TestJobFullSimNominalORLite    mc ${USE_FLAGS} --block-config --only-nominal-or --physlite --no-physlite-broken )
add_test_compare( TestJobFullSimCompareLite mc TestJobFullSimSequenceLite TestJobFullSimConfigLite )

add_test_job( TestJobFastSimSequenceLite afii ${USE_FLAGS} --for-compare --physlite --no-physlite-broken )
add_test_job( TestJobFastSimConfigLite   afii ${USE_FLAGS} --for-compare --block-config --physlite --no-physlite-broken )
add_test_job( TestJobFastSimTextConfigLite afii ${USE_FLAGS} --for-compare --text-config ${CONFIG_PATH} --physlite --no-physlite-broken )
add_test_job( TestJobFastSimFullLite     afii ${USE_FLAGS} --block-config --physlite --no-physlite-broken )
add_test_job( TestJobFastSimNominalORLite    afii ${USE_FLAGS} --block-config --only-nominal-or --physlite --no-physlite-broken )
add_test_compare( TestJobFastSimCompareLite afii TestJobFastSimSequenceLite TestJobFastSimConfigLite )

# this test is for testing that the algorithm monitors defined in EventLoop
# don't break a job of reasonable complexity.  they are tested here instead of
# in the EventLoop package, because we have a much more complex payload here.
add_test_job( TestJobDataConfigMonitors   data --for-compare --block-config --no-systematics --algorithm-timer --algorithm-memory )
add_test_compare( TestJobDataCompareMonitor data TestJobDataConfig TestJobDataConfigMonitors )

endif()
