# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( CrackRegionGeoModel )

find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager GeoModelRead)

# Component(s) in the package.
atlas_add_library( CrackRegionGeoModel
		   src/*.cxx
		   PUBLIC_HEADERS CrackRegionGeoModel
		   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
		   LINK_LIBRARIES ${GEOMODEL_LIBRARIES}
		   PRIVATE_LINK_LIBRARIES GaudiKernel GeoModelUtilities StoreGateLib RDBAccessSvcLib )

