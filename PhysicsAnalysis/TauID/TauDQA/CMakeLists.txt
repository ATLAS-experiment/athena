# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TauDQA )

# Component(s) in the package:
atlas_add_component( TauDQA
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthenaMonitoringLib TauAnalysisToolsLib TrkValHistUtils xAODTau ) 

atlas_install_python_modules( python/*.py )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
