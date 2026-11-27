# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArGeoFcal )

# External dependencies:
find_package ( GeoModelCore )

# Component(s) in the package:
atlas_add_library( LArGeoFcal
                   src/*.cxx
                   PUBLIC_HEADERS LArGeoFcal
                   LINK_LIBRARIES GeoModelCore::GeoModelKernel StoreGateLib
                   PRIVATE_LINK_LIBRARIES GeoModelUtilities GaudiKernel LArReadoutGeometry RDBAccessSvcLib  )

