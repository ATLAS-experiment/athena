# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ITkStripsByteStreamCnv )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat )

atlas_add_library( ITkStripsByteStreamCnvLib
                   ITkStripsByteStreamCnv/*.h
                   INTERFACE
                   PUBLIC_HEADERS ITkStripsByteStreamCnv
                   LINK_LIBRARIES GaudiKernel InDetRawData ByteStreamCnvSvcBaseLib ByteStreamData InDetByteStreamErrors )

# Component(s) in the package:
atlas_add_component( ITkStripsByteStreamCnv
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps ByteStreamCnvSvcBaseLib ByteStreamData EventContainers GaudiKernel IRegionSelector Identifier InDetIdentifier InDetRawData InDetReadoutGeometry ITkStripCablingLib SCT_ConditionsData SCT_ConditionsToolsLib ITkStripsByteStreamCnvLib SCT_ReadoutGeometry StoreGateLib TrigSteeringEvent xAODEventInfo )

# tests to be added

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
