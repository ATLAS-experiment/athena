# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( LArDetDescr )

# External dependencies.
find_package ( GeoModelCore )

# Component(s) in the package:
atlas_add_component( LArDetDescr
   LArDetDescr/*.h src/*.cxx src/components/*.cxx

   LINK_LIBRARIES GeoModelCore::GeoModelKernel GaudiKernel AthenaBaseComps
   CaloDetDescrLib CaloGeoHelpers RDBAccessSvcLib StoreGateLib CaloIdentifier
   GeoModelInterfaces GeoModelUtilities GeoPrimitives )

