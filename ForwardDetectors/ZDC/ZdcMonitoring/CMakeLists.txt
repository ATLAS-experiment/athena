# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZdcMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( ZdcMonitoring
    src/*.cxx src/components/*.cxx
    INCLUDE_DIRS
        ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES
        ${ROOT_LIBRARIES}
        AthenaMonitoringLib
        AthenaMonitoringKernelLib
        StoreGateLib
        xAODEventInfo
        xAODForward
        xAODHIEvent
        ZdcAnalysisLib
        ZdcUtilsLib
)

# Install files from the package:
atlas_install_python_modules( python/*.py )
