# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArGeoHec )

# External dependencies:
find_package ( GeoModelCore )

# Component(s) in the package:
atlas_add_library( LArGeoHec
                   src/*.cxx
                   PUBLIC_HEADERS LArGeoHec
                   LINK_LIBRARIES StoreGateLib
                   PRIVATE_LINK_LIBRARIES GeoModelCore::GeoModelKernel AthenaKernel GeoModelUtilities 
                                          GaudiKernel RDBAccessSvcLib )

