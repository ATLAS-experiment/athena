# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCondTest )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( Boost )
find_package( CORAL )

# Component(s) in the package:
atlas_add_component( MuonCondTest
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${CORAL_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel CoralUtilitiesLib GaudiKernel 
                                    GeoPrimitives Identifier MuonAlignmentData MuonCondData MuonCondInterface MuonCondSvcLib MuonIdHelpersLib 
                                    MuonReadoutGeometry StoreGateLib MuonCablingData )

# Install files from the package:
atlas_install_python_modules( python/*.py )

atlas_add_test( MdtCabling_Run2Data
                  SCRIPT python -m MuonCondTest.MdtCablingTester
                  PROPERTIES TIMEOUT 600
                  POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MdtCabling_Run2MC
                  SCRIPT python -m MuonCondTest.MdtCablingTester -i /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/UnitTestInput/Run2MC.ESD.pool.root
                  PROPERTIES TIMEOUT 600
                  POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MdtCabling_Run3MC
                SCRIPT python -m MuonCondTest.MdtCablingTester -i /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/UnitTestInput/Run3MC.ESD.pool.root
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( RpcCabling_Run3MC
                SCRIPT python -m MuonNRPC_Cabling.createCablingJSON && python -m MuonCondTest.RpcCablingTester
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( NSWCondTest
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testNSWCondAlg.sh 
                PROPERTIES TIMEOUT 900
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( NSWPassivTest
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testNSWPassivAlg.sh 
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( NSWDcsTest
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testNSWDcsAlg.sh 
                PROPERTIES TIMEOUT 900
                POST_EXEC_SCRIPT nopost.sh)


atlas_add_test( TgcCondTest
                SCRIPT python -m MuonCondDump.dumpTgcDigiDeadChambers  &&  python -m MuonCondTest.TgcCondTester
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( TgcDigiThresholdTest
                SCRIPT python -m MuonCondDump.dumpTgcDigiThreshold  &&  python -m MuonCondTest.TgcDigiThresholdTester
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TgcDigiJitterTest
                SCRIPT python -m MuonCondDump.dumpTgcDigiJitter  &&  python -m MuonCondTest.TgcDigiJitterTester
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)
