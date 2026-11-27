# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArGeoCode )

# External dependencies:
find_package ( GeoModelCore )

# Component(s) in the package:
atlas_add_library( LArGeoCode
                   src/*.cxx
                   PUBLIC_HEADERS LArGeoCode
                   LINK_LIBRARIES StoreGateLib
                   PRIVATE_LINK_LIBRARIES GeoModelCore::GeoModelKernel GaudiKernel AthenaBaseComps 
                                          GeoModelInterfaces RDBAccessSvcLib GeoModelUtilities DetDescrConditions 
                                          GeoPrimitives )

