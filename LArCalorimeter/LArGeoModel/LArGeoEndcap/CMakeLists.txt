#Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArGeoEndcap )

# External dependencies:
find_package ( GeoModelCore )

# Component(s) in the package:
atlas_add_library( LArGeoEndcap
                   src/*.cxx
                   PUBLIC_HEADERS LArGeoEndcap

                   LINK_LIBRARIES GeoModelCore::GeoModelKernel LArGeoFcal LArGeoHec StoreGateLib CaloDetDescrLib
                   PRIVATE_LINK_LIBRARIES CaloIdentifier GeoModelUtilities GaudiKernel LArGeoCode RDBAccessSvcLib )

