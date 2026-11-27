# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ALFA_GeoModel )

# External dependencies:
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_component( ALFA_GeoModel
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GeoModelCore::GeoModelKernel AthenaKernel AthenaPoolUtilities 
                                    GeoModelInterfaces GeoModelUtilities ALFA_Geometry StoreGateLib 
                                    GaudiKernel RDBAccessSvcLib GeoPrimitives )

