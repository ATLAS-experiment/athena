# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DiTauDQA )

find_package( ROOT COMPONENTS Core RIO Tree Physics )

# Component(s) in the package:
atlas_add_component( DiTauDQA
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} xAODTau GaudiKernel AthenaMonitoringLib TauAnalysisToolsLib TrkValHistUtils AthContainers ) 

atlas_install_python_modules( python/*.py )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
