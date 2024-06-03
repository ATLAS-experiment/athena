# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( L1TopoOnlineMonitoring )

# Components in the package:
atlas_add_component( L1TopoOnlineMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaMonitoringKernelLib AthenaMonitoringLib
                     DecisionHandlingLib GaudiKernel L1TopoRDO StoreGateLib TrigCompositeUtilsLib
                     TrigConfData TrigT1Interfaces TrigT1Result xAODTrigL1Calo xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py )
