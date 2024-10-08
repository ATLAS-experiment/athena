# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCalibStreamCnv )

# External dependencies:
find_package( tdaq-common COMPONENTS MuCalDecode )

# Component(s) in the package:
atlas_add_component( MuonCalibStreamCnv
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES}
                     AthenaBaseComps AthenaKernel GaudiKernel MdtCalibData MdtCalibInterfacesLib
                     MuonCablingData MuonCalibEvent MuonCalibStreamCnvSvcLib MuonIdHelpersLib
                     MuonPrepRawData MuonRDO MuonReadoutGeometry MuonTrigCoinData RPC_CondCablingLib
                     StoreGateLib xAODEventInfo )

# Install files from the package:
atlas_install_python_modules( python/*.py )
