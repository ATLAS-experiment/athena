# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( L0MuonMDTTools )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore )

# Library in the package
atlas_add_library( L0MuonMDTToolsLib
                   src/*.cxx
                   PUBLIC_HEADERS L0MuonMDTTools
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps xAODMuonPrepData AthenaKernel GaudiKernel Identifier MuonDigitContainer MuonIdHelpersLib MuonRDO MuonReadoutGeometryR4  MuonSimData MuonSimEvent MuonPrepRawData MagFieldConditions MagFieldElements StoreGateLib
                 )

# Component(s) in the package:
atlas_add_component( L0MuonMDTTools
                     src/components/*.cxx
                     LINK_LIBRARIES L0MuonMDTToolsLib )


