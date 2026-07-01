# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: PhantomCalorimeter
################################################################################

# Declare the package name:
atlas_subdir( PhantomCalorimeter )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( GeoModel COMPONENTS GeoModelKernel )

# Component(s) in the package:
atlas_add_library( PhantomCalorimeter
                   src/*.cc
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} ${GEOMODEL_LIBRARIES} CaloIdentifier StoreGateLib GaudiKernel G4AtlasToolsLib GeoMaterial2G4 GeoModelInterfaces HitManagement MCTruth TileSimEvent )
set_target_properties( PhantomCalorimeter PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_runtime( share/phantombarrel.dtd share/phantombarrel.xml share/TileAncillary.mac share/phantomvis.mac )
