# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigBphysMonitoring )

# Component(s) in the package:
atlas_add_component( TrigBphysMonitoring
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaMonitoringKernelLib AthenaMonitoringLib FourMomUtils GaudiKernel InDetConversionFinderToolsLib TrigDecisionToolLib TrigInDetToolInterfacesLib TrkVKalVrtFitterLib TrkVertexAnalysisUtilsLib xAODBPhysLib xAODMuon xAODTracking xAODTrigBphys )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
