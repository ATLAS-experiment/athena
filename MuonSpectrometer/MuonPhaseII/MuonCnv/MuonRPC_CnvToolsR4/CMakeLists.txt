# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonRPC_CnvToolsR4 )

# External dependencies:
find_package( tdaq-common )


# Component(s) in the package:
atlas_add_component( MuonRPC_CnvToolsR4
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} MuonRPC_CnvToolsLib GaudiKernel AthenaBaseComps AthenaKernel StoreGateLib 
                                    ByteStreamCnvSvcBaseLib MuonCondData MuonCablingData MuonReadoutGeometryR4 MuonIdHelpersLib MuonRDO xAODMuonRDO MuonPrepRawData  
                                    MuonCnvToolInterfacesLib xAODMuonPrepData )
