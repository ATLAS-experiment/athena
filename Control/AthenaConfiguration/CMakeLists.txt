# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaConfiguration )

if( XAOD_STANDALONE )
    atlas_install_python_modules( python/AllConfigFlags.py
                                  python/AthConfigFlags.py
                                  python/AutoConfigFlags.py
                                  python/GeoModelConfigFlags.py
                                  python/Enums.py
                                  python/TestDefaults.py
                                  POST_BUILD_CMD ${ATLAS_FLAKE8} )
else()
    # External dependencies.
    find_package( future )

    # Install files from the package:
    atlas_install_python_modules( python/*.py python/iconfTool
                                  POST_BUILD_CMD ${ATLAS_FLAKE8} )
    
    atlas_install_scripts( share/confTool.py python/iconfTool/iconfTool share/CARunner.py
                           POST_BUILD_CMD ${ATLAS_FLAKE8} )
endif()

# common tests
atlas_add_test( AthConfigFlagsTest
                SCRIPT test/testAthConfigFlags.py
                POST_EXEC_SCRIPT nopost.sh )

# tests for analysis releases and Athena
if( NOT GENERATIONBASE AND NOT SIMULATIONBASE )
    atlas_add_test( AllConfigFlagsTest_AOD_MC
                    SCRIPT test/testAllConfigFlags_AOD_MC.py
                    POST_EXEC_SCRIPT nopost.sh )

    atlas_add_test( AllConfigFlagsTest_AOD_data
                    SCRIPT test/testAllConfigFlags_AOD_data.py
                    POST_EXEC_SCRIPT nopost.sh )
endif()

# tests for Athena-based releases
if( NOT XAOD_STANDALONE )    
    atlas_add_test( ComponentAccumulatorTest
                    SCRIPT test/testComponentAccumulator.py
                    POST_EXEC_SCRIPT nopost.sh )

    atlas_add_test( AtlasSemantics
                    SCRIPT test/testAtlasSemantics.py
                    POST_EXEC_SCRIPT nopost.sh )

    atlas_add_test( AccumulatorCache
                    SCRIPT test/testAccumulatorCache.py
                    POST_EXEC_SCRIPT nopost.sh )

    atlas_add_test( MainServicesConfig
                    SCRIPT python -m AthenaConfiguration.MainServicesConfig
                    POST_EXEC_SCRIPT nopost.sh )
endif()

# tests that should not run in analysis releases
if( NOT XAOD_ANALYSIS )
    atlas_add_test( AllConfigFlagsTest_EVNT
                    SCRIPT test/testAllConfigFlags_EVNT.py
                    POST_EXEC_SCRIPT nopost.sh )
endif()

# tests that should not run in analysis releases and in AthGeneration
if( NOT GENERATIONBASE AND NOT XAOD_ANALYSIS )
    atlas_add_test( FPEAndCoreDump
                    SCRIPT test/testFPEHandling.py )

    atlas_add_test( DetectorConfigFlags
                    SCRIPT test/testDetectorFlags.py
                    POST_EXEC_SCRIPT nopost.sh )

    atlas_add_test( AllConfigFlagsTest_HITS
                    SCRIPT test/testAllConfigFlags_HITS.py
                    POST_EXEC_SCRIPT nopost.sh )
endif()

# tests that should run only in Athena
if( NOT SIMULATIONBASE AND NOT GENERATIONBASE AND NOT XAOD_ANALYSIS )
    atlas_add_test( AllConfigFlagsTest_RDO
                    SCRIPT test/testAllConfigFlags_RDO.py
                    POST_EXEC_SCRIPT nopost.sh )

    atlas_add_test( AllConfigFlagsTest_AOD_Full
                    SCRIPT python -m AthenaConfiguration.AllConfigFlags
                    POST_EXEC_SCRIPT nopost.sh )

    atlas_add_test( AllConfigFlagsTest_TRIG
                    SCRIPT test/testAllConfigFlags_TRIG.py
                    POST_EXEC_SCRIPT noerror.sh )

    atlas_add_test( ComponentAccumulatorTestReco
                    SCRIPT test/testComponentAccumulatorReco.py
                    POST_EXEC_SCRIPT nopost.sh )

    atlas_add_test( OldFlags2NewFlagsTest
                    SCRIPT python -m AthenaConfiguration.OldFlags2NewFlags
                    POST_EXEC_SCRIPT nopost.sh )
endif()
