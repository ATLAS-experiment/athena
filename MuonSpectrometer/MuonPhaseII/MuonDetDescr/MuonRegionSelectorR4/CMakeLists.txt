# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonRegionSelectorR4 )


# Component(s) in the package:
atlas_add_component( MuonRegionSelectorR4
                     src/*.cxx
                     src/components/*.cxx                     
                     LINK_LIBRARIES AthenaBaseComps IRegionSelector MuonReadoutGeometryR4
                                    RegSelLUT GaudiKernel Identifier StoreGateLib MuonCablingData
                                    MuonTGC_CablingLib MuonNSWCommonDecode FourMomUtils)
