# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PixelGeoModel )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager GeoModelRead)

# Component(s) in the package:
atlas_add_library( PixelGeoModelLib
   PixelGeoModel/*.h src/*.cxx
   PUBLIC_HEADERS PixelGeoModel
   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
   LINK_LIBRARIES AthenaBaseComps GaudiKernel GeoModelUtilities GeometryDBSvcLib InDetGeoModelUtils
   PRIVATE_LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaKernel CxxUtils DetDescrConditions GeoModelInterfaces GeoPrimitives Identifier InDetIdentifier InDetReadoutGeometry PixelReadoutGeometryLib ReadoutGeometryBase RDBAccessSvcLib SGTools StoreGateLib
   PRIVATE_DEFINITIONS "-DGEOTORUS=1" )

atlas_add_component( PixelGeoModel
   src/components/*.cxx
   LINK_LIBRARIES PixelGeoModelLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Test(s) in the package:
atlas_add_test( PixelGMConfig_test
                SCRIPT test/PixelGMConfig_test.py
                POST_EXEC_SCRIPT nopost.sh )
