# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArGeoBarrel )

# External dependencies:
find_package( GeoModelCore )
find_package( GeoModelIO )
# Component(s) in the package:
atlas_add_library( LArGeoBarrel
                   src/*.cxx
                   PUBLIC_HEADERS LArGeoBarrel
                   LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead LArGeoCode StoreGateLib
                   PRIVATE_LINK_LIBRARIES GeoModelUtilities GaudiKernel LArReadoutGeometry RDBAccessSvcLib CrackRegionGeoModel )

