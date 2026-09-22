# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ByteStreamSamplingSvc )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat webdaq webdaq-noroot)
find_package( nlohmann_json )

# Component(s) in the package:
atlas_add_component( ByteStreamSamplingSvc
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS} ${NLOHMANN_JSON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps ByteStreamData CxxUtils GaudiKernel StoreGateLib PersistentDataModel ByteStreamCnvSvcBaseLib xAODEventInfo)

# Install files from the package:
atlas_install_python_modules( python/*.py )
