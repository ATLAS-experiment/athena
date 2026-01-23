# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GlobalSimulation )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist)
find_package( GTest )
find_package( APTypes )

atlas_add_component (GlobalSimulation
   src/*.h src/*.cxx  
   # src/GlobalAlgs/*.h src/GlobalAlgs/*.cxx FIXME
   src/Utilities/*.cxx
   src/GlobalSimComponents/*.h src/GlobalSimComponents/*.cxx
   src/FirstChain/BDT/*.h src/FirstChain/Egamma1BDT/*.cpp
   src/FirstChain/*.h src/FirstChain/*.cxx 
   src/IO/*.h src/IO/*.cxx
   src/components/*.cxx
   INCLUDE_DIRS ${APTYPES_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${APTYPES_LIBRARIES} ${ROOT_LIBRARIES} GaudiKernel
   CaloDetDescrLib
   CaloIdentifier
   L1TopoSimulationLib
   TrigConfData
   L1TopoEvent
   L1TopoInterfaces
   AthenaBaseComps
   AthenaMonitoringKernelLib
   xAODTrigger
   xAODEventInfo
   CaloEvent
   PathResolver)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime(share/*.txt share/*.json)
atlas_install_data( data/* )

atlas_add_test(ap_fixed
               SOURCES src/NumericTypes/Tests/all_tests.cxx
	       src/NumericTypes/Tests/ap_fixedTest.cxx
               INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
               LINK_LIBRARIES ${GTEST_LIBRARIES} ${ROOT_LIBRARIES} CxxUtils
	       POST_EXEC_SCRIPT nopost.sh
   )

atlas_add_test(testEgamma1BDT_tv
               SOURCES src/FirstChain/Egamma1BDT/TestEgamma1/all_tests.cxx
	       src/FirstChain/Egamma1BDT/*.cpp
               INCLUDE_DIRS ${APTYPES_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
               LINK_LIBRARIES ${APTYPES_LIBRARIES} ${GTEST_LIBRARIES} ${ROOT_LIBRARIES} CxxUtils
	       		      PathResolver
	       POST_EXEC_SCRIPT nopost.sh
   )


atlas_add_test( testEfex
   SCRIPT python/run_Egamma1_LArStrip_Fex.py --evtMax 3
   PRIVATE_WORKING_DIRECTORY
   POST_EXEC_SCRIPT noerror.sh )


atlas_add_test(test_eEmMult
   SCRIPT python/run_hypo_mult_ctest.py --evtMax 1
   PRIVATE_WORKING_DIRECTORY
   POST_EXEC_SCRIPT noerror.sh )


