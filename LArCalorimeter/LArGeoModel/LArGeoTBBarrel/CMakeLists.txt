# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArGeoTBBarrel )

# External dependencies:
find_package ( GeoModelCore )

# Component(s) in the package:
atlas_add_library( LArGeoTBBarrel
                   src/*.cxx
                   PUBLIC_HEADERS LArGeoTBBarrel
                   LINK_LIBRARIES GeoModelCore::GeoModelKernel StoreGateLib
                   PRIVATE_LINK_LIBRARIES GeoModelUtilities GaudiKernel LArGeoBarrel )

