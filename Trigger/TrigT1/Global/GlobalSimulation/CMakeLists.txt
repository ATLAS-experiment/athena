# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GlobalSimulation )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist)

atlas_add_component (GlobalSimulation
   src/*.h src/*.cxx src/L1TopoAlgs/*.h src/L1TopoAlgs/*.cxx
   src/IO/*.h src/IO/*.cxx
   src/components/*.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} GaudiKernel
   L1TopoSimulationLib
   TrigConfData
   L1TopoEvent
   L1TopoInterfaces
   AthenaBaseComps
   AthenaMonitoringKernelLib
   L1TopoAlgorithms) # L1TopoAlgorithms added temporarily during development


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime(share/*.txt)
