# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DiTauDQA )

# Component(s) in the package:
atlas_add_component( DiTauDQA
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES xAODTau GaudiKernel AthenaMonitoringLib TauAnalysisToolsLib TrkValHistUtils AthContainers ) 

atlas_install_python_modules( python/*.py )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
