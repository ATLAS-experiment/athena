# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_G4_SD )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelRead GeoModelDBManager )

# Component(s) in the package:
atlas_add_library( HGTD_G4_SD
                   src/*.h src/*.cxx src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${GEANT4_LIBRARIES} ${GEOMODEL_LIBRARIES} StoreGateLib
                   HGTD_Identifier InDetSimEvent G4AtlasToolsLib MCTruth GeoModelInterfaces GeoPrimitives )

# Install files from the package:
atlas_install_python_modules( python/*.py )
