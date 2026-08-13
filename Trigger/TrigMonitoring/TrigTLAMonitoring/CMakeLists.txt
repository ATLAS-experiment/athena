# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigTLAMonitoring )

# Component(s) in the package:
atlas_add_component( TrigTLAMonitoring
                     src/TrigTLAMonitorAlgorithm.cxx src/components/TrigTLAMonitoring_entries.cxx
                     LINK_LIBRARIES AthenaMonitoringKernelLib AthenaMonitoringLib TrigDecisionToolLib xAODEgamma xAODJet xAODMuon xAODTracking xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
