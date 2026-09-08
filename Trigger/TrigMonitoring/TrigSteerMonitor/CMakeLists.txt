

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigSteerMonitor )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist )
find_package( tdaq-common COMPONENTS eformat )

# Interface library
atlas_add_library( TrigSteerMonitorLib
                   INTERFACE
                   PUBLIC_HEADERS TrigSteerMonitor
                   LINK_LIBRARIES GaudiKernel )

# Component(s) in the package:
atlas_add_component( TrigSteerMonitor
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps AthenaInterprocess AthenaKernel AthenaMonitoringKernelLib ByteStreamData CxxUtils GaudiKernel Gaudi::GaudiUtilsLib LumiBlockData MagFieldConditions StoreGateLib TrigCompositeUtilsLib TrigConfData TrigCostMonitorLib TrigSteerMonitorLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
