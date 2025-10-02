# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCondTest )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( CORAL )

# Component(s) in the package:
atlas_add_component( MuonCondTest
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel CoralUtilitiesLib GaudiKernel 
                                    GeoPrimitives Identifier MuonAlignmentData MuonCondData MuonCondInterface MuonIdHelpersLib 
                                    MuonReadoutGeometry MdtCalibData StoreGateLib MuonCablingData MuonReadoutGeometryR4 )

# Install files from the package:
atlas_install_python_modules( python/*.py )

atlas_add_test( MdtCabling_Run2Data
                  SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testMdtCabling.sh RAW_RUN2_DATA18 RUN2_BEST_KNOWLEDGE
                  PRIVATE_WORKING_DIRECTORY
                  PROPERTIES TIMEOUT 600
                  POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MdtCabling_Run2MC
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testMdtCabling.sh ESD_RUN2_MC 
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MdtCabling_Run3MC
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testMdtCabling.sh ESD_RUN3_MC
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MdtCabling_Run3Data
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testMdtCabling.sh RAW_RUN3_DATA24 RUN3
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( RpcCabling_Run3MC
                SCRIPT python -m MuonNRPC_Cabling.createCablingJSON && python -m MuonCondTest.RpcCablingTester --cablingMap CablingFile.json
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MmCabling_Run3Data
                SCRIPT python -m MuonNSW_Cabling.zebraShift && python -m MuonCondTest.MicroMegaCablingTester
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)


atlas_add_test( NSWCondTest_MC
                SCRIPT python -m MuonCondTest.NswCondAlgTest --isMC 
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1000
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( NSWCondTest_Data
                SCRIPT python -m MuonCondTest.NswCondAlgTest
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1000
                POST_EXEC_SCRIPT nopost.sh)
                
atlas_add_test( NSWPassivTest
                SCRIPT python -m MuonCondTest.NswPassivationAlgTest 
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)
atlas_add_test( NSWDcsTest
                PRIVATE_WORKING_DIRECTORY
                SCRIPT python -m MuonCondTest.NswDcsAlgTest 
                PROPERTIES TIMEOUT 900
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


##
###
###         RUN-4 conditions tests
###
##
atlas_add_test( RpcCabling_Run4MC_COOL
                SCRIPT python -m MuonCondTest.RpcCablingTesterR4  --defaultGeoFile RUN4  
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( RpcCabling_Run4MC
                SCRIPT python -m MuonCondDump.dumpR4ToyCablings --defaultGeoFile RUN4 && python -m MuonCondTest.RpcCablingTesterR4  --defaultGeoFile RUN4 --JSONFile RpcCabling.json
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT nopost.sh)


atlas_add_test( MdtCalibDbR4Test
                SCRIPT python -m MuonCondTest.MdtCalibDbTesterR4 --defaultGeoFile RUN4
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MdtTwinCablingTest
                SCRIPT python -m MuonCondDump.dumpTwinCabling --defaultGeoFile RUN4 --cablingMap twinCabling.json  --chamberToTwin all && python -m MuonCondTest.MdtTwinTester --defaultGeoFile RUN4 --cablingJSON twinCabling.json
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT nopost.sh)
