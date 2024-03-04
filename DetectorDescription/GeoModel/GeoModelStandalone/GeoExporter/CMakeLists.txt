# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GeoExporter )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager
   GeoModelWrite )

# Component(s) in the package:
atlas_add_library( GeoExporter
   GeoExporter/*.h src/*.cxx
   PUBLIC_HEADERS GeoExporter
   PRIVATE_INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
   PRIVATE_LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaKernel GeoModelUtilities VP1Base VP1Utils CxxUtils )
