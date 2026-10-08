# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArReadoutGeometry )

# External dependencies:
find_package( CLHEP )
find_package( GeoModel COMPONENTS GeoModelKernel )

# Component(s) in the package:
atlas_add_library( LArReadoutGeometry
                   src/*.cxx
                   PUBLIC_HEADERS LArReadoutGeometry
		   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS  ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaKernel GeoPrimitives Identifier LArHV StoreGateLib CxxUtils
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} GeoModelUtilities GaudiKernel RDBAccessSvcLib GeoModelInterfaces )

