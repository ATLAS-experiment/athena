# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GlobalSimulation )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist)
find_package( GTest )

atlas_add_component (GlobalSimulation
   src/*.h src/*.cxx src/L1TopoAlgs/*.h src/L1TopoAlgs/*.cxx
   src/GlobalAlgs/*.h src/GlobalAlgs/*.cxx
   src/GlobalAlgs/BDT/*.h src/GlobalAlgs/Egamma1BDT/*.cpp 
   src/GlobalAlgs/Hypothesis/UCL/*.h src/GlobalAlgs/Hypothesis/UCL/*.cxx 
   src/IO/*.h src/IO/*.cxx
   src/components/*.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} GaudiKernel
   CaloDetDescrLib
   CaloIdentifier
   L1TopoSimulationLib
   TrigConfData
   L1TopoEvent
   L1TopoInterfaces
   AthenaBaseComps
   AthenaMonitoringKernelLib
   xAODTrigger
   CaloEvent
   PathResolver)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime(share/*.txt)

atlas_add_test(ap_fixed
               SOURCES src/NumericTypes/Tests/all_tests.cxx
	       src/NumericTypes/Tests/ap_fixedTest.cxx
               INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
               LINK_LIBRARIES ${GTEST_LIBRARIES} ${ROOT_LIBRARIES} CxxUtils
	       POST_EXEC_SCRIPT nopost.sh
   )

atlas_add_test(testEgamma1BDT_tv
               SOURCES src/GlobalAlgs/TestEgamma1/all_tests.cxx
	       src/GlobalAlgs/TestEgamma1/egamma1BDT_Test.cxx
	       src/GlobalAlgs/Egamma1BDT/*.cpp
               INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
               LINK_LIBRARIES ${GTEST_LIBRARIES} ${ROOT_LIBRARIES} CxxUtils
	       		      PathResolver
	       POST_EXEC_SCRIPT nopost.sh
   )

atlas_add_test( testGlobalL1TopoSim
   SCRIPT python/GlobalL1TopoSimulation_ctest.py --evtMax 3
   PRIVATE_WORKING_DIRECTORY
   POST_EXEC_SCRIPT noerror.sh )
   

atlas_add_test( testEfex
   SCRIPT python/run_Egamma1_LArStrip_Fex.py --evtMax 3
   PRIVATE_WORKING_DIRECTORY
   POST_EXEC_SCRIPT noerror.sh )


