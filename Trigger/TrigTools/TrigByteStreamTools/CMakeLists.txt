################################################################################
# Package: TrigByteStreamTools
################################################################################

# Declare the package name:
atlas_subdir( TrigByteStreamTools )

# External dependencies:
#Needed as dependency by TDAQ
find_package( Boost )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( tdaq-common COMPONENTS CTPfragment eformat eformat_write DataReader DataWriter )

# Component(s) in the package:
atlas_add_dictionary( TrigByteStreamToolsDict
                      TrigByteStreamTools/TrigByteStreamToolsDict.h
                      TrigByteStreamTools/selection.xml
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${TDAQ-COMMON_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} ${Boost_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} )

atlas_add_executable( trigbs_orderedMerge
                      bin/trigbs_orderedMerge.cxx
                      INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                      LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} ${Boost_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( bin/*.py python/trigbs_modifyEvent.py python/trigbs_prescaleL1.py python/slimHLTBSFile.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( share/*.ref )

# Unit tests
set( trigbs_testFileRun2 "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data18_13TeV.00360026.physics_EnhancedBias.merge.RAW._lb0151._SFO-1._0001.1_80evt")
set( trigbs_testFileRun3 "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data24_13p6TeV.00475321.physics_Main.daq.RAW._lb0247._SFO-11._0006.data_150evt")

atlas_add_test( dumpHLTContentInBS_run3
                SCRIPT trigbs_dumpHLTContentInBS_run3.py -n 5 -s 71 --l1 --hlt --hltres --stag --sizes --sizeSummary --confKeys ${trigbs_testFileRun3}
                LOG_IGNORE_PATTERN "^----.*(bytes|B/ev)" )

atlas_add_test( dumpHLTNav
                SCRIPT trigbs_dumpHLTNav.py -n 3 -s 71 ${trigbs_testFileRun3}
                LOG_IGNORE_PATTERN "no dictionary for class" )

atlas_add_test( updateMetadata
                SCRIPT rm -f newMeta.*.data
                       && trigbs_updateBSMetadata.py -n 5 --copyFrom ${trigbs_testFileRun2} --outputName newMeta ${trigbs_testFileRun3}
                       && meta-reader -m full newMeta._0001.data
                LOG_IGNORE_PATTERN "file_guid" )

atlas_add_test( mergeBSfiles
                SCRIPT rm -rf mergedBS.* test_mergeBSfiles_input*
                       && mkdir test_mergeBSfiles_input1 test_mergeBSfiles_input2
                       && ln -s ${trigbs_testFileRun2} test_mergeBSfiles_input1/file1.data
                       && ln -s ${trigbs_testFileRun3} test_mergeBSfiles_input1/file2.data
                       && ln -s ${trigbs_testFileRun2} test_mergeBSfiles_input2/file1.data
                       && ln -s ${trigbs_testFileRun3} test_mergeBSfiles_input2/file2.data
                       && trigbs_mergeBSfiles.py -v -r -N 5 -n 10 mergedBS test_mergeBSfiles_input1 test_mergeBSfiles_input2
                       && meta-reader mergedBS.*
                LOG_IGNORE_PATTERN "file_guid" )
