# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
                                    GeoPrimitives Identifier MuonAlignmentData MuonCondData MuonCondInterface MuonIdHelpersLib 
                                    MuonReadoutGeometry MdtCalibData StoreGateLib MuonCablingData MuonReadoutGeometryR4 )

# Install files from the package:
atlas_install_python_modules( python/*.py )

atlas_add_test( MdtCabling_Run2Data
                  SCRIPT python -m MuonCondTest.MdtCablingTester
                  PRIVATE_WORKING_DIRECTORY
                  PROPERTIES TIMEOUT 600
                  POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MdtCabling_Run2MC
                SCRIPT python -m MuonCondTest.MdtCablingTester -i /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/UnitTestInput/Run2MC.ESD.pool.root
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MdtCabling_Run3MC
                SCRIPT python -m MuonCondTest.MdtCablingTester -i /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/UnitTestInput/Run3MC.ESD.pool.root
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( RpcCabling_Run3MC
                SCRIPT python -m MuonNRPC_Cabling.createCablingJSON && python -m MuonCondTest.RpcCablingTester --cablingMap CablingFile.json
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)


atlas_add_test( RpcCabling_Run4MC
                SCRIPT python -m MuonCondDump.dumpR4ToyCablings --setupRun4 && python -m MuonCondTest.RpcCablingTesterR4  --setupRun4 --JSONFile RpcCabling.json
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( RpcCabling_Run4MC_COOL
                SCRIPT python -m MuonCondTest.RpcCablingTesterR4  --setupRun4  
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( NSWCondTest
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testNSWCondAlg.sh 
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( NSWPassivTest
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testNSWPassivAlg.sh 
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( NSWDcsTest
                PRIVATE_WORKING_DIRECTORY
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testNSWDcsAlg.sh 
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( TgcCondTest
                SCRIPT python -m MuonCondDump.dumpTgcDigiDeadChambers  &&  python -m MuonCondTest.TgcCondTester
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( TgcDigiThresholdTest
                SCRIPT python -m MuonCondDump.dumpTgcDigiThreshold  &&  python -m MuonCondTest.TgcDigiThresholdTester
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TgcDigiJitterTest
                SCRIPT python -m MuonCondDump.dumpTgcDigiJitter  &&  python -m MuonCondTest.TgcDigiJitterTester
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)


atlas_add_test( MdtCalibDbR4Test
                SCRIPT python -m MuonCondTest.MdtCalibDbTesterR4 --setupRun4
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MdtTwinCablingTest
                SCRIPT python -m MuonCondDump.dumpTwinCabling --setupRun4 --cablingMap twinCabling.json  --chamberToTwin all && python -m MuonCondTest.MdtTwinTester --setupRun4 --cablingJSON twinCabling.json
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT nopost.sh)
