# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( CrackRegionGeoModel )

find_package( GeoModelCore )
find_package( GeoModelIO )


# Component(s) in the package.
atlas_add_library( CrackRegionGeoModel
		   src/*.cxx
		   PUBLIC_HEADERS CrackRegionGeoModel
		   LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead
		   PRIVATE_LINK_LIBRARIES GaudiKernel GeoModelUtilities StoreGateLib RDBAccessSvcLib )

