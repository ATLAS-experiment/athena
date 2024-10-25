# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1CaloMonitoringTools )

# Component(s) in the package:
atlas_add_library( TrigT1CaloMonitoringToolsLib
                   TrigT1CaloMonitoringTools/*.h
                   INTERFACE
                   PUBLIC_HEADERS TrigT1CaloMonitoringTools
                   LINK_LIBRARIES AsgTools GaudiKernel )

atlas_add_component( TrigT1CaloMonitoringTools
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AsgTools AthContainers GaudiKernel SGTools StoreGateLib TrigT1CaloEventLib TrigT1CaloMonitoringToolsLib TrigT1Interfaces xAODEventInfo xAODTrigL1Calo )
