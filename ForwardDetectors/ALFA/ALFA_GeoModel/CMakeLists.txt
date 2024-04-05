# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ALFA_GeoModel )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel )

# Component(s) in the package:
atlas_add_component( ALFA_GeoModel
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaKernel AthenaPoolUtilities GeoModelInterfaces GeoModelUtilities ALFA_Geometry StoreGateLib GaudiKernel RDBAccessSvcLib GeoPrimitives )

