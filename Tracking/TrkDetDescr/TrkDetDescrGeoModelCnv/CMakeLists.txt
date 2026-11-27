# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkDetDescrGeoModelCnv )

# External dependencies:
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_library( TrkDetDescrGeoModelCnv
                   src/*.cxx
                   PUBLIC_HEADERS TrkDetDescrGeoModelCnv
                   LINK_LIBRARIES GeoModelUtilities GeoPrimitives TrkGeometry AthenaBaseComps
                   PRIVATE_LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelCore::GeoModelHelpers 
                                          GaudiKernel TrkVolumes CxxUtils )

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/GMTreeBrowser.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
     COMPILE_DEFINITIONS "FLATTEN" )
endif()
