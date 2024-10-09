# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonMM_CnvTools )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat )

atlas_add_library( MuonMM_CnvToolsLib
                   MuonMM_CnvTools/*.h
                   INTERFACE
                   PUBLIC_HEADERS MuonMM_CnvTools
                   LINK_LIBRARIES ByteStreamData GaudiKernel MuonRDO xAODMuonRDO )

# Component(s) in the package:
atlas_add_component( MuonMM_CnvTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} ByteStreamCnvSvcBaseLib ByteStreamData GaudiKernel AthenaBaseComps StoreGateLib Identifier MuonCondData MuonReadoutGeometry MuonDigitContainer MuonIdHelpersLib MuonNSWCommonDecode MuonRDO MuonPrepRawData MMClusterizationLib NSWCalibToolsLib MuonCnvToolInterfacesLib MuonMM_CnvToolsLib MuonCablingData TrkEventPrimitives xAODMuonRDO xAODMuonPrepData)
