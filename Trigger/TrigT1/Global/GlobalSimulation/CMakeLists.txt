# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GlobalSimulation )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist)
find_package( GTest )
find_package( APTypes )

atlas_add_library( GlobalSimulationLib
		PUBLIC_HEADERS GlobalSimulation
		LINK_LIBRARIES xAODCore )
atlas_add_dictionary( GlobalSimulationDict
		GlobalSimulation/GlobalSimulationDict.h
		GlobalSimulation/selection.xml
		LINK_LIBRARIES GlobalSimulationLib )


atlas_add_component (GlobalSimulation
   src/Egamma1/*.h src/Egamma1/*.cxx
   src/Egamma1BDT/*.h src/Egamma1BDT/*.cxx
   src/Egamma1BDT/Egamma1BDT/*.h src/Egamma1BDT/Egamma1BDT/*.cpp
   src/FEX_Unpacker/*.h src/FEX_Unpacker/*.cxx
   src/GlobalSimComponents/*.h src/GlobalSimComponents/*.cxx
   src/Hypothesis/*.h src/Hypothesis/*.cxx
   src/IO/*.h src/IO/*.cxx
   src/Jet1/*.h src/Jet1/*.cxx
   src/Lar_Preproc/*.h src/Lar_Preproc/*.cxx
   src/PU1/*.h src/PU1/*.cxx
   src/Utilities/*.h src/Utilities/*.cxx
   src/*.cxx
   src/components/*.cxx
   INCLUDE_DIRS ${APTYPES_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${APTYPES_LIBRARIES} ${ROOT_LIBRARIES} GaudiKernel
   CaloDetDescrLib
   CaloIdentifier
   L1TopoSimulationLib
   TrigConfData
   TrigGepPerfLib # For JET1(WTAConeJet), use TGP headers
   L1TopoEvent
   L1TopoInterfaces
   AthenaBaseComps
   AthenaMonitoringKernelLib
   xAODTrigger
   xAODEventInfo
   CaloEvent
   PathResolver)

remove_definitions(-DFLOATING_POINT_SIMULATION)
add_definitions(-DBITWISE_SIMULATION) # For JET1(WTAConeJet), need extra definition

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( data/* )
atlas_install_data( share/*.xml share/*.json )

atlas_install_joboptions( share/plugin_*.py )

atlas_add_test(testEgamma1BDT_tv
               SOURCES src/Egamma1BDT/Egamma1BDT/TestEgamma1/all_tests.cxx
	       src/Egamma1BDT/Egamma1BDT/*.cpp
               INCLUDE_DIRS ${APTYPES_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
               LINK_LIBRARIES ${APTYPES_LIBRARIES} ${GTEST_LIBRARIES} ${ROOT_LIBRARIES} CxxUtils
	       		      PathResolver
	       POST_EXEC_SCRIPT noerror.sh
   )


atlas_add_test( AllChains
        SCRIPT "l1calo-ath-mon --postInclude GlobalSimulation/plugin_local_cfg.py   --filesInput RAW_RUN3_DATA24 --evtMax 10 IOVDb.GlobalTag='CONDBR2-BLKPA-2024-05' -- Trigger.L1.doeFex=True -- Trigger.L1.dogFex=True"
        PROPERTIES TIMEOUT 600
        PRIVATE_WORKING_DIRECTORY
        POST_EXEC_SCRIPT noerror.sh )


# need to copy the TestSpec into a file that can then be read by the test, to avoid needing a dictionary for TestSpec
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/test/TestSpec.h" TESTSPEC_SOURCE)
configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/test/TestSpec_source.h.in"
		"${CMAKE_CURRENT_BINARY_DIR}/TestSpec_source.h"
		@ONLY
)
atlas_add_test( Object_test
		SOURCES test/Object_test.cxx
		INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}
		LINK_LIBRARIES ${GTEST_LIBRARIES} GlobalSimulationLib
		POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( JsonConfig
        SCRIPT "l1calo-ath-mon --evtMax 10 --postInclude GlobalSimulation/plugin_GlobalSimFromJson.py --filesInput RAW_RUN3_DATA24 -- Trigger.L1.doeFex=True Trigger.L1.dogFex=True IOVDb.GlobalTag='CONDBR2-BLKPA-2024-05'"
        PROPERTIES TIMEOUT 600
        PRIVATE_WORKING_DIRECTORY
        POST_EXEC_SCRIPT noerror.sh )

atlas_add_alias( l1global-minisim l1calo-ath-mon GlobalSimulation/plugin_miniSim.py )
