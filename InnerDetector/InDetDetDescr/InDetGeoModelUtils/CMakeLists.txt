# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetGeoModelUtils )

# External dependencies:
find_package( GeoModelCore )
find_package( GeoModelTools )
find_package( GeoModelIO )
# Component(s) in the package:
atlas_add_library( InDetGeoModelUtils
                   src/*.cxx
                   PUBLIC_HEADERS InDetGeoModelUtils
                   INCLUDE_DIRS ${XERCESC_INCLUDE_DIRS}
                   LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelIO::GeoModelRead 
                                  GeoModelIO::GeoModelDBManager GeoModelTools::GeoModelXml 
                                  GeoModelTools::ExpressionEvaluator
                                  AthenaKernel AthenaBaseComps CxxUtils GaudiKernel
                                  GeoModelInterfaces GeoModelUtilities GeoPrimitives RDBAccessSvcLib
                   PRIVATE_LINK_LIBRARIES PathResolver StoreGateLib )
