# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GlobalSimulation )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist)
find_package( GTest )
find_package( APTypes )

atlas_add_component (GlobalSimulation
   src/Egamma1/*.h src/Egamma1/*.cxx
   src/Egamma1BDT/*.h src/Egamma1BDT/*.cxx
   src/Egamma1BDT/Egamma1BDT/*.h src/Egamma1BDT/Egamma1BDT/*.cpp
   src/FEX_Unpacker/*.h src/FEX_Unpacker/*.cxx
   src/GlobalSimComponents/*.h src/GlobalSimComponents/*.cxx
   src/Hypothesis/*.h src/Hypothesis/*.cxx
   src/IO/*.h src/IO/*.cxx
   src/Lar_Preproc/*.h src/Lar_Preproc/*.cxx
   src/PU1/*.h src/PU1/*.cxx
   src/Utilities/*.h src/Utilities/*.cxx
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
atlas_install_runtime(share/*.txt share/*.json )
atlas_install_data( data/* share/*.xml )
atlas_install_joboptions( share/plugin_*.py )

atlas_add_test(testEgamma1BDT_tv
               SOURCES src/Egamma1BDT/Egamma1BDT/TestEgamma1/all_tests.cxx
	       src/Egamma1BDT/Egamma1BDT/*.cpp
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



