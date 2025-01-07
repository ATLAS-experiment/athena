# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCalibStreamCnvSvc )

# External dependencies:
find_package( COOL COMPONENTS CoolKernel )
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( tdaq-common COMPONENTS MuCalDecode EventStorage )

# Component(s) in the package:
atlas_add_library( MuonCalibStreamCnvSvcLib
                   src/*.cxx MuonCalibStreamCnvSvc/*.h
                   PUBLIC_HEADERS MuonCalibStreamCnvSvc
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS} ${COOL_INCLUDE_DIRS} ${TDAQ-COMMON_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} ${CORAL_LIBRARIES} ${COOL_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} 
                   AthenaBaseComps AthenaKernel StoreGateLib GaudiKernel
                   PRIVATE_LINK_LIBRARIES CoraCool SGTools xAODEventInfo )

atlas_add_component( MuonCalibStreamCnvSvc
                     src/components/*.cxx
                     LINK_LIBRARIES MuonCalibStreamCnvSvcLib )

# Install files from the package:
atlas_install_python_modules( python/*.py )

