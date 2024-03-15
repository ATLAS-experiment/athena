# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>

# Declare the package name.
atlas_subdir( DumpGeo )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager
   GeoModelWrite )

# Component(s) in the package.
atlas_add_component( DumpGeo
   DumpGeo/*.h src/*.cxx src/components/*.cxx
   PRIVATE_INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps AthenaKernel CxxUtils GaudiKernel GeoModelUtilities )

# Install files from the package.
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
