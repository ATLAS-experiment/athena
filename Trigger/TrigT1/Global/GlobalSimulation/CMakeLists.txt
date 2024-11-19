# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GlobalSimulation )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist)

atlas_add_component (GlobalSimulation
   src/*.h src/*.cxx src/L1TopoAlgs/*.h src/L1TopoAlgs/*.cxx
   src/GlobalAlgs/*.h src/GlobalAlgs/*.cxx 
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
   CaloEvent)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime(share/*.txt)

atlas_add_test( testGlobalL1TopoSim
   SCRIPT python/GlobalL1TopoSimulation_ctest.py --evtMax 3
   PRIVATE_WORKING_DIRECTORY
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( testEfex
   SCRIPT python/run_Egamma1_LArStrip_Fex.py --evtMax 10
   PRIVATE_WORKING_DIRECTORY
   POST_EXEC_SCRIPT noerror.sh )


