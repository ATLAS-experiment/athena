# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1Monitoring )

# Component(s) in the package:
atlas_add_component( TrigT1Monitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaMonitoringKernelLib AthenaMonitoringLib L1TopoRDO StoreGateLib TrigConfData TrigConfInterfaces TrigConfL1Data TrigT1CaloMonitoringToolsLib TrigT1Interfaces TrigT1Result xAODTrigL1Calo )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
