# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GeoModelEnvelopes )

# External dependencies:
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_component( GeoModelEnvelopes
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  GeoModelCore::GeoModelKernel AthenaKernel GaudiKernel GeoModelInterfaces 
                                     GeoModelUtilities SGTools StoreGateLib )
