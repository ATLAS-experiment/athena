# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( L1CaloFEXByteStream )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat eformat_write )

atlas_add_library( L1CaloFEXByteStreamLib INTERFACE PUBLIC_HEADERS L1CaloFEXByteStream )

# Component(s) in the package:
atlas_add_component( L1CaloFEXByteStream
                     src/*.cxx
                     src/components/*.cxx
                     src/bytestreamDecoder/src/*.cxx
                     src/channelMappings/src/*.cxx
                     src/infraL1Calo/src/*.cxx
                     src/defsL1Calo/src/*.cxx
                     DEFINITIONS OFFLINE_DECODER
                     INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS} src/bytestreamDecoder src/channelMappings src/infraL1Calo src/defsL1Calo
                     LINK_LIBRARIES L1CaloFEXByteStreamLib TrigT1ResultByteStreamLib xAODTrigger TrigConfData xAODTrigL1Calo PathResolver AthenaMonitoringKernelLib ${TDAQ-COMMON_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )
