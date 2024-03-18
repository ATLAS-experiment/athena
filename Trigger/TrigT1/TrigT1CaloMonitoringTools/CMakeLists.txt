# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1CaloMonitoringTools )

# External dependencies:
find_package( ROOT COMPONENTS Hist )

# Component(s) in the package:
atlas_add_library( TrigT1CaloMonitoringToolsLib
                   src/*.cxx
                   PUBLIC_HEADERS TrigT1CaloMonitoringTools
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AsgTools AthenaMonitoringLib GaudiKernel
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers xAODEventInfo LWHists SGTools TrigConfInterfaces TrigConfL1Data TrigT1CaloEventLib TrigT1CaloUtilsLib TrigT1Interfaces xAODTrigL1Calo )

atlas_add_component( TrigT1CaloMonitoringTools
                     src/components/*.cxx
                     LINK_LIBRARIES TrigT1CaloMonitoringToolsLib )
