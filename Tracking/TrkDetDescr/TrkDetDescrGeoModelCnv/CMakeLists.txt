# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkDetDescrGeoModelCnv )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel )

# Component(s) in the package:
atlas_add_library( TrkDetDescrGeoModelCnv
                   src/*.cxx
                   PUBLIC_HEADERS TrkDetDescrGeoModelCnv
                   PRIVATE_INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES GeoModelUtilities GeoPrimitives TrkGeometry AthenaBaseComps
                   PRIVATE_LINK_LIBRARIES ${GEOMODEL_LIBRARIES} GaudiKernel TrkVolumes )

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/GMTreeBrowser.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
     COMPILE_DEFINITIONS "FLATTEN" )
endif()
