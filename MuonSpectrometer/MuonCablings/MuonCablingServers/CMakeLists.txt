# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCablingServers )

# Component(s) in the package:
atlas_add_library( MuonCablingServersLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonCablingServers
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel TGCcablingInterfaceLib StoreGateLib
                   PRIVATE_LINK_LIBRARIES EventInfoMgtLib )

atlas_add_component( MuonCablingServers
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel TGCcablingInterfaceLib StoreGateLib EventInfoMgtLib MuonCablingServersLib )

