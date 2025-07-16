# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ReadoutGeometryBase )

# External dependencies:
find_package( CLHEP )
find_package( GeoModel COMPONENTS GeoModelKernel )
find_package( Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_library( ReadoutGeometryBase
   ReadoutGeometryBase/*.h ReadoutGeometryBase/*.icc src/*.cxx
   PUBLIC_HEADERS ReadoutGeometryBase
   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                
   DEFINITIONS ${CLHEP_DEFINITIONS}
   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} ${CLHEP_LIBRARIES}  
                  CxxUtils AthenaKernel AtlasDetDescr GeoPrimitives Identifier
                  GaudiKernel InDetCondTools TrkDetElementBase AthenaBaseComps
   PRIVATE_LINK_LIBRARIES GeoModelUtilities TrkSurfaces )
   
atlas_add_test( SiCellId_test
   SOURCES test/SiCellId_test.cxx 
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES}  CxxUtils TestTools ReadoutGeometryBase
   POST_EXEC_SCRIPT "nopost.sh" )

atlas_add_test( PixelDiodeTree_test
   SOURCES test/PixelDiodeTree_test.cxx
   INCLUDE_DIRS
   LINK_LIBRARIES ReadoutGeometryBase
   POST_EXEC_SCRIPT "nopost.sh" )
